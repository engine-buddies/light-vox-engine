#include "FrameResource.h"
#include "Camera.h"

using namespace DirectX;

FrameResource::FrameResource(
    ID3D12Device * device,
    ID3D12PipelineState* geometryBufferPso,
    ID3D12PipelineState* scenePso,
    ID3D12DescriptorHeap * dsvHeap,
    ID3D12DescriptorHeap * rtvHeap,
    ID3D12DescriptorHeap * cbvSrvHeap,
    IDXGISwapChain3 * swapChain,
    D3D12_VIEWPORT * viewport,
    UINT frameResourceIndex )
{
    fenceValue = 0;
    this->geometryBufferPso = geometryBufferPso;
    this->deferredPso = scenePso;
    const UINT cbvSrvDescriptorSize = device->GetDescriptorHandleIncrementSize( D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV );

    //create the scene rendering command list (one for g-buffer, one for deferred)
    CreateCommandAllocatorsAndLists( device );

    //get handle to the SRV of the current frame (the g-buffer SRVs)
    CD3DX12_CPU_DESCRIPTOR_HANDLE cbvSrvCpuHandle( cbvSrvHeap->GetCPUDescriptorHandleForHeapStart(),
        frameResourceIndex * ( LV_NUM_GBUFFER_RTV + 1 ),
        cbvSrvDescriptorSize
    );

    CD3DX12_GPU_DESCRIPTOR_HANDLE cbvSrvGpuHandle( cbvSrvHeap->GetGPUDescriptorHandleForHeapStart(),
        frameResourceIndex * ( LV_NUM_GBUFFER_RTV + 1 ),
        cbvSrvDescriptorSize
    );

    {
        //Texture Description for G Buffer
        D3D12_RESOURCE_DESC textureDesc = { };
        textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        textureDesc.Alignment = 0;
        textureDesc.SampleDesc.Count = 1;
        textureDesc.SampleDesc.Quality = 0;
        textureDesc.MipLevels = 1;
        textureDesc.DepthOrArraySize = 1;
        textureDesc.Width = viewport->Width;
        textureDesc.Height = viewport->Height;
        textureDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        textureDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
        textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

        //Clear color for the RTVs
        //Normal and Position RTVs MUST BE CLEARED TO BLACK
        float gBufferClearColor[ 4 ] = { 0.0f, 0.0f, 0.0f, 1.0f };

        D3D12_CLEAR_VALUE clearVal = { };
        clearVal.Color[ 0 ] = gBufferClearColor[ 0 ];
        clearVal.Color[ 1 ] = gBufferClearColor[ 1 ];
        clearVal.Color[ 2 ] = gBufferClearColor[ 2 ];
        clearVal.Color[ 3 ] = gBufferClearColor[ 3 ];
        clearVal.Format = textureDesc.Format;


        //Render Target View Description
        D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = { };
        rtvDesc.Texture2D.MipSlice = 0;
        rtvDesc.Texture2D.PlaneSlice = 0;
        rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
        rtvDesc.Format = textureDesc.Format;

        //Shader Resource View Description
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = { };
        srvDesc.Texture2D.MipLevels = textureDesc.MipLevels;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Format = textureDesc.Format;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

        //Create Textures
        CD3DX12_HEAP_PROPERTIES heapProperty( D3D12_HEAP_TYPE_DEFAULT );
        for ( UINT i = 0; i < LV_NUM_GBUFFER_RTV; ++i )
        {
            ThrowIfFailed( device->CreateCommittedResource(
                &heapProperty,
                D3D12_HEAP_FLAG_NONE,
                &textureDesc,
                D3D12_RESOURCE_STATE_RENDER_TARGET,
                &clearVal,
                IID_PPV_ARGS( rtvTextures[ i ].GetAddressOf() ) )
            );
            NAME_D3D12_OBJECT_INDEXED( rtvTextures, i );
        }

        // Create RTVs:
        //1 for each frame * 1 for each gbuffer resource
        UINT rtvDescriptorSize = device->GetDescriptorHandleIncrementSize( D3D12_DESCRIPTOR_HEAP_TYPE_RTV );
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(
            rtvHeap->GetCPUDescriptorHandleForHeapStart(),
            frameResourceIndex * ( LV_NUM_GBUFFER_RTV + 1 ),
            rtvDescriptorSize
        );

        gBufferSrvHandle = cbvSrvGpuHandle;
        for ( UINT i = 0; i < LV_NUM_GBUFFER_RTV; i++ )
        {
            device->CreateRenderTargetView( rtvTextures[ i ].Get(), &rtvDesc, rtvHandle );
            rtvHandle.Offset( 1, rtvDescriptorSize );

            device->CreateShaderResourceView( rtvTextures[ i ].Get(), &srvDesc, cbvSrvCpuHandle );
            cbvSrvCpuHandle.Offset( cbvSrvDescriptorSize );
            cbvSrvGpuHandle.Offset( cbvSrvDescriptorSize );
        }
    }

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

    // Map the constant buffers and cache their heap pointers.
    CD3DX12_RANGE readRange( 0, 0 );		// We do not intend to read from this resource on the CPU.
    ThrowIfFailed( sceneConstantBuffer->Map( 0,
        &readRange,
        reinterpret_cast<void**>( &sceneConstantBufferWO )
    )
    );

    // Create the constant buffer view for scene pass
    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
    cbvDesc.SizeInBytes = constantBufferSize;

    // Describe and create the scene constant buffer view (CBV) and 
    // cache the GPU descriptor handle.
    cbvDesc.BufferLocation = sceneConstantBuffer->GetGPUVirtualAddress();
    device->CreateConstantBufferView( &cbvDesc, cbvSrvCpuHandle );

    sceneCbvHandle = cbvSrvGpuHandle;

    //combine all of our command lists

    //const UINT batchSize = 2 + LV_COMMAND_LIST_COUNT;
    batchedCommandList[ 0 ] = commandLists[ LV_COMMAND_LIST_PRE ].Get();
    batchedCommandList[ 1 ] = geometryBufferCommandList.Get();
    batchedCommandList[ 2 ] = commandLists[ LV_COMMAND_LIST_MID ].Get();
    batchedCommandList[ 3 ] = deferredCommandList.Get();
    batchedCommandList[ 4 ] = commandLists[ LV_COMMAND_LIST_POST ].Get();



    //batchedCommandList[ 0 ] = geometryBufferCommandList.Get();
    //batchedCommandList[ 1 ] = commandLists[ LV_COMMAND_LIST_MID ].Get();
    //batchedCommandList[ 2 ] = deferredCommandList.Get();
    //batchedCommandList[ 3 ] = commandLists[ LV_COMMAND_LIST_POST ].Get();
}

