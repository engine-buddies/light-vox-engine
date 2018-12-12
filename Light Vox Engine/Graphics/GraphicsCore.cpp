#include "GraphicsCore.h"
#include "FrameResource.h"
#include "Camera.h"

//needed for 'right now'
#include "ObjLoader.h"

#include <DDSTextureLoader.h>

using namespace Microsoft::WRL;
using namespace Graphics;

GraphicsCore::GraphicsCore( HWND hWindow, uint32_t windowW, uint32_t windowH )
{
    this->hWindow = hWindow;
    windowWidth = windowW;
    windowHeight = windowH;
    fenceValue = 0;
    fenceFrameIndex = 0;
    currentFrameResourceIndex = 0;
    currentFrameResource = nullptr;
    debugRenderer = DebugRenderer::GetInstance();
}

GraphicsCore::~GraphicsCore()
{
    //wait for the current event fencce to complete
    const uint64_t fenceToWaitFor = fenceValue;
    ThrowIfFailed( commandQueue->Signal( fence.Get(), fenceToWaitFor ) );
    ThrowIfFailed( fence->SetEventOnCompletion( fenceToWaitFor, fenceEvent ) );
    WaitForSingleObject( fenceEvent, INFINITE );

    //deallocate all our frame resources
    for ( size_t i = 0; i < LV_FRAME_COUNT; ++i )
        delete frameResources[ i ];
}

void GraphicsCore::OnResize( uint32_t width, uint32_t height )
{
    //TODO - recreate scssisor rectangle, viewport, and back buffers
}

HRESULT GraphicsCore::Init()
{
    ThrowIfFailed( InitDeviceCommandQueueSwapChain() );
    ThrowIfFailed( InitRootSignature() );
    ThrowIfFailed( InitGeometryPSO() );
    ThrowIfFailed( InitSkyboxPSO() );
#ifdef _DEBUG
    ThrowIfFailed( InitDebugPSO() );
#endif
    ThrowIfFailed( InitLightPassPSO() );
    ThrowIfFailed( InitRtvHeap() );
    ThrowIfFailed( InitDepthStencil() );
    ThrowIfFailed( InitViewportScissorRectangle() );
    ThrowIfFailed( InitSynchronizationObjects() );
    ThrowIfFailed( InitInputShaderResources() );
    ThrowIfFailed( InitFrameResources() );

    return S_OK;
}

