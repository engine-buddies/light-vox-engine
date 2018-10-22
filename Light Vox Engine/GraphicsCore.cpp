#include "GraphicsCore.h"
#include "FrameResource.h"
#include "Camera.h"

//needed for 'right now'
#include "ObjLoader.h"

using namespace Microsoft::WRL;

GraphicsCore::GraphicsCore( HWND hWindow, UINT windowW, UINT windowH )
{
    this->hWindow = hWindow;
    windowWidth = windowW;
    windowHeight = windowH;
    fenceValue = 0;
    frameIndex = 0;
    currentFrameResource = nullptr;
}

GraphicsCore::~GraphicsCore()
{
}

void GraphicsCore::OnResize( UINT width, UINT height )
{
    //TODO - recreate scssisor rectangle, viewport, and back buffers
}

HRESULT GraphicsCore::Init()
{
    ThrowIfFailed( InitDeviceCommandQueueSwapChain() );
    ThrowIfFailed( InitRootSignature() );
    ThrowIfFailed( InitPSO() );
    ThrowIfFailed( InitLightPassPSO() );
    ThrowIfFailed( InitRtvHeap() );
    ThrowIfFailed( InitDepthStencil() );
    ThrowIfFailed( InitViewportScissorRectangle() );
    ThrowIfFailed( InitSynchronizationObjects() );
    ThrowIfFailed( InitInputShaderResources() );
    ThrowIfFailed( InitFrameResources() );

    return S_OK;
}

void GraphicsCore::Update( DirectX::XMFLOAT4X4 transforms[], Camera* camera )
{
    PIXSetMarker( commandQueue.Get(), 0, L"Getting last completed fence" );

    const UINT64 lastCompletedFence = fence->GetCompletedValue();
    currentFrameResourceIndex = ( currentFrameResourceIndex + 1 ) % LV_FRAME_COUNT;
    currentFrameResource = frameResources[ currentFrameResourceIndex ];

    //make sure the frame we're on isn't currently in use by the GPU
    if ( currentFrameResource->fenceValue > lastCompletedFence )
    {
        HANDLE eventHandle = CreateEvent( nullptr, FALSE, FALSE, nullptr );
        if ( eventHandle == nullptr )
            ThrowIfFailed( HRESULT_FROM_WIN32( GetLastError() ) );

        ThrowIfFailed( fence->SetEventOnCompletion(
            currentFrameResource->fenceValue,
            eventHandle )
        );
        WaitForSingleObject( eventHandle, INFINITE );
        CloseHandle( eventHandle );
    }

    currentFrameResource->WriteConstantBuffers( transforms, &viewport, camera );

}