FrameResource::~FrameResource()
{
    geometryBufferCommandList = nullptr;
    geometryBufferCommandAllocator = nullptr;

    for ( size_t i = 0; i < LV_COMMAND_LIST_COUNT; ++i )
    {
        commandLists[ i ] = nullptr;
        commandAllocators[ i ] = nullptr;
    }

    sceneConstantBuffer = nullptr;
    //lightConstantBuffer = nullptr;

    deferredCommandList = nullptr;
    deferredCommandAllocator = nullptr;
}

void FrameResource::BindGBuffer(
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle[],
    UINT rtvCount,
    D3D12_CPU_DESCRIPTOR_HANDLE * dsvHandle
)
{
    //geometryBufferCommandList->SetGraphicsRootDescriptorTable( 0, gBufferSrvHandle );
    geometryBufferCommandList->SetGraphicsRootDescriptorTable( 1, sceneCbvHandle );
    geometryBufferCommandList->OMSetRenderTargets( rtvCount, rtvHandle, FALSE, dsvHandle );
}

void FrameResource::BindDeferred(
    D3D12_CPU_DESCRIPTOR_HANDLE * rtvHandle,
    D3D12_CPU_DESCRIPTOR_HANDLE * dsvHandle
)
{
    deferredCommandList->SetGraphicsRootDescriptorTable( 0, gBufferSrvHandle );
    deferredCommandList->SetGraphicsRootDescriptorTable( 1, sceneCbvHandle );
    deferredCommandList->OMSetRenderTargets( 1, rtvHandle, FALSE, dsvHandle );
}

void FrameResource::ResetCommandListsAndAllocators()
{
    ThrowIfFailed( geometryBufferCommandAllocator->Reset() );
    ThrowIfFailed( geometryBufferCommandList->Reset(
        geometryBufferCommandAllocator.Get(),
        geometryBufferPso.Get() )
    );

    ThrowIfFailed( deferredCommandAllocator->Reset() );
    ThrowIfFailed( deferredCommandList->Reset(
        deferredCommandAllocator.Get(),
        deferredPso.Get() )
    );

    for ( UINT i = 0; i < LV_COMMAND_LIST_COUNT; i++ )
    {
        ThrowIfFailed( commandAllocators[ i ]->Reset() );
    }
    ThrowIfFailed( commandLists[ LV_COMMAND_LIST_PRE ]->Reset(
        commandAllocators[ LV_COMMAND_LIST_PRE ].Get(),
        geometryBufferPso.Get()
    ) );

    ThrowIfFailed( commandLists[ LV_COMMAND_LIST_MID ]->Reset(
        commandAllocators[ LV_COMMAND_LIST_MID ].Get(),
        deferredPso.Get()
    ) );

    ThrowIfFailed( commandLists[ LV_COMMAND_LIST_POST ]->Reset(
        commandAllocators[ LV_COMMAND_LIST_POST ].Get(),
        deferredPso.Get()
    ) );
}