HRESULT GraphicsCore::InitFrameResources()
{
    for ( int i = 0; i < LV_FRAME_COUNT; ++i )
    {
        frameResources[ i ] = new FrameResource(
            device.Get(),
            geometryPso.Get(),
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

void GraphicsCore::Update( glm::mat4x4_packed transforms[], Camera* camera )
{
    PIXSetMarker( commandQueue.Get(), 0, L"Getting last completed fence" );

    const uint64_t lastCompletedFence = fence->GetCompletedValue();
    fenceFrameIndex = ( fenceFrameIndex + 1 ) % LV_FRAME_COUNT;
    currentFrameResource = frameResources[ fenceFrameIndex ];

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

#ifdef _DEBUG
    currentFrameResource->WriteDebugInstanceBuffers(
        debugRenderer->GetCubeInstanceDataPtr(),
        debugRenderer->GetCubeInstanceDataCount()
    );
#endif

}

void GraphicsCore::Render()
{
    CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle( dsvHeap->GetCPUDescriptorHandleForHeapStart() );
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle( rtvHeap->GetCPUDescriptorHandleForHeapStart(),
        LV_NUM_RTV_PER_FRAME * currentFrameResourceIndex,
        rtvDescriptorSize );
    CD3DX12_GPU_DESCRIPTOR_HANDLE cbvSrvGpuHandle(cbvSrvHeap->GetGPUDescriptorHandleForHeapStart());

    //reset for current frame
    currentFrameResource->ResetCommandListsAndAllocators();

    //clear all RTVs and DSV as part of init
    {
        ID3D12GraphicsCommandList* initCommandList = currentFrameResource->commandLists[ LV_COMMAND_LIST_INIT ].Get();
        float albedoClear[ 4 ] = LV_RTV_CLEAR_BG_COLOR;
        float blackClear[ 4 ] = LV_RTV_CLEAR_COLOR;
        currentFrameResource->commandLists[ LV_COMMAND_LIST_INIT ]->ClearRenderTargetView(
            rtvHandle,
            albedoClear,
            0,
            nullptr
        );
        rtvHandle.Offset( rtvDescriptorSize );
        for ( uint32_t i = 1; i < LV_NUM_GBUFFER_RTV; ++i )
        {
            currentFrameResource->commandLists[ LV_COMMAND_LIST_INIT ]->ClearRenderTargetView(
                rtvHandle,
                blackClear,
                0,
                nullptr
            );
            rtvHandle.Offset( rtvDescriptorSize );
        }

        //transition the back-bufffer over and clear it
        initCommandList->ResourceBarrier( 1, &CD3DX12_RESOURCE_BARRIER::Transition(
            renderTargets[ currentFrameResourceIndex ].Get(),
            D3D12_RESOURCE_STATE_PRESENT,
            D3D12_RESOURCE_STATE_RENDER_TARGET
        ) );
        initCommandList->ClearRenderTargetView(
            rtvHandle,
            blackClear,
            0,
            nullptr
        );

        //clear depth-stencil view (dsv)
        currentFrameResource->commandLists[ LV_COMMAND_LIST_INIT ]->ClearDepthStencilView(
            dsvHeap->GetCPUDescriptorHandleForHeapStart(),
            D3D12_CLEAR_FLAG_DEPTH,
            1.0f,
            0,
            0,
            nullptr
        );

        ThrowIfFailed( initCommandList->Close() );
    }

    //geometry pass
    {
        ID3D12GraphicsCommandList* geometryBufferCommandList = currentFrameResource->geometryCmdLists[ 0 ].Get();
        PIXBeginEvent( geometryBufferCommandList, 0, L"Worker drawing scene pass..." );

        SetGBufferPSO( geometryBufferCommandList );
        currentFrameResource->BindGBuffer();
        geometryBufferCommandList->DrawIndexedInstanced( verticesCount, LV_MAX_INSTANCE_COUNT, 0, 0, 0 );

        PIXEndEvent( geometryBufferCommandList );
        ThrowIfFailed( geometryBufferCommandList->Close() );
    }

    //lighting pass
    {
        ID3D12GraphicsCommandList* deferredCommandList = currentFrameResource->commandLists[ LV_COMMAND_LIST_LIGHTING_PASS ].Get();
        PIXBeginEvent( deferredCommandList, 0, L"Worker deferred pass..." );

        //transition the DSV over and transition the g-buffer textures over
        currentFrameResource->SwapBarriers();


        //draw onto our FSQ
        SetLightPassPSO( deferredCommandList );
        currentFrameResource->BindDeferred( &rtvHandle, samplerHeap->GetGPUDescriptorHandleForHeapStart() );
        deferredCommandList->DrawInstanced( 4, 1, 0, 0 );

        //Transition and clean-up
        currentFrameResource->Cleanup();

#ifdef _DEBUG
        SetDebugPSO( deferredCommandList );
        currentFrameResource->BindDebug( &rtvHandle );
        deferredCommandList->DrawIndexedInstanced( verticesCount, debugRenderer->GetCubeInstanceDataCount(), 0, 0, 0 );
#endif

        //Render Skybox after all opaque Geometry:
        SetSkyboxPSO(deferredCommandList);
        cbvSrvGpuHandle.Offset(LV_NUM_GBUFFER_RTV + LV_NUM_NULL_SRV + currentFrameResourceIndex * LV_NUM_CBVSRV_PER_FRAME);
        currentFrameResource->BindSkybox(&rtvHandle, samplerHeap->GetGPUDescriptorHandleForHeapStart(), cbvSrvGpuHandle);
        deferredCommandList->DrawIndexedInstanced(skyboxVerticesCount, 1, 0, 0, 0);

        deferredCommandList->ResourceBarrier( 1, &CD3DX12_RESOURCE_BARRIER::Transition(
            renderTargets[ currentFrameResourceIndex ].Get(),
            D3D12_RESOURCE_STATE_RENDER_TARGET,
            D3D12_RESOURCE_STATE_PRESENT
        ) );

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
    currentFrameResourceIndex = swapChain->GetCurrentBackBufferIndex();

    currentFrameResource->fenceValue = fenceValue;
    ThrowIfFailed( commandQueue->Signal( fence.Get(), fenceValue ) );
    ++fenceValue;
}

HRESULT GraphicsCore::InitRootSignature()
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
    CD3DX12_DESCRIPTOR_RANGE1 descriptorRanges[ LV_ROOT_SIGNATURE_COUNT ];
    CD3DX12_ROOT_PARAMETER1		rootParameters[ LV_ROOT_SIGNATURE_COUNT ];

    //Instancing data
    rootParameters[ LV_ROOT_SIGNATURE_INSTANCED_DATA ].InitAsShaderResourceView(
        0,       //register
        1        //space
    );

    //G-Buffer: Albedo + Normal + Position
    descriptorRanges[ LV_ROOT_SIGNATURE_GBUFFER_SRV ].Init(
        D3D12_DESCRIPTOR_RANGE_TYPE_SRV,            //type of descriptor
        LV_NUM_GBUFFER_RTV + LV_NUM_TEXTURES,       //number of descriptors
        0,                                          //base shader register
        0                                           //space in register
    );
    rootParameters[ LV_ROOT_SIGNATURE_GBUFFER_SRV ].InitAsDescriptorTable(
        1,                                                  //number of descriptor ranges
        &descriptorRanges[ LV_ROOT_SIGNATURE_GBUFFER_SRV ], //address
        D3D12_SHADER_VISIBILITY_PIXEL                       //what it's visible to
    );

    //constant buffer
    descriptorRanges[ LV_ROOT_SIGNATURE_CBV ].Init( D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
        1,
        0,
        0,
        D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC
    );
    rootParameters[ LV_ROOT_SIGNATURE_CBV ].InitAsDescriptorTable( 1,
        &descriptorRanges[ LV_ROOT_SIGNATURE_CBV ],
        D3D12_SHADER_VISIBILITY_ALL
    );

    //Sampler
    descriptorRanges[ LV_ROOT_SIGNATURE_SAMPLER ].Init(
        D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER,
        1,
        0
    );
    rootParameters[ LV_ROOT_SIGNATURE_SAMPLER ].InitAsDescriptorTable(
        1,
        &descriptorRanges[ LV_ROOT_SIGNATURE_SAMPLER ],
        D3D12_SHADER_VISIBILITY_PIXEL
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

HRESULT GraphicsCore::InitGeometryPSO()
{
    ComPtr<ID3DBlob> vs;
    ComPtr<ID3DBlob> ps;

    D3DReadFileToBlob( L"Assets/Shaders/vs_basic.cso", &vs );
    D3DReadFileToBlob( L"Assets/Shaders/ps_basic.cso", &ps );

    //build the input layout
    D3D12_INPUT_ELEMENT_DESC vertexInputDescription[ LV_NUM_VS_INPUT_COUNT ];
    ShaderDefinitions::SetGeometryPassInputLayout( vertexInputDescription );

    D3D12_INPUT_LAYOUT_DESC inputLayoutDescription;
    inputLayoutDescription.pInputElementDescs = vertexInputDescription;
    inputLayoutDescription.NumElements = _countof( vertexInputDescription );

    //build out our depth stencil description
    CD3DX12_DEPTH_STENCIL_DESC1 depthStencilDesc( D3D12_DEFAULT );
    depthStencilDesc.DepthEnable = TRUE;
    depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    depthStencilDesc.StencilEnable = TRUE;

    //describe our PSO
    D3D12_GRAPHICS_PIPELINE_STATE_DESC geometryPsoDesc = { };
    geometryPsoDesc.InputLayout = inputLayoutDescription;
    geometryPsoDesc.pRootSignature = rootSignature.Get();
    geometryPsoDesc.VS = CD3DX12_SHADER_BYTECODE( vs.Get() );
    geometryPsoDesc.PS = CD3DX12_SHADER_BYTECODE( ps.Get() );
    geometryPsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC( D3D12_DEFAULT );
    geometryPsoDesc.BlendState = CD3DX12_BLEND_DESC( D3D12_DEFAULT );
    geometryPsoDesc.DepthStencilState = depthStencilDesc;
    geometryPsoDesc.SampleMask = UINT_MAX;
    geometryPsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    geometryPsoDesc.NumRenderTargets = LV_NUM_GBUFFER_RTV;
    geometryPsoDesc.RTVFormats[ 0 ] = ShaderDefinitions::GeometryBufferFormat( 0 );   //albedo
    geometryPsoDesc.RTVFormats[ 1 ] = ShaderDefinitions::GeometryBufferFormat( 1 );   //normal
    geometryPsoDesc.RTVFormats[ 2 ] = ShaderDefinitions::GeometryBufferFormat( 2 );   //position
    geometryPsoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    geometryPsoDesc.SampleDesc.Count = 1;

    //create our PSO
    ThrowIfFailed( device->CreateGraphicsPipelineState(
        &geometryPsoDesc,
        IID_PPV_ARGS( &geometryPso )
    ) );
    NAME_D3D12_OBJECT_WITH_NAME( geometryPso, "%s", "First pass" );

    return S_OK;
}

#ifdef _DEBUG
HRESULT GraphicsCore::InitDebugPSO()
{
    ComPtr<ID3DBlob> vs;
    ComPtr<ID3DBlob> ps;

    D3DReadFileToBlob( L"Assets/Shaders/vs_debug.cso", &vs );
    D3DReadFileToBlob( L"Assets/Shaders/ps_debug.cso", &ps );

    D3D12_INPUT_ELEMENT_DESC inputVertexDesc[ LV_NUM_VS_INPUT_COUNT ];
    ShaderDefinitions::SetGeometryPassInputLayout( inputVertexDesc );

    D3D12_INPUT_LAYOUT_DESC inputLayoutDescription;
    inputLayoutDescription.pInputElementDescs = inputVertexDesc;
    inputLayoutDescription.NumElements = _countof( inputVertexDesc );

    //build out our depth stencil description
    CD3DX12_DEPTH_STENCIL_DESC1 depthStencilDesc( D3D12_DEFAULT );
    depthStencilDesc.DepthEnable = true;
    depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    depthStencilDesc.StencilEnable = FALSE;

    //describe the PSO
    D3D12_GRAPHICS_PIPELINE_STATE_DESC debugPsoDesc{};
    debugPsoDesc.InputLayout = inputLayoutDescription;
    debugPsoDesc.pRootSignature = rootSignature.Get();
    debugPsoDesc.VS = CD3DX12_SHADER_BYTECODE( vs.Get() );
    debugPsoDesc.PS = CD3DX12_SHADER_BYTECODE( ps.Get() );
    debugPsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC( D3D12_DEFAULT );
    debugPsoDesc.BlendState = CD3DX12_BLEND_DESC( D3D12_DEFAULT );
    debugPsoDesc.DepthStencilState = depthStencilDesc;
    debugPsoDesc.SampleMask = UINT_MAX;
    debugPsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
    debugPsoDesc.NumRenderTargets = 1;
    debugPsoDesc.RTVFormats[ 0 ] = DXGI_FORMAT_R8G8B8A8_UNORM;
    debugPsoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    debugPsoDesc.SampleDesc.Count = 1;

    //create our PSO
    ThrowIfFailed( device->CreateGraphicsPipelineState(
        &debugPsoDesc,
        IID_PPV_ARGS( &debugPso )
    ) );
    NAME_D3D12_OBJECT_WITH_NAME( debugPso, "%s", "Debug Wireframes" );
    return S_OK;
}
#endif

HRESULT GraphicsCore::InitLightPassPSO()
{
    ComPtr<ID3DBlob> vs;
    ComPtr<ID3DBlob> ps;

    D3DReadFileToBlob( L"Assets/Shaders/vs_FSQ.cso", &vs );
    D3DReadFileToBlob( L"Assets/Shaders/ps_lighting.cso", &ps );

    D3D12_INPUT_ELEMENT_DESC screenQuadVertexDesc[ LV_NUM_VS_INPUT_COUNT ];
    ShaderDefinitions::SetGeometryPassInputLayout( screenQuadVertexDesc );

    //build out our depth stencil description
    CD3DX12_DEPTH_STENCIL_DESC1 depthStencilDesc( D3D12_DEFAULT );
#ifdef _DEBUG
    depthStencilDesc.DepthEnable = false;
    depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
    depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
#else
    depthStencilDesc.DepthEnable = false;
#endif

    //describe the PSO
    D3D12_GRAPHICS_PIPELINE_STATE_DESC lightPsoDesc{};
    lightPsoDesc.VS = CD3DX12_SHADER_BYTECODE( vs.Get() );
    lightPsoDesc.PS = CD3DX12_SHADER_BYTECODE( ps.Get() );
    lightPsoDesc.InputLayout.pInputElementDescs = screenQuadVertexDesc;
    lightPsoDesc.InputLayout.NumElements = _countof( screenQuadVertexDesc );
    lightPsoDesc.pRootSignature = rootSignature.Get();
    lightPsoDesc.DepthStencilState = depthStencilDesc;
    lightPsoDesc.BlendState = CD3DX12_BLEND_DESC( D3D12_DEFAULT );
    lightPsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC( D3D12_DEFAULT );
    lightPsoDesc.RasterizerState.DepthClipEnable = false;
    lightPsoDesc.SampleMask = UINT_MAX;
    lightPsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    lightPsoDesc.NumRenderTargets = 1;
    lightPsoDesc.RTVFormats[ 0 ] = DXGI_FORMAT_R8G8B8A8_UNORM;
    lightPsoDesc.SampleDesc.Count = 1;

#ifdef _DEBUG
    lightPsoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
#endif

    //create our PSO
    ThrowIfFailed( device->CreateGraphicsPipelineState(
        &lightPsoDesc,
        IID_PPV_ARGS( &lightPso )
    ) );
    NAME_D3D12_OBJECT_WITH_NAME( lightPso, "%s", "Second Pass" );
    return S_OK;
}

HRESULT GraphicsCore::InitSkyboxPSO()
{
	ComPtr<ID3DBlob> vs;
	ComPtr<ID3DBlob> ps;

	D3DReadFileToBlob(L"Assets/Shaders/vs_sky.cso", &vs);
	D3DReadFileToBlob(L"Assets/Shaders/ps_sky.cso", &ps);

	//build the input layout
	D3D12_INPUT_ELEMENT_DESC vertexInputDescription[LV_NUM_VS_INPUT_COUNT];
	ShaderDefinitions::SetGeometryPassInputLayout(vertexInputDescription);

	D3D12_INPUT_LAYOUT_DESC inputLayoutDescription;
	inputLayoutDescription.pInputElementDescs = vertexInputDescription;
	inputLayoutDescription.NumElements = _countof(vertexInputDescription);

	//build out our depth stencil description
    CD3DX12_DEPTH_STENCIL_DESC1 depthStencilDesc(D3D12_DEFAULT);
    depthStencilDesc.DepthEnable = TRUE;
    depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    depthStencilDesc.StencilEnable = TRUE;

	//Rasterizer Description
	D3D12_RASTERIZER_DESC rsDesc = {};
	rsDesc.FillMode = D3D12_FILL_MODE_SOLID;
	rsDesc.CullMode = D3D12_CULL_MODE_FRONT;

	//describe the PSO
	D3D12_GRAPHICS_PIPELINE_STATE_DESC skyboxPsoDesc{};
	skyboxPsoDesc.VS = CD3DX12_SHADER_BYTECODE(vs.Get());
	skyboxPsoDesc.PS = CD3DX12_SHADER_BYTECODE(ps.Get());
    skyboxPsoDesc.InputLayout = inputLayoutDescription;
	skyboxPsoDesc.pRootSignature = rootSignature.Get();
	skyboxPsoDesc.DepthStencilState = depthStencilDesc;
	skyboxPsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	skyboxPsoDesc.RasterizerState = rsDesc;
	skyboxPsoDesc.SampleMask = UINT_MAX;
	skyboxPsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	skyboxPsoDesc.NumRenderTargets = 1;
	skyboxPsoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    skyboxPsoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	skyboxPsoDesc.SampleDesc.Count = 1;

	//create our PSO
	ThrowIfFailed(device->CreateGraphicsPipelineState(
		&skyboxPsoDesc,
		IID_PPV_ARGS(&skyboxPso)
	));
	NAME_D3D12_OBJECT_WITH_NAME(skyboxPso, "%s", "Skybox");

	return S_OK;
}

HRESULT GraphicsCore::InitRtvHeap()
{
    // Create heap of render target descriptors
    // 1 - Albedo
    // 2 - Normal
    // 3 - Position
    // 4 - Back Buffer

    //create actual heap
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = LV_FRAME_COUNT * (LV_NUM_GBUFFER_RTV + 1);
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

    for ( uint32_t i = 0; i < LV_FRAME_COUNT; ++i )
    {
        rtvHandle.Offset( LV_NUM_GBUFFER_RTV, rtvDescriptorSize );

        ThrowIfFailed( swapChain->GetBuffer( i, IID_PPV_ARGS( &renderTargets[ i ] ) ) );
        device->CreateRenderTargetView(
            renderTargets[ i ].Get(),
            &rtvDesc,
            rtvHandle );
        NAME_D3D12_OBJECT_WITH_NAME( renderTargets[ i ], "%s (%d)", "Back Buffer", i );

        rtvHandle.Offset( rtvDescriptorSize );
    }

    return S_OK;
}

HRESULT GraphicsCore::InitDepthStencil()
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

HRESULT GraphicsCore::InitInputShaderResources()
{
    //create command allocator
    ComPtr<ID3D12CommandAllocator> commandAllocator;
    ThrowIfFailed( device->CreateCommandAllocator(
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        IID_PPV_ARGS( &commandAllocator )
    ) );
    NAME_D3D12_OBJECT_WITH_NAME( commandAllocator, "%s", "Shader Resource" );

    //create a command list
    ComPtr<ID3D12GraphicsCommandList> commandList;
    ThrowIfFailed( device->CreateCommandList(
        0,
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        commandAllocator.Get(),
        geometryPso.Get(),
        IID_PPV_ARGS( &commandList )
    ) );
    NAME_D3D12_OBJECT_WITH_NAME( commandList, "%s", "Shader Resource" );

    PIXBeginEvent( commandList.Get(), 0, L"Resetting commandlist" );
    commandList->Close();
    commandAllocator->Reset();
    commandList->Reset( commandAllocator.Get(), geometryPso.Get() );
    PIXEndEvent( commandList.Get() );

    std::vector<Vertex>* vertices = new std::vector<Vertex>();
    std::vector<uint16_t>* indices = new std::vector<uint16_t>();
	std::vector<Vertex>* skyboxVertices = new std::vector<Vertex>();
	std::vector<uint16_t>* skyboxIndices = new std::vector<uint16_t>();
	
    ObjLoader::LoadObj( vertices, indices, "Assets/Models/voxel.obj" );
	ObjLoader::LoadObj(skyboxVertices, skyboxIndices, "Assets/Models/voxel.obj");

    //make vertex buffer for 'n' floats
    uint32_t vertexDataSize = static_cast<uint32_t>( vertices->size() * sizeof( Vertex ) );
    uint32_t vertexDataOffset = 0;
    uint32_t vertexStride = sizeof( Vertex );
    uint32_t indexDataSize = static_cast<uint32_t>( indices->size() * sizeof( uint16_t ) );
    uint32_t indexDataOffset = 0;

    skyboxVerticesCount = static_cast<uint32_t>(skyboxIndices->size());
    verticesCount = static_cast<uint32_t>( indices->size() );

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
        uint32_t fsqVertexDataSize = static_cast<uint32_t>( screenQuad.vertices.size() * sizeof( Vertex ) );
        uint32_t fsqVertexDataOffset = 0;
        uint32_t fsqVertexStride = sizeof( Vertex );

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
        PIXBeginEvent( commandList.Get(), 0, L"Copy screenquad vertex buffer to default resource" );
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
#pragma region SkyboxVertexBufferInit

		//make vertex buffer for 'n' floats
		uint32_t skyboxVertexDataSize = static_cast<uint32_t>(skyboxVertices->size() * sizeof(Vertex));
		uint32_t skyboxVertexDataOffset = 0;
		uint32_t skyboxVertexStride = sizeof(Vertex);

		//create vertex buffer
		ThrowIfFailed(device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(skyboxVertexDataSize),
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&skyboxVertexBuffer)
		));
		NAME_D3D12_OBJECT(skyboxVertexBuffer);

		//create vertex upload buffer
		ThrowIfFailed(device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(skyboxVertexDataSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&skyboxVertexBufferUpload)
		));

		D3D12_SUBRESOURCE_DATA skyboxVertexData = {};
		skyboxVertexData.pData = &((*skyboxVertices)[0]) + skyboxVertexDataOffset;
		skyboxVertexData.RowPitch = skyboxVertexDataSize;
		skyboxVertexData.SlicePitch = skyboxVertexData.RowPitch;


		//this looks like it's pre-emptively loading all the static geometry 
		PIXBeginEvent(commandList.Get(), 0, L"Copy skybox vertex buffer to default resource");
		UpdateSubresources<1>(
			commandList.Get(),
			skyboxVertexBuffer.Get(),
			skyboxVertexBufferUpload.Get(),
			0,
			0,
			1,
			&skyboxVertexData
			);
		commandList->ResourceBarrier(
			1,
			&CD3DX12_RESOURCE_BARRIER::Transition(
				skyboxVertexBuffer.Get(),
				D3D12_RESOURCE_STATE_COPY_DEST,
				D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER
			)
		);
		PIXEndEvent(commandList.Get());


		skyboxVertexBufferView = {};
		skyboxVertexBufferView.BufferLocation = skyboxVertexBuffer->GetGPUVirtualAddress();
		skyboxVertexBufferView.SizeInBytes = skyboxVertexDataSize;
		skyboxVertexBufferView.StrideInBytes = skyboxVertexStride;

#pragma endregion
    }

    //index buffer(s)
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

        //create index upload buffer
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

#pragma region SkyboxIndexBufferInit

		uint32_t skyboxIndexDataSize = static_cast<uint32_t>(skyboxIndices->size() * sizeof(uint16_t));
		uint32_t skyboxIndexDataOffset = 0;

		//create index buffer
		ThrowIfFailed(device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(skyboxIndexDataSize),
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&skyboxIndexBuffer)
		));
		NAME_D3D12_OBJECT(skyboxIndexBuffer);

		//create index upload buffer
		ThrowIfFailed(device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(skyboxIndexDataSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&skyboxIndexBufferUpload)
		));

		D3D12_SUBRESOURCE_DATA skyboxIndexData = {};
		skyboxIndexData.pData = &((*skyboxIndices)[0]) + skyboxIndexDataOffset;
		skyboxIndexData.RowPitch = skyboxIndexDataSize;
		skyboxIndexData.SlicePitch = skyboxIndexData.RowPitch;

		//this looks like it's pre-emptively loading all the static geometry 
		PIXBeginEvent(commandList.Get(), 0, L"Copy skybox index buffer to default resource");
		UpdateSubresources<1>(
			commandList.Get(),
			skyboxIndexBuffer.Get(),
			skyboxIndexBufferUpload.Get(),
			0,
			0,
			1,
			&skyboxIndexData
			);
		commandList->ResourceBarrier(
			1,
			&CD3DX12_RESOURCE_BARRIER::Transition(
				skyboxIndexBuffer.Get(),
				D3D12_RESOURCE_STATE_COPY_DEST,
				D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER
			)
		);
		PIXEndEvent(commandList.Get());

		skyboxIndexBufferView = {};
		skyboxIndexBufferView.BufferLocation = skyboxIndexBuffer->GetGPUVirtualAddress();
		skyboxIndexBufferView.SizeInBytes = skyboxIndexDataSize;
		skyboxIndexBufferView.Format = DXGI_FORMAT_R16_UINT;