void GraphicsCore::Render()
{
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle( rtvHeap->GetCPUDescriptorHandleForHeapStart(),
        currentFrameResourceIndex * ( LV_NUM_GBUFFER_RTV + 1 ),
        rtvDescriptorSize
    );
    CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(
        dsvHeap->GetCPUDescriptorHandleForHeapStart()
    );

    CD3DX12_CPU_DESCRIPTOR_HANDLE gBufferHandles[ LV_NUM_GBUFFER_RTV ];
    for ( UINT i = 0; i < LV_NUM_GBUFFER_RTV; ++i )
    {
        gBufferHandles[ i ] = CD3DX12_CPU_DESCRIPTOR_HANDLE(
            rtvHeap->GetCPUDescriptorHandleForHeapStart(),
            frameIndex * ( LV_NUM_GBUFFER_RTV + 1 ) + i,
            rtvDescriptorSize
        );
    }
    const float clearColor[] = { 0.f, 0.f, 0.f, 1.0f };

    currentFrameResource->ResetCommandListsAndAllocators();
    {
        ID3D12GraphicsCommandList* geometryBufferCommandList = currentFrameResource->geometryBufferCommandList.Get();
        PIXBeginEvent( geometryBufferCommandList, 0, L"Worker drawing scene pass..." );

        for ( UINT i = 0; i < LV_NUM_GBUFFER_RTV; ++i )
        {
            currentFrameResource->commandLists[ LV_COMMAND_LIST_PRE ]->ClearRenderTargetView(
                rtvHandle,
                clearColor,
                0,
                nullptr
            );
            rtvHandle.Offset( 1, rtvDescriptorSize );
        }

        //clear depth-stencil view (dsv)
        currentFrameResource->commandLists[ LV_COMMAND_LIST_PRE ]->ClearDepthStencilView(
            dsvHeap->GetCPUDescriptorHandleForHeapStart(),
            D3D12_CLEAR_FLAG_DEPTH,
            1.0f,
            0,
            0,
            nullptr
        );
        ThrowIfFailed( currentFrameResource->commandLists[ LV_COMMAND_LIST_PRE ]->Close() );


        SetGBufferPSO( geometryBufferCommandList );
        currentFrameResource->BindGBuffer( gBufferHandles, LV_NUM_GBUFFER_RTV, &dsvHandle );
        geometryBufferCommandList->DrawIndexedInstanced( verticesCount, 1, 0, 0, 0 );

        //transition our depth stencil view
        geometryBufferCommandList->ResourceBarrier( 1, &CD3DX12_RESOURCE_BARRIER::Transition(
            depthStencilView.Get(),
            D3D12_RESOURCE_STATE_DEPTH_WRITE,
            D3D12_RESOURCE_STATE_GENERIC_READ
        ) );


        PIXEndEvent( geometryBufferCommandList );
        ThrowIfFailed( geometryBufferCommandList->Close() );
    }


    {
        ID3D12GraphicsCommandList* deferredCommandList = currentFrameResource->deferredCommandList.Get();
        PIXBeginEvent( deferredCommandList, 0, L"Worker deferred pass..." );

        currentFrameResource->SwapBarriers();
        currentFrameResource->commandLists[ LV_COMMAND_LIST_MID ]->ResourceBarrier( 1, &CD3DX12_RESOURCE_BARRIER::Transition(
            renderTargets[ 1 - frameIndex ].Get(),
            D3D12_RESOURCE_STATE_PRESENT,
            D3D12_RESOURCE_STATE_RENDER_TARGET
        ) );
        const float clearColor2[] = { 0.392f, 0.584f, 0.929f, 1.0f };
        currentFrameResource->commandLists[ LV_COMMAND_LIST_MID ]->ClearRenderTargetView(
            rtvHandle,
            clearColor2,
            0,
            nullptr
        );

        ThrowIfFailed( currentFrameResource->commandLists[ LV_COMMAND_LIST_MID ]->Close() );


        SetLightPassPSO( deferredCommandList );
        currentFrameResource->BindDeferred( &rtvHandle, &dsvHandle );
        deferredCommandList->DrawInstanced( 4, 1, 0, 0 );
        currentFrameResource->Finish();
        //transition our depth stencil view
        currentFrameResource->commandLists[ LV_COMMAND_LIST_POST ]->ResourceBarrier( 1, &CD3DX12_RESOURCE_BARRIER::Transition(
            depthStencilView.Get(),
            D3D12_RESOURCE_STATE_GENERIC_READ,
            D3D12_RESOURCE_STATE_DEPTH_WRITE
        ) );
        currentFrameResource->commandLists[LV_COMMAND_LIST_POST]->ResourceBarrier( 1, &CD3DX12_RESOURCE_BARRIER::Transition(
            renderTargets[1 - frameIndex].Get(),
            D3D12_RESOURCE_STATE_RENDER_TARGET,
            D3D12_RESOURCE_STATE_PRESENT
        ) );
        ThrowIfFailed( currentFrameResource->commandLists[LV_COMMAND_LIST_POST]->Close() );

        PIXEndEvent( deferredCommandList );
        ThrowIfFailed( deferredCommandList->Close() );
    }

    commandQueue->ExecuteCommandLists(
        _countof( currentFrameResource->batchedCommandList ),
        currentFrameResource->batchedCommandList 
    );

    PIXBeginEvent( commandQueue.Get(), 0, L"Presenting to screen" );
    ThrowIfFailed( swapChain->Present( 1, 0 ) );
    PIXEndEvent( commandQueue.Get() );
    frameIndex = swapChain->GetCurrentBackBufferIndex();

    currentFrameResource->fenceValue = fenceValue;
    ThrowIfFailed( commandQueue->Signal( fence.Get(), fenceValue ) );
    fenceValue++;
}

