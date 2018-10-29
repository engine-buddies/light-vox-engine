#include "FrameResource.h"
#include "Camera.h"

using namespace DirectX;

#pragma region Init & Dealloc

FrameResource::FrameResource(
    ID3D12Device * device,
    ID3D12PipelineState* geometryBufferPso,
    ID3D12PipelineState* scenePso,
    ID3D12DescriptorHeap * dsvHeap,
    ID3D12DescriptorHeap * rtvHeap,
    ID3D12DescriptorHeap * cbvSrvHeap,
    D3D12_VIEWPORT * viewport,
    UINT frameResourceIndex )
{
    fenceValue = 0;
    this->geometryBufferPso = geometryBufferPso;
    this->deferredPso = scenePso;

    //create the scene rendering command list (one for g-buffer, one for deferred)
    InitCmdAllocatorsAndLists( device, frameResourceIndex );
    InitDescriptorHandles( device, dsvHeap, rtvHeap, cbvSrvHeap, frameResourceIndex );
    InitGraphicsResources( device, dsvHeap, rtvHeap, cbvSrvHeap, viewport, frameResourceIndex );
    InitCBV( device, cbvSrvHeap, frameResourceIndex );

    //combine all of our command lists
    batchedCommandList[ 0 ] = commandLists[ LV_COMMAND_LIST_INIT ].Get();
    memcpy( batchedCommandList + 1, geometryCmdLists, sizeof( geometryCmdLists ) );
    batchedCommandList[ _countof( geometryCmdLists ) + 1 ] = commandLists[ LV_COMMAND_LIST_LIGHTING_PASS ].Get();
}

inline void FrameResource::InitCmdAllocatorsAndLists(
    ID3D12Device * device,
    UINT frameResourceIndex )
{
    //  then create command list, name it, and close it
    for ( size_t i = 0; i < LV_NUM_CONTEXTS; ++i )
    {
        ThrowIfFailed( device->CreateCommandAllocator( D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS( &geometryCmdAllocators[ i ] ) ) );
        ThrowIfFailed( device->CreateCommandList(
            0,
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            geometryCmdAllocators[ i ].Get(),
            this->geometryBufferPso.Get(),
            IID_PPV_ARGS( &geometryCmdLists[ i ] )
        ) );
        NAME_D3D12_OBJECT_WITH_NAME( geometryCmdAllocators[ i ], "%s (F%d T%d)", "Geometry", frameResourceIndex, i );
        NAME_D3D12_OBJECT_WITH_NAME( geometryCmdLists[ i ], "%s (F%d T%d)", "Geometry", frameResourceIndex, i );
        ThrowIfFailed( geometryCmdLists[ i ]->Close() );
    }

    //go through other frmelist
    for ( UINT i = 0; i < LV_COMMAND_LIST_COUNT; ++i )
    {
        ThrowIfFailed( device->CreateCommandAllocator( D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS( &commandAllocators[ i ] ) ) );
        NAME_D3D12_OBJECT_WITH_NAME( commandAllocators[ i ], "%s (Frame#%d)", "Base", i );

        switch ( i )
        {
            case LV_COMMAND_LIST_INIT:
                ThrowIfFailed( device->CreateCommandList(
                    0,
                    D3D12_COMMAND_LIST_TYPE_DIRECT,
                    commandAllocators[ i ].Get(),
                    geometryBufferPso.Get(),
                    IID_PPV_ARGS( &commandLists[ i ] )
                ) );
                NAME_D3D12_OBJECT_WITH_NAME( commandLists[ i ], "%s (Frame#%d)", "Init", frameResourceIndex );
                break;
            case LV_COMMAND_LIST_LIGHTING_PASS:
                ThrowIfFailed( device->CreateCommandList(
                    0,
                    D3D12_COMMAND_LIST_TYPE_DIRECT,
                    commandAllocators[ i ].Get(),
                    deferredPso.Get(),
                    IID_PPV_ARGS( &commandLists[ i ] )
                ) );
                NAME_D3D12_OBJECT_WITH_NAME( commandLists[ i ], "%s (Frame#%d)", "Lighting", frameResourceIndex );
                break;
        }

        ThrowIfFailed( commandLists[ i ]->Close() );
    }
}