#pragma endregion
    }

    delete vertices;
	delete skyboxVertices;
    delete indices;
	delete skyboxIndices;

    // Describe and create a shader resource view (SRV) and constant 
    // buffer view (CBV) descriptor heap.  Heap layout: null views, 
    // frame 1's constant buffer, frame 2's constant buffers, etc...
    const uint32_t nullSrvCount = LV_NUM_GBUFFER_RTV;		// Null descriptors are needed for out of bounds behavior reads.
    const uint32_t cbvCount = LV_FRAME_COUNT * 1; //Frame Count * Number of CBVs
    const uint32_t srvCount = LV_FRAME_COUNT * (LV_NUM_GBUFFER_RTV + LV_NUM_TEXTURES); // _countof(SampleAssets::Textures) + (FrameCount * 1);
    D3D12_DESCRIPTOR_HEAP_DESC cbvSrvHeapDesc = {};
    cbvSrvHeapDesc.NumDescriptors = nullSrvCount + cbvCount + srvCount;
    cbvSrvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    cbvSrvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    ThrowIfFailed( device->CreateDescriptorHeap( &cbvSrvHeapDesc, IID_PPV_ARGS( &cbvSrvHeap ) ) );
    NAME_D3D12_OBJECT( cbvSrvHeap );

    CD3DX12_CPU_DESCRIPTOR_HANDLE cbvSrvHandle( cbvSrvHeap->GetCPUDescriptorHandleForHeapStart() );
    cbvSrvDescriptorSize = device->GetDescriptorHandleIncrementSize( D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV );

    for ( uint32_t i = 0; i < LV_NUM_NULL_SRV; ++i )
    {
        // Describe and create 3 null SRVs. Null descriptors are needed in order 
        // to achieve the effect of an "unbound" resource.
        D3D12_SHADER_RESOURCE_VIEW_DESC nullSrvDesc = {};
        nullSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        nullSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        nullSrvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        nullSrvDesc.Texture2D.MipLevels = 1;
        nullSrvDesc.Texture2D.MostDetailedMip = 0;
        nullSrvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

        device->CreateShaderResourceView( nullptr, &nullSrvDesc, cbvSrvHandle );
        cbvSrvHandle.Offset( cbvSrvDescriptorSize );
    }

	//Skybox SRV
	{

		std::unique_ptr<uint8_t[]> ddsData;
		std::vector<D3D12_SUBRESOURCE_DATA> subresources;
		ThrowIfFailed(
			DirectX::LoadDDSTextureFromFile(device.Get(), L"Assets/Textures/SunnyCubeMap.dds", skyboxTexture.GetAddressOf(),
				ddsData, subresources));

		const UINT64 uploadBufferSize = GetRequiredIntermediateSize(skyboxTexture.Get(), 0,
			static_cast<UINT>(subresources.size()));

		// Create the GPU upload buffer.
		ThrowIfFailed(
			device->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
				D3D12_HEAP_FLAG_NONE,
				&CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(skyboxTextureUpload.GetAddressOf()))
		);

		PIXBeginEvent(commandList.Get(), 0, L"Copy DDS Subtextures");
		UpdateSubresources<1>(
			commandList.Get(), 
            skyboxTexture.Get(),
            skyboxTextureUpload.Get(),
			0, 
			0, 
			1, 
			subresources.data());
		commandList->ResourceBarrier(
			1, 
			&CD3DX12_RESOURCE_BARRIER::Transition(
                skyboxTexture.Get(),
				D3D12_RESOURCE_STATE_COPY_DEST, 
				D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
		);
		PIXEndEvent(commandList.Get());

		D3D12_SHADER_RESOURCE_VIEW_DESC skyboxSrvDesc = {};
		skyboxSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		skyboxSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		skyboxSrvDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		skyboxSrvDesc.Texture2D.MipLevels = 1;
		skyboxSrvDesc.Texture2D.MostDetailedMip = 0;
		skyboxSrvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

        //Offset to after the gbuffer srv's
        cbvSrvHandle.Offset(LV_NUM_GBUFFER_RTV, cbvSrvDescriptorSize);
        for (uint32_t i = 0; i < LV_NUM_TEXTURES * LV_FRAME_COUNT; ++i)
        {
            device->CreateShaderResourceView(skyboxTexture.Get(), &skyboxSrvDesc, cbvSrvHandle);
            cbvSrvHandle.Offset(LV_NUM_CBVSRV_PER_FRAME, cbvSrvDescriptorSize);
        }
        ddsData.release();
	}

    D3D12_DESCRIPTOR_HEAP_DESC samplerHeapDesc = {};
    samplerHeapDesc.NumDescriptors = 1;
    samplerHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
    samplerHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	HRESULT h;
	h = device->CreateDescriptorHeap(&samplerHeapDesc, IID_PPV_ARGS(&samplerHeap));
    ThrowIfFailed( h );
    NAME_D3D12_OBJECT( samplerHeap );

    //Create samplers
    {
        // Get the sampler descriptor size for the current device.
        const uint32_t samplerDescriptorSize = device->GetDescriptorHandleIncrementSize( D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER );

        // Get a handle to the start of the descriptor heap.
        CD3DX12_CPU_DESCRIPTOR_HANDLE samplerHandle( samplerHeap->GetCPUDescriptorHandleForHeapStart() );

        D3D12_SAMPLER_DESC clampSamplerDesc = {};
        clampSamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
        clampSamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        clampSamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        clampSamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        clampSamplerDesc.MinLOD = 0;
        clampSamplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
        clampSamplerDesc.MipLODBias = 0.0f;
        clampSamplerDesc.MaxAnisotropy = 1;
        clampSamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
        device->CreateSampler( &clampSamplerDesc, samplerHandle );
    }

    //close the command list and transfer static data
    ThrowIfFailed( commandList->Close() );

    ID3D12CommandList* ppCommandList[] = { commandList.Get() };
    PIXBeginEvent( commandQueue.Get(), 0, L"Executing commandlist" );
    commandQueue->ExecuteCommandLists( _countof( ppCommandList ), ppCommandList );
    PIXEndEvent( commandQueue.Get() );

    const uint64_t fenceToWaitFor = fenceValue;
    ThrowIfFailed( commandQueue->Signal( fence.Get(), fenceToWaitFor ) );
    ++fenceValue;

    ThrowIfFailed( fence->SetEventOnCompletion( fenceToWaitFor, fenceEvent ) );
    WaitForSingleObject( fenceEvent, INFINITE );

    return S_OK;
}