inline HRESULT GraphicsCore::InitDeviceCommandQueueSwapChain()
{
    UINT dxgiFactoryFlags = 0;

#if defined(_DEBUG)
    //enable debug layer if needed (must do before device creation)
    ComPtr<ID3D12Debug> debugController;
    if ( SUCCEEDED( D3D12GetDebugInterface( IID_PPV_ARGS( &debugController ) ) ) )
    {
        debugController->EnableDebugLayer();
        dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
    }
#endif

    //create the factory
    ComPtr<IDXGIFactory4> factory;
    ThrowIfFailed( CreateDXGIFactory2( dxgiFactoryFlags, IID_PPV_ARGS( &factory ) ) );

    //try creating device with hardware first then use WARP
    if ( FAILED( D3D12CreateDevice( nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS( &device ) ) ) )
    {
        //use a warp device
        ComPtr<IDXGIAdapter> warpAdapter;
        ThrowIfFailed( factory->EnumWarpAdapter( IID_PPV_ARGS( &warpAdapter ) ) );
        ThrowIfFailed( D3D12CreateDevice(
            warpAdapter.Get(),
            D3D_FEATURE_LEVEL_11_0,
            IID_PPV_ARGS( &device )
        ) );
    }
    NAME_D3D12_OBJECT( device );


    //describe the main queue
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    //create it
    ThrowIfFailed( device->CreateCommandQueue( &queueDesc, IID_PPV_ARGS( &commandQueue ) ) );
    NAME_D3D12_OBJECT( commandQueue );

    //describe the swap chain
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { };
    swapChainDesc.BufferCount = LV_FRAME_COUNT;
    swapChainDesc.Width = windowWidth; // adjust width
    swapChainDesc.Height = windowHeight; // adjust Height
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count = 1;

    //create and copy over swapchain
    ComPtr<IDXGISwapChain1> tempSwapChain;
    ThrowIfFailed( factory->CreateSwapChainForHwnd(
        commandQueue.Get(),
        hWindow,
        &swapChainDesc,
        nullptr,
        nullptr,
        &tempSwapChain
    ) );
    ThrowIfFailed( tempSwapChain.As( &swapChain ) );

    return S_OK;
}

inline HRESULT GraphicsCore::InitRootSignature()
{
    D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = { };
    featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

    if ( FAILED( device->CheckFeatureSupport(
        D3D12_FEATURE_ROOT_SIGNATURE,
        &featureData,
        sizeof( featureData ) ) ) )
    {
        featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
    }

    //this should be ordered from most to least frequent
    CD3DX12_DESCRIPTOR_RANGE1 descriptorRanges[ 2 ];
    CD3DX12_ROOT_PARAMETER1		rootParameters[ 2 ];

    //Init: Albedo + Normal + Position
    descriptorRanges[ 0 ].Init(
        D3D12_DESCRIPTOR_RANGE_TYPE_SRV,            //type of descriptor
        LV_NUM_GBUFFER_RTV,                         //number of descriptors
        0,                                          //base shader register
        0                                           //space in register
    );

    //Deferred SRVs
    rootParameters[ 0 ].InitAsDescriptorTable(
        1,             //number of descriptor ranges
        &descriptorRanges[ 0 ],           //address
        D3D12_SHADER_VISIBILITY_PIXEL   //what it's visible to
    );

    //constant buffer
    descriptorRanges[ 1 ].Init( D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
        1,
        0,
        0,
        D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC
    );
    rootParameters[ 1 ].InitAsDescriptorTable( 1,
        &descriptorRanges[ 1 ],
        D3D12_SHADER_VISIBILITY_ALL
    );

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Init_1_1(
        _countof( rootParameters ),
        rootParameters,
        0,
        nullptr,
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
    );

    ComPtr<ID3DBlob> signature;
    ComPtr<ID3DBlob> error;

    HRESULT result = D3DX12SerializeVersionedRootSignature(
        &rootSignatureDesc,
        featureData.HighestVersion,
        &signature,
        &error
    );

    if ( error != NULL )
    {
        //print this // static_cast<char*>(error->GetBufferPointer());
        ThrowIfFailed( S_FALSE );
    }


    ThrowIfFailed( device->CreateRootSignature(
        0,
        signature->GetBufferPointer(),
        signature->GetBufferSize(),
        IID_PPV_ARGS( &rootSignature )
    ) );
    NAME_D3D12_OBJECT( rootSignature );


    return S_OK;
}