inline void FrameResource::InitDescriptorHandles(
    ID3D12Device * device,
    ID3D12DescriptorHeap * dsvHeap,
    ID3D12DescriptorHeap * rtvHeap,
    ID3D12DescriptorHeap * cbvSrvHeap,
    UINT frameResourceIndex )
{
    const UINT cbvSrvDescriptorSize = device->GetDescriptorHandleIncrementSize( D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV );
    const UINT rtvDescriptorSize = device->GetDescriptorHandleIncrementSize( D3D12_DESCRIPTOR_HEAP_TYPE_RTV );

    //Get Handle to Null SRVs
    CD3DX12_GPU_DESCRIPTOR_HANDLE cbvSrvGpuHandle( cbvSrvHeap->GetGPUDescriptorHandleForHeapStart() );
    nullHandle = cbvSrvGpuHandle;

    //Offset to the start of the current frame's SRv (g-buffers)
    cbvSrvGpuHandle.Offset( LV_NUM_NULL_SRV + frameResourceIndex * LV_NUM_CBVSRV_PER_FRAME, cbvSrvDescriptorSize );
    gBufferSrvHandle = cbvSrvGpuHandle;

    //offset to where the CBV's are
    cbvSrvGpuHandle.Offset( LV_NUM_GBUFFER_RTV, cbvSrvDescriptorSize );
    sceneCbvHandle = cbvSrvGpuHandle;

    //cache the handles to the G-buffers
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(
        rtvHeap->GetCPUDescriptorHandleForHeapStart(),
        frameResourceIndex * LV_NUM_CBVSRV_PER_FRAME,
        rtvDescriptorSize
    );
    for ( UINT i = 0; i < LV_NUM_GBUFFER_RTV; ++i )
    {
        rtvGbufferHandles[ i ] = rtvHandle;
        rtvHandle.Offset( rtvDescriptorSize );
    }

    dsvHandle = dsvHeap->GetCPUDescriptorHandleForHeapStart();
}