inline void GraphicsCore::SetGBufferPSO( ID3D12GraphicsCommandList * commandList )
{
    commandList->SetGraphicsRootSignature( rootSignature.Get() );
    commandList->SetPipelineState( geometryPso.Get() );
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
    ID3D12DescriptorHeap* ppHeaps[] = { cbvSrvHeap.Get(), samplerHeap.Get() };
    commandList->SetDescriptorHeaps( _countof( ppHeaps ), ppHeaps );
    commandList->RSSetViewports( 1, &viewport );
    commandList->RSSetScissorRects( 1, &scissorRect );
    commandList->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );
    commandList->IASetVertexBuffers( 0, 1, &fsqVertexBufferView );
    commandList->OMSetStencilRef( 0 );
}

inline void GraphicsCore::SetSkyboxPSO(ID3D12GraphicsCommandList * commandList)
{
	commandList->SetGraphicsRootSignature(rootSignature.Get());
	commandList->SetPipelineState(skyboxPso.Get());
	ID3D12DescriptorHeap* ppHeaps[] = { cbvSrvHeap.Get(), samplerHeap.Get() };
	commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissorRect);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->IASetVertexBuffers(0, 1, &skyboxVertexBufferView);
	commandList->IASetIndexBuffer(&skyboxIndexBufferView);
	commandList->OMSetStencilRef(0);
}

#ifdef _DEBUG
inline void GraphicsCore::SetDebugPSO( ID3D12GraphicsCommandList * commandList )
{
    commandList->SetGraphicsRootSignature( rootSignature.Get() );
    commandList->SetPipelineState( debugPso.Get() );
    ID3D12DescriptorHeap* ppHeaps[] = { cbvSrvHeap.Get() };
    commandList->SetDescriptorHeaps( _countof( ppHeaps ), ppHeaps );
    commandList->RSSetViewports( 1, &viewport );
    commandList->RSSetScissorRects( 1, &scissorRect );
    commandList->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_LINESTRIP );
    commandList->IASetVertexBuffers( 0, 1, &vertexBufferView );
    commandList->IASetIndexBuffer( &indexBufferView );
    commandList->OMSetStencilRef( 0 );
}
#endif