void FrameResource::Begin()
{


    //create transitional barriers for the 3 g-buffers
    //CD3DX12_RESOURCE_BARRIER barriers[ LV_NUM_GBUFFER_RTV ];
    //for ( size_t i = 0; i < LV_NUM_GBUFFER_RTV; ++i )
    //{
    //    barriers[ i ] = CD3DX12_RESOURCE_BARRIER::Transition(
    //        rtvTextures[ i ].Get(),
    //        D3D12_RESOURCE_STATE_GENERIC_READ,
    //        D3D12_RESOURCE_STATE_RENDER_TARGET
    //    );
    //}

    //transition the g-buffers buffer to be used as render target
    //geometryBufferCommandList->ResourceBarrier( LV_NUM_GBUFFER_RTV, barriers);



    //clear back buffer with c o r n f l o w e r   b l u e
    //const float clearColor[] = { 0.392f, 0.584f, 0.929f, 1.0f };
}

void FrameResource::SwapBarriers()
{
    //transition all our g-buffers
    CD3DX12_RESOURCE_BARRIER barriers[ LV_NUM_GBUFFER_RTV ];
    for ( size_t i = 0; i < LV_NUM_GBUFFER_RTV; ++i )
    {
        barriers[ i ] = CD3DX12_RESOURCE_BARRIER::Transition(
            rtvTextures[ i ].Get(),
            D3D12_RESOURCE_STATE_RENDER_TARGET,
            D3D12_RESOURCE_STATE_GENERIC_READ
        );
    }
    commandLists[ LV_COMMAND_LIST_MID ]->ResourceBarrier( LV_NUM_GBUFFER_RTV, barriers );
}


void FrameResource::Finish()
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
    commandLists[ LV_COMMAND_LIST_POST ]->ResourceBarrier( LV_NUM_GBUFFER_RTV, barriers );
}

void FrameResource::WriteConstantBuffers(
    DirectX::XMFLOAT4X4 transforms[],
    D3D12_VIEWPORT * viewport,
    Camera * camera )
{
    SceneConstantBuffer sceneConsts = {};
    //LightConstantBuffer lightConsts = {};

    sceneConsts.model = transforms[ 0 ];

    camera->GetViewProjMatrix(
        &sceneConsts.view,
        &sceneConsts.projection,
        viewport->Width,
        viewport->Height
    );


    //copy over
    memcpy( sceneConstantBufferWO, &sceneConsts, sizeof( SceneConstantBuffer ) );
    //memcpy(lightConstantBufferWO, &lightConsts, sizeof(LightConstantBuffer));
}

inline void FrameResource::CreateCommandAllocatorsAndLists( ID3D12Device * device )
{
    //  then create command list, name it, and close it
    ThrowIfFailed( device->CreateCommandAllocator(
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        IID_PPV_ARGS( &geometryBufferCommandAllocator ) ) );
    ThrowIfFailed( device->CreateCommandList(
        0,
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        geometryBufferCommandAllocator.Get(),
        this->geometryBufferPso.Get(),
        IID_PPV_ARGS( &geometryBufferCommandList )
    ) );
    NAME_D3D12_OBJECT( geometryBufferCommandList );
    ThrowIfFailed( geometryBufferCommandList->Close() );

    //do the same for deferred stuff (2nd pass)
    ThrowIfFailed( device->CreateCommandAllocator(
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        IID_PPV_ARGS( &deferredCommandAllocator ) ) );
    ThrowIfFailed( device->CreateCommandList(
        0,
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        deferredCommandAllocator.Get(),
        this->deferredPso.Get(),
        IID_PPV_ARGS( &deferredCommandList )
    ) );
    NAME_D3D12_OBJECT( deferredCommandList );
    ThrowIfFailed( deferredCommandList->Close() );


    for ( UINT i = 0; i < LV_COMMAND_LIST_COUNT; i++ )
    {
        ThrowIfFailed( device->CreateCommandAllocator(
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            IID_PPV_ARGS( &commandAllocators[ i ] )
        ) );

        if ( i == 0 )
        {
            ThrowIfFailed( device->CreateCommandList(
                0,
                D3D12_COMMAND_LIST_TYPE_DIRECT,
                commandAllocators[ i ].Get(),
                geometryBufferPso.Get(),
                IID_PPV_ARGS( &commandLists[ i ] )
            ) );
        }
        else
        {
            ThrowIfFailed( device->CreateCommandList(
                0,
                D3D12_COMMAND_LIST_TYPE_DIRECT,
                commandAllocators[ i ].Get(),
                deferredPso.Get(),
                IID_PPV_ARGS( &commandLists[ i ] )
            ) );
        }

        NAME_D3D12_OBJECT_INDEXED( commandLists, i );
        ThrowIfFailed( commandLists[ i ]->Close() );
    }
}