inline void FrameResource::InitGraphicsResources(
    ID3D12Device * device,
    ID3D12DescriptorHeap * dsvHeap,
    ID3D12DescriptorHeap * rtvHeap,
    ID3D12DescriptorHeap * cbvSrvHeap,
    D3D12_VIEWPORT * viewport,
    UINT frameResourceIndex )
{
    const UINT cbvSrvDescriptorSize = device->GetDescriptorHandleIncrementSize( D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV );
    const UINT rtvDescriptorSize = device->GetDescriptorHandleIncrementSize( D3D12_DESCRIPTOR_HEAP_TYPE_RTV );

    // Create RTVs:
    //1 for each frame * 1 for each gbuffer resource
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(
        rtvHeap->GetCPUDescriptorHandleForHeapStart(),
        frameResourceIndex * ( LV_NUM_RTV_PER_FRAME ),
        rtvDescriptorSize
    );

    //Get CPU handle to the start of the frame's SRVs (g-buffers)
    CD3DX12_CPU_DESCRIPTOR_HANDLE cbvSrvCpuHandle( cbvSrvHeap->GetCPUDescriptorHandleForHeapStart(),
        LV_NUM_NULL_SRV + frameResourceIndex * LV_NUM_CBVSRV_PER_FRAME,
        cbvSrvDescriptorSize
    );

    DXGI_FORMAT textureFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;
    UINT16 textureMipLevels = 1;

    {
        //Texture Description for G Buffer
        D3D12_RESOURCE_DESC textureDesc = { };
        textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        textureDesc.Alignment = 0;
        textureDesc.SampleDesc.Count = 1;
        textureDesc.SampleDesc.Quality = 0;
        textureDesc.MipLevels = textureMipLevels;
        textureDesc.DepthOrArraySize = 1;
        textureDesc.Width = static_cast<UINT64>( viewport->Width );
        textureDesc.Height = static_cast<UINT>( viewport->Height );
        textureDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        textureDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
        textureDesc.Format = textureFormat;

        //Clear color for the RTVs
        //Normal and Position RTVs MUST BE CLEARED TO BLACK
        D3D12_CLEAR_VALUE clearValueBlack = { };
        float gBufferClearColor[ 4 ] = LV_RTV_CLEAR_COLOR;
        clearValueBlack.Color[ 0 ] = gBufferClearColor[ 0 ];
        clearValueBlack.Color[ 1 ] = gBufferClearColor[ 1 ];
        clearValueBlack.Color[ 2 ] = gBufferClearColor[ 2 ];
        clearValueBlack.Color[ 3 ] = gBufferClearColor[ 3 ];
        clearValueBlack.Format = textureFormat;

        D3D12_CLEAR_VALUE clearValueAlbedo = { };
        float gBufferClearColorAlbedo[ 4 ] = LV_RTV_CLEAR_BG_COLOR;
        clearValueAlbedo.Color[ 0 ] = gBufferClearColorAlbedo[ 0 ];
        clearValueAlbedo.Color[ 1 ] = gBufferClearColorAlbedo[ 1 ];
        clearValueAlbedo.Color[ 2 ] = gBufferClearColorAlbedo[ 2 ];
        clearValueAlbedo.Color[ 3 ] = gBufferClearColorAlbedo[ 3 ];
        clearValueAlbedo.Format = textureFormat;

        //Create texture resources for the RTV/SRV
        CD3DX12_HEAP_PROPERTIES heapProperty( D3D12_HEAP_TYPE_DEFAULT );
        for ( UINT i = 0; i < LV_NUM_GBUFFER_RTV; ++i )
        {
            D3D12_CLEAR_VALUE clearVal = clearValueBlack;
            if ( i == 0 )
                clearVal = clearValueAlbedo;

            ThrowIfFailed( device->CreateCommittedResource(
                &heapProperty,                          //heap property (default)
                D3D12_HEAP_FLAG_NONE,                   //heap flags
                &textureDesc,                           //description for texture
                D3D12_RESOURCE_STATE_RENDER_TARGET,     //what kind of resource is it
                &clearVal,                              //value to clear with
                IID_PPV_ARGS( rtvTextures[ i ].GetAddressOf() ) )
            );

            NAME_D3D12_OBJECT_WITH_NAME( rtvTextures[ i ], "(%d) gBuffer of frame %d", i, frameResourceIndex );
        }
    }
    {
        //Render Target View Description
        D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = { };
        {
            rtvDesc.Texture2D.MipSlice = 0;
            rtvDesc.Texture2D.PlaneSlice = 0;
            rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
            rtvDesc.Format = textureFormat;
        }

        //Shader Resource View Description
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = { };
        {
            srvDesc.Texture2D.MipLevels = textureMipLevels;
            srvDesc.Texture2D.MostDetailedMip = 0;
            srvDesc.Format = textureFormat;
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        }



        for ( UINT i = 0; i < LV_NUM_GBUFFER_RTV; ++i )
        {
            //create RTV and SRV and stay consistent
            device->CreateRenderTargetView( rtvTextures[ i ].Get(), &rtvDesc, rtvHandle );
            device->CreateShaderResourceView( rtvTextures[ i ].Get(), &srvDesc, cbvSrvCpuHandle );
            rtvHandle.Offset( 1, rtvDescriptorSize );

            cbvSrvCpuHandle.Offset( cbvSrvDescriptorSize );
        }
    }
}

inline void FrameResource::InitCBV(
    ID3D12Device * device,
    ID3D12DescriptorHeap * cbvSrvHeap,
    UINT frameResourceIndex )
{
    const UINT cbvSrvDescriptorSize = device->GetDescriptorHandleIncrementSize( D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV );
    CD3DX12_RANGE zeroReadRange( 0, 0 );

    //Get CPU handle to the start of the frame's SRVs (g-buffers)
    CD3DX12_CPU_DESCRIPTOR_HANDLE cbvSrvCpuHandle( cbvSrvHeap->GetCPUDescriptorHandleForHeapStart(),
        LV_NUM_NULL_SRV + frameResourceIndex * LV_NUM_CBVSRV_PER_FRAME + LV_NUM_GBUFFER_RTV,
        cbvSrvDescriptorSize
    );

    //create the upload buffer for instance data
    ThrowIfFailed( device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_UPLOAD ),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer( sizeof( InstanceBuffer ) * LV_MAX_INSTANCE_COUNT ),
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS( &instanceUploadBuffer )
    ) );;

    // map it to a WO struct buffer thing
    ThrowIfFailed( instanceUploadBuffer->Map( 0,
        &zeroReadRange,
        reinterpret_cast<void**>( &instanceBufferWO )
    ) );

#ifdef _DEBUG
    //create the upload buffer for instance data
    ThrowIfFailed( device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_UPLOAD ),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer( sizeof( CubeInstanceData ) * LV_DEBUG_MAX_CUBE_COUNT ),
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS( &instanceDebugUploadBuffer )
    ) );;

    // map it to a WO struct buffer thing
    ThrowIfFailed( instanceDebugUploadBuffer->Map( 0,
        &zeroReadRange,
        reinterpret_cast<void**>( &debugInstanceBufferWO )
    ) );

