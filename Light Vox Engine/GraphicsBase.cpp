#include "GraphicsCore.h"
#include "FrameResource.h"

GraphicsCore::~GraphicsCore()
{
}

HRESULT GraphicsCore::InitDeviceCommandQueueSwapChain()
{
    UINT dxgiFactoryFlags = 0;

    //enable debug layer if needed (must do before device creation)
#if defined(_DEBUG)
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
    NAME_D3D12_OBJECT_WITH_NAME( commandQueue, "%s", "Main" );

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

HRESULT GraphicsCore::InitViewportScissorRectangle()
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

HRESULT GraphicsCore::InitSynchronizationObjects()
{
    ThrowIfFailed( device->CreateFence(
        fenceValue,
        D3D12_FENCE_FLAG_NONE,
        IID_PPV_ARGS( &fence )
    ) );
    NAME_D3D12_OBJECT( fence );
    fenceValue++;

    fenceEvent = CreateEvent( nullptr, FALSE, FALSE, nullptr );
    if ( fenceEvent == nullptr )
        ThrowIfFailed( HRESULT_FROM_WIN32( GetLastError() ) );

    return S_OK;
}

HRESULT GraphicsCore::InitFrameResources()
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
            &viewport,
            i
        );
    }

    currentFrameResourceIndex = 0;
    currentFrameResource = frameResources[ currentFrameResourceIndex ];

    return S_OK;
}