inline HRESULT GraphicsCore::InitPSO()
{
    ComPtr<ID3DBlob> vs;
    ComPtr<ID3DBlob> ps;

    D3DReadFileToBlob( L"Assets/Shaders/vs_basic.cso", &vs );
    D3DReadFileToBlob( L"Assets/Shaders/ps_basic.cso", &ps );

    //input from our vertices
    D3D12_INPUT_ELEMENT_DESC vertexInputDescription[] = {
        {
            "POSITION",                                     //semantic name                         
            0,                                              //semantic index
            DXGI_FORMAT_R32G32B32_FLOAT,                    //format of data
            0,                                              //input slot
            0,                                              //the offset
            D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,     //input classification
            0                                               //istance rate
        },
        {
            "TEXCOORD",
            0,
            DXGI_FORMAT_R32G32_FLOAT,
            0,
            12,
            D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
            0
        },
        {
            "NORMAL",
            0,
            DXGI_FORMAT_R32G32B32_FLOAT,
            0,
            20,
            D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
            0
        },
    };

    //build the input layout
    D3D12_INPUT_LAYOUT_DESC inputLayoutDescription;
    inputLayoutDescription.pInputElementDescs = vertexInputDescription;
    inputLayoutDescription.NumElements = _countof( vertexInputDescription );

    //build out our depth stencil description
    CD3DX12_DEPTH_STENCIL_DESC1 depthStencilDesc( D3D12_DEFAULT );
    depthStencilDesc.DepthEnable = true;
    depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    depthStencilDesc.StencilEnable = FALSE;

    //describe our PSO
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = { };
    psoDesc.InputLayout = inputLayoutDescription;
    psoDesc.pRootSignature = rootSignature.Get();
    psoDesc.VS = CD3DX12_SHADER_BYTECODE( vs.Get() );
    psoDesc.PS = CD3DX12_SHADER_BYTECODE( ps.Get() );
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC( D3D12_DEFAULT );
    psoDesc.BlendState = CD3DX12_BLEND_DESC( D3D12_DEFAULT );
    psoDesc.DepthStencilState = depthStencilDesc;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 3;
    psoDesc.RTVFormats[ 0 ] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.RTVFormats[ 1 ] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.RTVFormats[ 2 ] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    psoDesc.SampleDesc.Count = 1;

    //create our PSO
    ThrowIfFailed( device->CreateGraphicsPipelineState(
        &psoDesc,
        IID_PPV_ARGS( &pso )
    ) );
    NAME_D3D12_OBJECT( pso );

    return S_OK;
}

inline HRESULT GraphicsCore::InitLightPassPSO()
{
    ComPtr<ID3DBlob> vs;
    ComPtr<ID3DBlob> ps;

    D3DReadFileToBlob( L"Assets/Shaders/vs_FSQ.cso", &vs );
    D3DReadFileToBlob( L"Assets/Shaders/ps_lighting.cso", &ps );


    static D3D12_INPUT_ELEMENT_DESC screenQuadVertexDesc[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        {
            "NORMAL",
            0,
            DXGI_FORMAT_R32G32B32_FLOAT,
            0,
            20,
            D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
            0
        },
    };

    //describe the PSO
    D3D12_GRAPHICS_PIPELINE_STATE_DESC lightPsoDesc{};
    lightPsoDesc.VS = CD3DX12_SHADER_BYTECODE( vs.Get() );
    lightPsoDesc.PS = CD3DX12_SHADER_BYTECODE( ps.Get() );
    lightPsoDesc.InputLayout.pInputElementDescs = screenQuadVertexDesc;
    lightPsoDesc.InputLayout.NumElements = _countof( screenQuadVertexDesc );
    lightPsoDesc.pRootSignature = rootSignature.Get();
    lightPsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC( D3D12_DEFAULT );
    lightPsoDesc.DepthStencilState.DepthEnable = false;
    lightPsoDesc.BlendState = CD3DX12_BLEND_DESC( D3D12_DEFAULT );
    lightPsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC( D3D12_DEFAULT );
    lightPsoDesc.RasterizerState.DepthClipEnable = false;
    lightPsoDesc.SampleMask = UINT_MAX;
    lightPsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    lightPsoDesc.NumRenderTargets = 2;
    //descPipelineState.RTVFormats[0] = mRtvFormat[0];
    lightPsoDesc.RTVFormats[ 0 ] = DXGI_FORMAT_R8G8B8A8_UNORM;
    lightPsoDesc.SampleDesc.Count = 1;

    //create our PSO
    ThrowIfFailed( device->CreateGraphicsPipelineState(
        &lightPsoDesc,
        IID_PPV_ARGS( &lightPso )
    ) );
    NAME_D3D12_OBJECT( lightPso );
    return S_OK;
}