#endif


    // Create the constant buffers.
    const UINT constantBufferSize = ( sizeof( SceneConstantBuffer )
        + ( D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1 ) )
        & ~( D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1 ); // must be a multiple 256 bytes
    ThrowIfFailed( device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_UPLOAD ),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer( constantBufferSize ),
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS( &sceneConstantBuffer ) )
    );
    NAME_D3D12_OBJECT_WITH_NAME( sceneConstantBuffer, "frame#%d", frameResourceIndex );

    // Map the constant buffers and cache their heap pointers.
    ThrowIfFailed( sceneConstantBuffer->Map( 0,
        &zeroReadRange,
        reinterpret_cast<void**>( &sceneConstantBufferWO )
    ) );

    // Create the constant buffer view for scene pass
    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
    cbvDesc.SizeInBytes = constantBufferSize;

    // Describe and create the scene constant buffer view (CBV) and 
    // cache the GPU descriptor handle.
    cbvDesc.BufferLocation = sceneConstantBuffer->GetGPUVirtualAddress();
    device->CreateConstantBufferView( &cbvDesc, cbvSrvCpuHandle );
}

FrameResource::~FrameResource()
{
    for ( size_t i = 0; i < LV_NUM_CONTEXTS; ++i )
    {
        geometryCmdLists[ i ] = nullptr;
        geometryCmdAllocators[ i ] = nullptr;
    }

    for ( size_t i = 0; i < LV_COMMAND_LIST_COUNT; ++i )
    {
        commandLists[ i ] = nullptr;
        commandAllocators[ i ] = nullptr;
    }

    if ( sceneConstantBuffer != nullptr )
        sceneConstantBuffer->Unmap( 0, nullptr );
    sceneConstantBuffer = nullptr;

    if ( instanceUploadBuffer != nullptr )
        instanceUploadBuffer->Unmap( 0, nullptr );
    instanceUploadBuffer = nullptr;
    //lightConstantBuffer = nullptr;
}

#pragma endregion 

#pragma region Render Logic

void FrameResource::ResetCommandListsAndAllocators()
{
    for ( size_t i = 0; i < LV_NUM_CONTEXTS; ++i )
    {
        ThrowIfFailed( geometryCmdAllocators[ i ]->Reset() );
        ThrowIfFailed( geometryCmdLists[ i ]->Reset(
            geometryCmdAllocators[ i ].Get(),
            geometryBufferPso.Get() )
        );
    }

    for ( UINT i = 0; i < LV_COMMAND_LIST_COUNT; ++i )
    {
        ThrowIfFailed( commandAllocators[ i ]->Reset() );
    }
    ThrowIfFailed( commandLists[ LV_COMMAND_LIST_INIT ]->Reset(
        commandAllocators[ LV_COMMAND_LIST_INIT ].Get(),
        geometryBufferPso.Get()
    ) );

    ThrowIfFailed( commandLists[ LV_COMMAND_LIST_LIGHTING_PASS ]->Reset(
        commandAllocators[ LV_COMMAND_LIST_LIGHTING_PASS ].Get(),
        deferredPso.Get()
    ) );
}

void FrameResource::BindGBuffer()
{
    geometryCmdLists[ 0 ]->SetGraphicsRootShaderResourceView( LV_ROOT_SIGNATURE_INSTANCED_DATA, instanceUploadBuffer->GetGPUVirtualAddress() );
    geometryCmdLists[ 0 ]->SetGraphicsRootDescriptorTable( LV_ROOT_SIGNATURE_GBUFFER_SRV, nullHandle );
    geometryCmdLists[ 0 ]->SetGraphicsRootDescriptorTable( LV_ROOT_SIGNATURE_CBV, sceneCbvHandle );
    geometryCmdLists[ 0 ]->OMSetRenderTargets( LV_NUM_GBUFFER_RTV, rtvGbufferHandles, FALSE, &dsvHandle );
}

#ifdef _DEBUG

