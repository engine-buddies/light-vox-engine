#include "GraphicsCore.h"
#include "FrameResource.h"
#include "Camera.h"
#include "ShaderDefinitions.h"    //Utils for initialization
#include <vector>

#include "ObjLoader.h"

//needed for 'right now'

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
    for ( size_t i = 0; i < LV_FRAME_COUNT; ++i )
        delete frameResources[ i ];
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
    ThrowIfFailed( InitRTV() );
    ThrowIfFailed( InitDepthStencil() );
    ThrowIfFailed( InitViewportScissorRectangle() );
    ThrowIfFailed( InitInputShaderResources() );
    ThrowIfFailed( InitFrameResources() );
    ThrowIfFailed( InitSynchronizationObjects() );

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
    BeginFrame();

    //init worker threads?
    ID3D12GraphicsCommandList* pSceneCommandList = currentFrameResource->sceneCommandLists[ 0 ].Get();

    SetCommonPipelineState( pSceneCommandList );
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(
        rtvHeap->GetCPUDescriptorHandleForHeapStart(),
        frameIndex,
        rtvDescriptorSize
    );
    CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(
        dsvHeap->GetCPUDescriptorHandleForHeapStart()
    );
    currentFrameResource->Bind( pSceneCommandList, TRUE, &rtvHandle, &dsvHandle );

    PIXBeginEvent( pSceneCommandList, 0, L"Worker drawing scene pass..." );

    D3D12_GPU_DESCRIPTOR_HANDLE cbvSrvHeapStart = cbvSrvHeap->GetGPUDescriptorHandleForHeapStart();
    const UINT cbvSrvDescriptorSize = device->GetDescriptorHandleIncrementSize( D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV );
    const UINT nullSrvCount = 2;

    pSceneCommandList->DrawIndexedInstanced( verticesCount, LV_MAX_INSTANCE_COUNT, 0, 0, 0 );

    PIXEndEvent( pSceneCommandList );
    ThrowIfFailed( pSceneCommandList->Close() );

    MidFrame();
    EndFrame();

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
    swapChainDesc.BufferCount = LV_FRAME_COUNT; //frame count
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
    CD3DX12_ROOT_PARAMETER1		rootParameters[ 3 ];

    rootParameters[ 0 ].InitAsShaderResourceView(
        0,                              //register
        1
    );

    //diffuse + normal SRV
    descriptorRanges[ 0 ].Init(
        D3D12_DESCRIPTOR_RANGE_TYPE_SRV,            //type of descriptor
        2,                                          //number of descriptors
        1,                                          //base shader register
        0,                                          //space in register
        D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC     //data flag
    );
    rootParameters[ 1 ].InitAsDescriptorTable(
        1,                              //number of descriptor ranges
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
    rootParameters[ 2 ].InitAsDescriptorTable( 1,
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

    ThrowIfFailed( D3DX12SerializeVersionedRootSignature(
        &rootSignatureDesc,
        featureData.HighestVersion,
        &signature,
        &error
    ) );
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
    D3D12_INPUT_ELEMENT_DESC vertexInputDescription[ 3 ];
    ShaderDefinitions::SetInputLayout( vertexInputDescription );

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
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[ 0 ] = DXGI_FORMAT_R8G8B8A8_UNORM;
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

inline HRESULT GraphicsCore::InitRTV()
{
    // Create heap of render target descriptors
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = LV_FRAME_COUNT;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    ThrowIfFailed( device->CreateDescriptorHeap(
        &rtvHeapDesc,
        IID_PPV_ARGS( &rtvHeap )
    ) );
    NAME_D3D12_OBJECT( rtvHeap );

    rtvDescriptorSize = device->GetDescriptorHandleIncrementSize( D3D12_DESCRIPTOR_HEAP_TYPE_RTV );

    // Create frame resources.
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle( rtvHeap->GetCPUDescriptorHandleForHeapStart() );

    // Create an RTV for each frame.
    for ( UINT n = 0; n < LV_FRAME_COUNT; n++ )
    {
        ThrowIfFailed( swapChain->GetBuffer( n, IID_PPV_ARGS( &renderTargets[ n ] ) ) );
        device->CreateRenderTargetView( renderTargets[ n ].Get(), nullptr, rtvHandle );
        rtvHandle.Offset( 1, rtvDescriptorSize );
        NAME_D3D12_OBJECT_INDEXED( renderTargets, n );
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

    std::vector<Vertex>* vertices = new std::vector<Vertex>();
    std::vector<uint16_t>* indices = new std::vector<uint16_t>();
    ObjLoader::LoadObj( vertices, indices, "Assets/Models/voxel.obj" );

    //make vertex buffer for 'n' floats
    UINT vertexDataSize = static_cast<UINT>(vertices->size()) * sizeof( Vertex );
    UINT vertexDataOffset = 0;
    UINT vertexStride = sizeof( Vertex );
    UINT indexDataSize = static_cast<UINT>(indices->size()) * sizeof( uint16_t );
    UINT indexDataOffset = 0;

    verticesCount = static_cast<UINT>(indices->size());

    //vertex buffer
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
    }

    //index buffer
    {
        //create vertex buffer
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
    // frame 1's constant buffer, frame 1's SRVs,
    // frame 2's constant buffers, frame 2's SRVs, etc...
    const UINT nullSrvCount = 0;		// Null descriptors are needed for out of bounds behavior reads.
    const UINT cbvCount = 1 * LV_FRAME_COUNT;
    const UINT srvCount = 0 * LV_FRAME_COUNT; // _countof(SampleAssets::Textures) + (FrameCount * 1);
    D3D12_DESCRIPTOR_HEAP_DESC cbvSrvHeapDesc = {};
    cbvSrvHeapDesc.NumDescriptors = nullSrvCount + cbvCount + srvCount;
    cbvSrvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    cbvSrvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    ThrowIfFailed( device->CreateDescriptorHeap( &cbvSrvHeapDesc, IID_PPV_ARGS( &cbvSrvHeap ) ) );
    NAME_D3D12_OBJECT( cbvSrvHeap );

    CD3DX12_CPU_DESCRIPTOR_HANDLE cbvSrvHandle( cbvSrvHeap->GetCPUDescriptorHandleForHeapStart() );
    const UINT cbvSrvDescriptorSize = device->GetDescriptorHandleIncrementSize( D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV );

    {
        // Describe and create 2 null SRVs. Null descriptors are needed in order 
        // to achieve the effect of an "unbound" resource.
        D3D12_SHADER_RESOURCE_VIEW_DESC nullSrvDesc = {};
        nullSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        nullSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        nullSrvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        nullSrvDesc.Texture2D.MipLevels = 1;
        nullSrvDesc.Texture2D.MostDetailedMip = 0;
        nullSrvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

        for ( size_t i = 0; i < nullSrvCount; ++i )
        {
            device->CreateShaderResourceView( nullptr, &nullSrvDesc, cbvSrvHandle );
            cbvSrvHandle.Offset( cbvSrvDescriptorSize );
        }
    }

    //close the command list and transfer static data
    ThrowIfFailed( commandList->Close() );
    ID3D12CommandList* ppCommandList[] = { commandList.Get() };
    commandQueue->ExecuteCommandLists( _countof( ppCommandList ), ppCommandList );

    return S_OK;
}

inline HRESULT GraphicsCore::InitFrameResources()
{
    for ( UINT i = 0; i < LV_FRAME_COUNT; i++ )
    {
        frameResources[ i ] = new FrameResource(
            device.Get(),
            pso.Get(),
            dsvHeap.Get(),
            cbvSrvHeap.Get(),
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

    const UINT64 fenceToWaitFor = fenceValue;
    ThrowIfFailed( commandQueue->Signal( fence.Get(), fenceToWaitFor ) );
    fenceValue++;

    ThrowIfFailed( fence->SetEventOnCompletion( fenceToWaitFor, fenceEvent ) );
    WaitForSingleObject( fenceEvent, INFINITE );

    return S_OK;
}

inline void GraphicsCore::BeginFrame()
{
    currentFrameResource->Init();

    //transition back buffer to be used as render target
    currentFrameResource->commandLists[ LV_COMMAND_LIST_PRE ]->ResourceBarrier(
        1,
        &CD3DX12_RESOURCE_BARRIER::Transition(
            renderTargets[ frameIndex ].Get(),
            D3D12_RESOURCE_STATE_PRESENT,
            D3D12_RESOURCE_STATE_RENDER_TARGET
        )
    );

    //clear back buffer with c o r n f l o w e r   b l u e
    const float clearColor[] = { 0.392f, 0.584f, 0.929f, 1.0f };
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(
        rtvHeap->GetCPUDescriptorHandleForHeapStart(),
        frameIndex,
        rtvDescriptorSize
    );
    currentFrameResource->commandLists[ LV_COMMAND_LIST_PRE ]->ClearRenderTargetView(
        rtvHandle,
        clearColor,
        0,
        nullptr
    );

    //clear depth buffer
    currentFrameResource->commandLists[ LV_COMMAND_LIST_PRE ]->ClearDepthStencilView(
        dsvHeap->GetCPUDescriptorHandleForHeapStart(),
        D3D12_CLEAR_FLAG_DEPTH,
        1.0f,
        0,
        0,
        nullptr
    );

    //close the 'pre' command list
    ThrowIfFailed( currentFrameResource->commandLists[ LV_COMMAND_LIST_PRE ]->Close() );
}

inline void GraphicsCore::MidFrame()
{
    currentFrameResource->SwapBarriers();
    ThrowIfFailed( currentFrameResource->commandLists[ LV_COMMAND_LIST_MID ]->Close() );
}

inline void GraphicsCore::EndFrame()
{
    currentFrameResource->Finish();
    currentFrameResource->commandLists[ LV_COMMAND_LIST_POST ]->ResourceBarrier(
        1,
        &CD3DX12_RESOURCE_BARRIER::Transition(
            renderTargets[ frameIndex ].Get(),
            D3D12_RESOURCE_STATE_RENDER_TARGET,
            D3D12_RESOURCE_STATE_PRESENT
        )
    );
    ThrowIfFailed( currentFrameResource->commandLists[ LV_COMMAND_LIST_POST ]->Close() );
}

inline void GraphicsCore::SetCommonPipelineState( ID3D12GraphicsCommandList * commandList )
{
    commandList->SetGraphicsRootSignature( rootSignature.Get() );

    ID3D12DescriptorHeap* ppHeaps[] = { cbvSrvHeap.Get() };
    commandList->SetDescriptorHeaps( _countof( ppHeaps ), ppHeaps );

    commandList->RSSetViewports( 1, &viewport );
    commandList->RSSetScissorRects( 1, &scissorRect );
    commandList->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
    commandList->IASetVertexBuffers( 0, 1, &vertexBufferView );
    commandList->IASetIndexBuffer( &indexBufferView );
    commandList->OMSetStencilRef( 0 );
}