inline HRESULT GraphicsCore::InitRtvHeap()
{
    // Create heap of render target descriptors
    // 1 - Albedo
    // 2 - Normal
    // 3 - Position
    // 4 - Back Buffer

    //create actual heap
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = LV_FRAME_COUNT * ( LV_NUM_GBUFFER_RTV + 1 );
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    ThrowIfFailed( device->CreateDescriptorHeap(
        &rtvHeapDesc,
        IID_PPV_ARGS( &rtvHeap )
    ) );
    NAME_D3D12_OBJECT( rtvHeap );

    //cache size 
    rtvDescriptorSize = device->GetDescriptorHandleIncrementSize( D3D12_DESCRIPTOR_HEAP_TYPE_RTV );

    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = { };
    rtvDesc.Texture2D.MipSlice = 0;
    rtvDesc.Texture2D.PlaneSlice = 0;
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
    rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(
        rtvHeap->GetCPUDescriptorHandleForHeapStart()
    );

    for ( UINT i = 0; i < LV_FRAME_COUNT; ++i )
    {
        for( UINT j = 0; j < LV_NUM_GBUFFER_RTV; ++j )
            rtvHandle.Offset( 1, rtvDescriptorSize );

        ThrowIfFailed( swapChain->GetBuffer( 1 - i, IID_PPV_ARGS( &renderTargets[i] ) ) );
        device->CreateRenderTargetView(
            renderTargets[i].Get(),
            &rtvDesc,
            rtvHandle );
        NAME_D3D12_OBJECT_INDEXED( renderTargets, i );

        rtvHandle.Offset( 1, rtvDescriptorSize );
    }

    return S_OK;
}

inline HRESULT GraphicsCore::InitDepthStencil()
{
    //describe and create descriptor for the DSV heap
    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDescriptor = {};
    dsvHeapDescriptor.NumDescriptors = 1;
    dsvHeapDescriptor.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDescriptor.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    ThrowIfFailed( device->CreateDescriptorHeap(
        &dsvHeapDescriptor,
        IID_PPV_ARGS( &dsvHeap )
    ) );
    NAME_D3D12_OBJECT( dsvHeap );

    //describe the depth stencil view
    D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = { };
    depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
    depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

    //create the clear value for depth
    D3D12_CLEAR_VALUE depthOptimizedClearValue = { };
    depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
    depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
    depthOptimizedClearValue.DepthStencil.Stencil = 0;

    //create the depth stencil view
    ThrowIfFailed( device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_DEFAULT ),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Tex2D(
            DXGI_FORMAT_D32_FLOAT,
            windowWidth,
            windowHeight,
            1,
            0,
            1,
            0,
            D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
        ),
        D3D12_RESOURCE_STATE_DEPTH_WRITE,
        &depthOptimizedClearValue,
        IID_PPV_ARGS( &depthStencilView )
    ) );

    device->CreateDepthStencilView(
        depthStencilView.Get(),
        &depthStencilDesc,
        dsvHeap->GetCPUDescriptorHandleForHeapStart()
    );
    NAME_D3D12_OBJECT( depthStencilView );

    return S_OK;
}