void FrameResource::BindDebug( D3D12_CPU_DESCRIPTOR_HANDLE * rtvHandle )
{
    commandLists[ LV_COMMAND_LIST_LIGHTING_PASS ]->SetGraphicsRootShaderResourceView( LV_ROOT_SIGNATURE_INSTANCED_DATA, instanceDebugUploadBuffer->GetGPUVirtualAddress() );
    commandLists[ LV_COMMAND_LIST_LIGHTING_PASS ]->SetGraphicsRootDescriptorTable( LV_ROOT_SIGNATURE_GBUFFER_SRV, nullHandle );
    commandLists[ LV_COMMAND_LIST_LIGHTING_PASS ]->SetGraphicsRootDescriptorTable( LV_ROOT_SIGNATURE_CBV, sceneCbvHandle );
    commandLists[ LV_COMMAND_LIST_LIGHTING_PASS ]->OMSetRenderTargets( 1, rtvHandle, FALSE, &dsvHandle );
}

#endif

void FrameResource::BindDeferred(
    D3D12_CPU_DESCRIPTOR_HANDLE * rtvHandle,
    D3D12_GPU_DESCRIPTOR_HANDLE samplerHandle
)
{
    commandLists[ LV_COMMAND_LIST_LIGHTING_PASS ]->SetGraphicsRootDescriptorTable( LV_ROOT_SIGNATURE_GBUFFER_SRV, gBufferSrvHandle );
    commandLists[ LV_COMMAND_LIST_LIGHTING_PASS ]->SetGraphicsRootDescriptorTable( LV_ROOT_SIGNATURE_CBV, sceneCbvHandle );
    commandLists[ LV_COMMAND_LIST_LIGHTING_PASS ]->SetGraphicsRootDescriptorTable( LV_ROOT_SIGNATURE_SAMPLER, samplerHandle );
    commandLists[ LV_COMMAND_LIST_LIGHTING_PASS ]->OMSetRenderTargets( 1, rtvHandle, FALSE, &dsvHandle );
}

void FrameResource::SwapBarriers()
{
    CD3DX12_RESOURCE_BARRIER barriers[ LV_NUM_GBUFFER_RTV ];
    for ( size_t i = 0; i < LV_NUM_GBUFFER_RTV; ++i )
    {
        barriers[ i ] = CD3DX12_RESOURCE_BARRIER::Transition(
            rtvTextures[ i ].Get(),
            D3D12_RESOURCE_STATE_RENDER_TARGET,
            D3D12_RESOURCE_STATE_GENERIC_READ
        );
    }
    commandLists[ LV_COMMAND_LIST_LIGHTING_PASS ]->ResourceBarrier( LV_NUM_GBUFFER_RTV, barriers );

}

void FrameResource::Cleanup()
{
    CD3DX12_RESOURCE_BARRIER barriers[ LV_NUM_GBUFFER_RTV ];
    for ( size_t i = 0; i < LV_NUM_GBUFFER_RTV; ++i )
    {
        barriers[ i ] = CD3DX12_RESOURCE_BARRIER::Transition(
            rtvTextures[ i ].Get(),
            D3D12_RESOURCE_STATE_GENERIC_READ,
            D3D12_RESOURCE_STATE_RENDER_TARGET
        );
    }
    commandLists[ LV_COMMAND_LIST_LIGHTING_PASS ]->ResourceBarrier( LV_NUM_GBUFFER_RTV, barriers );
}

#pragma endregion

#pragma region Update

void FrameResource::WriteConstantBuffers(
    DirectX::XMFLOAT4X4 transforms[],
    D3D12_VIEWPORT * viewport,
    Camera * camera )
{
    SceneConstantBuffer sceneConsts = {};
    camera->GetViewProjMatrix(
        &sceneConsts.view,
        &sceneConsts.projection,
        viewport->Width,
        viewport->Height
    );
    sceneConsts.cameraPosition = camera->GetPosition();

    //copy over
    memcpy( sceneConstantBufferWO, &sceneConsts, sizeof( SceneConstantBuffer ) );
    memcpy( instanceBufferWO, transforms, sizeof( InstanceBuffer ) * LV_MAX_INSTANCE_COUNT );
    //memcpy(lightConstantBufferWO, &lightConsts, sizeof(LightConstantBuffer));
}

#ifdef _DEBUG
void FrameResource::WriteDebugInstanceBuffers( 
    CubeInstanceData instanceData[],
    size_t count
)
{
    memcpy( debugInstanceBufferWO, instanceData, sizeof( CubeInstanceData ) * count );
}
#endif
#pragma endregion