inline HRESULT GraphicsCore::InitViewportScissorRectangle()
{
    //construct a new viewport
    viewport = CD3DX12_VIEWPORT(
        0.0f,
        0.0f,
        static_cast<float>( windowWidth ),
        static_cast<float>( windowHeight )
    );

    //construct a scissor rect
    scissorRect = CD3DX12_RECT(
        0,
        0,
        static_cast<LONG>( windowWidth ),
        static_cast<LONG>( windowHeight )
    );

    return S_OK;
}

inline HRESULT GraphicsCore::InitInputShaderResources()
{
    //create command allocator
    ComPtr<ID3D12CommandAllocator> commandAllocator;
    ThrowIfFailed( device->CreateCommandAllocator(
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        IID_PPV_ARGS( &commandAllocator )
    ) );
    NAME_D3D12_OBJECT( commandAllocator );

    //create a command list
    ComPtr<ID3D12GraphicsCommandList> commandList;
    ThrowIfFailed( device->CreateCommandList(
        0,
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        commandAllocator.Get(),
        pso.Get(),
        IID_PPV_ARGS( &commandList )
    ) );
    NAME_D3D12_OBJECT( commandList );

    PIXBeginEvent( commandList.Get(), 0, L"Resetting commandlist" );
    commandList->Close();
    commandAllocator->Reset();
    commandList->Reset( commandAllocator.Get(), pso.Get() );
    PIXEndEvent( commandList.Get() );

    std::vector<Vertex>* vertices = new std::vector<Vertex>();
    std::vector<uint16_t>* indices = new std::vector<uint16_t>();
    ObjLoader::LoadObj( vertices, indices, "Assets/Models/bus.obj" );

    //make vertex buffer for 'n' floats
    UINT vertexDataSize = vertices->size() * sizeof( Vertex );
    UINT vertexDataOffset = 0;
    UINT vertexStride = sizeof( Vertex );
    UINT indexDataSize = indices->size() * sizeof( uint16_t );
    UINT indexDataOffset = 0;

    verticesCount = indices->size();

    //vertex buffer(s)
    {
        //create vertex buffer
        ThrowIfFailed( device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_DEFAULT ),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer( vertexDataSize ),
            D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr,
            IID_PPV_ARGS( &vertexBuffer )
        ) );
        NAME_D3D12_OBJECT( vertexBuffer );

        //create vertex upload buffer
        ThrowIfFailed( device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_UPLOAD ),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer( vertexDataSize ),
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS( &vertexBufferUpload )
        ) );

        D3D12_SUBRESOURCE_DATA vertexData = { };
        vertexData.pData = &( ( *vertices )[ 0 ] ) + vertexDataOffset;
        vertexData.RowPitch = vertexDataSize;
        vertexData.SlicePitch = vertexData.RowPitch;


        //this looks like it's pre-emptively loading all the static geometry 
        PIXBeginEvent( commandList.Get(), 0, L"Copy vertex buffer to default resource" );
        UpdateSubresources<1>(
            commandList.Get(),
            vertexBuffer.Get(),
            vertexBufferUpload.Get(),
            0,
            0,
            1,
            &vertexData
            );
        commandList->ResourceBarrier(
            1,
            &CD3DX12_RESOURCE_BARRIER::Transition(
                vertexBuffer.Get(),
                D3D12_RESOURCE_STATE_COPY_DEST,
                D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER
            )
        );
        PIXEndEvent( commandList.Get() );


        vertexBufferView = { };
        vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
        vertexBufferView.SizeInBytes = vertexDataSize;
        vertexBufferView.StrideInBytes = vertexStride;

#pragma region FSQVertexBufferInit
        MeshData screenQuad = {};
        ObjLoader objl;
        objl.GenerateFullScreenQuad( screenQuad );

        //make vertex buffer for 'n' floats
        UINT fsqVertexDataSize = screenQuad.vertices.size() * sizeof( Vertex );
        UINT fsqVertexDataOffset = 0;
        UINT fsqVertexStride = sizeof( Vertex );

        //create FSQ vertex buffer
        ThrowIfFailed( device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_DEFAULT ),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer( fsqVertexDataSize ),
            D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr,
            IID_PPV_ARGS( &fsqVertexBuffer )
        ) );
        NAME_D3D12_OBJECT( fsqVertexBuffer );

        //create vertex upload buffer
        ThrowIfFailed( device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_UPLOAD ),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer( fsqVertexDataSize ),
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS( &fsqVertexBufferUpload )
        ) );

        D3D12_SUBRESOURCE_DATA fsqVertexData = { };
        fsqVertexData.pData = &( ( screenQuad.vertices )[ 0 ] ) + fsqVertexDataOffset;
        fsqVertexData.RowPitch = fsqVertexDataSize;
        fsqVertexData.SlicePitch = fsqVertexData.RowPitch;


        //this looks like it's pre-emptively loading all the static geometry 
        PIXBeginEvent( commandList.Get(), 0, L"Copy vertex buffer to default resource" );
        UpdateSubresources<1>(
            commandList.Get(),
            fsqVertexBuffer.Get(),
            fsqVertexBufferUpload.Get(),
            0,
            0,
            1,
            &fsqVertexData
            );
        commandList->ResourceBarrier(
            1,
            &CD3DX12_RESOURCE_BARRIER::Transition(
                fsqVertexBuffer.Get(),
                D3D12_RESOURCE_STATE_COPY_DEST,
                D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER
            )
        );
        PIXEndEvent( commandList.Get() );


        fsqVertexBufferView = { };
        fsqVertexBufferView.BufferLocation = fsqVertexBuffer->GetGPUVirtualAddress();
        fsqVertexBufferView.SizeInBytes = fsqVertexDataSize;
        fsqVertexBufferView.StrideInBytes = fsqVertexStride;
#pragma endregion

    }

    //index buffer
    {
        //create index buffer
        ThrowIfFailed( device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_DEFAULT ),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer( indexDataSize ),
            D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr,
            IID_PPV_ARGS( &indexBuffer )
        ) );
        NAME_D3D12_OBJECT( indexBuffer );

        //create vertex upload buffer
        ThrowIfFailed( device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_UPLOAD ),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer( indexDataSize ),
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS( &indexBufferUpload )
        ) );

        D3D12_SUBRESOURCE_DATA indexData = { };
        indexData.pData = &( ( *indices )[ 0 ] ) + indexDataOffset;
        indexData.RowPitch = indexDataSize;
        indexData.SlicePitch = indexData.RowPitch;

        //this looks like it's pre-emptively loading all the static geometry 
        PIXBeginEvent( commandList.Get(), 0, L"Copy index buffer to default resource" );
        UpdateSubresources<1>(
            commandList.Get(),
            indexBuffer.Get(),
            indexBufferUpload.Get(),
            0,
            0,
            1,
            &indexData
            );
        commandList->ResourceBarrier(
            1,
            &CD3DX12_RESOURCE_BARRIER::Transition(
                indexBuffer.Get(),
                D3D12_RESOURCE_STATE_COPY_DEST,
                D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER
            )
        );
        PIXEndEvent( commandList.Get() );

        indexBufferView = { };
        indexBufferView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
        indexBufferView.SizeInBytes = indexDataSize;
        indexBufferView.Format = DXGI_FORMAT_R16_UINT;
    }

    delete vertices;
    delete indices;

    // Describe and create a shader resource view (SRV) and constant 
    // buffer view (CBV) descriptor heap.  Heap layout: null views, 
    // frame 1's constant buffer, frame 2's constant buffers, etc...
    const UINT nullSrvCount = 0;		// Null descriptors are needed for out of bounds behavior reads.
    const UINT cbvCount = LV_FRAME_COUNT * 1; //Frame Count * Number of CBVs
    const UINT srvCount = LV_FRAME_COUNT * LV_NUM_GBUFFER_RTV; // _countof(SampleAssets::Textures) + (FrameCount * 1);
    D3D12_DESCRIPTOR_HEAP_DESC cbvSrvHeapDesc = {};
    cbvSrvHeapDesc.NumDescriptors = nullSrvCount + cbvCount + srvCount;
    cbvSrvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    cbvSrvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    ThrowIfFailed( device->CreateDescriptorHeap( &cbvSrvHeapDesc, IID_PPV_ARGS( &cbvSrvHeap ) ) );
    NAME_D3D12_OBJECT( cbvSrvHeap );

    CD3DX12_CPU_DESCRIPTOR_HANDLE cbvSrvHandle( cbvSrvHeap->GetCPUDescriptorHandleForHeapStart() );
    cbvSrvDescriptorSize = device->GetDescriptorHandleIncrementSize( D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV );

    {

    }

    //close the command list and transfer static data
    ThrowIfFailed( commandList->Close() );

    ID3D12CommandList* ppCommandList[] = { commandList.Get() };
    PIXBeginEvent( commandQueue.Get(), 0, L"Executing commandlist" );
    commandQueue->ExecuteCommandLists( _countof( ppCommandList ), ppCommandList );
    PIXEndEvent( commandQueue.Get() );

    const UINT64 fenceToWaitFor = fenceValue;
    ThrowIfFailed( commandQueue->Signal( fence.Get(), fenceToWaitFor ) );
    fenceValue++;

    ThrowIfFailed( fence->SetEventOnCompletion( fenceToWaitFor, fenceEvent ) );
    WaitForSingleObject( fenceEvent, INFINITE );

    return S_OK;
}

inline HRESULT GraphicsCore::InitFrameResources()
{
    for ( int i = 0; i < LV_FRAME_COUNT; i++ )
    {
        frameResources[ i ] = new FrameResource(
            device.Get(),
            pso.Get(),
            lightPso.Get(),
            dsvHeap.Get(),
            rtvHeap.Get(),
            cbvSrvHeap.Get(),
            swapChain.Get(),
            &viewport,
            i
        );
    }

    currentFrameResourceIndex = 0;
    currentFrameResource = frameResources[ currentFrameResourceIndex ];

    return S_OK;
}

inline HRESULT GraphicsCore::InitSynchronizationObjects()
{
    ThrowIfFailed( device->CreateFence(
        fenceValue,
        D3D12_FENCE_FLAG_NONE,
        IID_PPV_ARGS( &fence )
    ) );
    fenceValue++;

    fenceEvent = CreateEvent( nullptr, FALSE, FALSE, nullptr );
    if ( fenceEvent == nullptr )
        ThrowIfFailed( HRESULT_FROM_WIN32( GetLastError() ) );



    return S_OK;
}

inline void GraphicsCore::PrepareForGeometryPass()
{

}

inline void GraphicsCore::MidFrame()
{
}

inline void GraphicsCore::EndFrame()
{
}

inline void GraphicsCore::SetGBufferPSO( ID3D12GraphicsCommandList * commandList )
{
    commandList->SetGraphicsRootSignature( rootSignature.Get() );
    commandList->SetPipelineState( pso.Get() );
    ID3D12DescriptorHeap* ppHeaps[] = { cbvSrvHeap.Get() };
    commandList->SetDescriptorHeaps( _countof( ppHeaps ), ppHeaps );
    commandList->RSSetViewports( 1, &viewport );
    commandList->RSSetScissorRects( 1, &scissorRect );
    commandList->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
    commandList->IASetVertexBuffers( 0, 1, &vertexBufferView );
    commandList->IASetIndexBuffer( &indexBufferView );
    commandList->OMSetStencilRef( 0 );
}

inline void GraphicsCore::SetLightPassPSO( ID3D12GraphicsCommandList * commandList )
{
    commandList->SetGraphicsRootSignature( rootSignature.Get() );
    commandList->SetPipelineState( lightPso.Get() );
    ID3D12DescriptorHeap* ppHeaps[] = { cbvSrvHeap.Get() };
    commandList->SetDescriptorHeaps( _countof( ppHeaps ), ppHeaps );
    commandList->RSSetViewports( 1, &viewport );
    commandList->RSSetScissorRects( 1, &scissorRect );
    commandList->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );
    commandList->IASetVertexBuffers( 0, 1, &fsqVertexBufferView );
    commandList->OMSetStencilRef( 0 );
}
