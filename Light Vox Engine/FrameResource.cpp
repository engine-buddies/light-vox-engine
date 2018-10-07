#include "FrameResource.h"
#include "Camera.h"

using namespace DirectX;

#define MAX_INSTANCE_COUNT 100;

FrameResource::FrameResource(
    ID3D12Device * device, 
    ID3D12PipelineState * pso, 
    ID3D12DescriptorHeap * dsvHeap, 
    ID3D12DescriptorHeap * cbvSrvHeap, 
    D3D12_VIEWPORT * viewport, 
    UINT frameResourceIndex)
{
    instanceBufferWO = new InstanceBuffer[100];
	fenceValue = 0;
	this->pso = pso;

	//create the main command list
	for (UINT i = 0; i < LV_COMMAND_LIST_COUNT; ++i)
	{
		ThrowIfFailed(device->CreateCommandAllocator(
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            IID_PPV_ARGS(&commandAllocators[i])));
		ThrowIfFailed(device->CreateCommandList(
			0,
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			commandAllocators[i].Get(),
			this->pso.Get(),
			IID_PPV_ARGS(&commandLists[i])
		));

		NAME_D3D12_OBJECT_INDEXED(commandLists, i);
		ThrowIfFailed(commandLists[i]->Close());
	}

	//create the scene rendering command list (split through cores)
	for (UINT i = 0; i < LV_NUM_CONTEXTS; ++i)
	{
		ThrowIfFailed(device->CreateCommandAllocator(
            D3D12_COMMAND_LIST_TYPE_DIRECT, 
            IID_PPV_ARGS(&sceneCommandAllocators[i])));
		ThrowIfFailed(device->CreateCommandList(
			0,
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			sceneCommandAllocators[i].Get(),
			this->pso.Get(),
			IID_PPV_ARGS(&sceneCommandLists[i])
		));

		NAME_D3D12_OBJECT_INDEXED(sceneCommandLists, i);
		ThrowIfFailed(sceneCommandLists[i]->Close());
	}

    CD3DX12_RANGE zeroReadRange(0, 0);

    //create the upload buffer for instance data
    ThrowIfFailed(device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(sizeof(InstanceBuffer) * 100),
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&instanceUploadBuffer)
    ));;

    // map it to a WO struct buffer thing
    ThrowIfFailed(instanceUploadBuffer->Map(0,
        nullptr,
        reinterpret_cast<void**>(&instanceBufferWO)
    ));

    // Null descriptors at the start of the heap (used for shadows)
	const UINT nullSrvCount = 0;			
    const UINT textureCount = 0;
    const UINT cbvCount     = 1;
	const UINT cbvSrvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	//get handle to the cbv heap
	CD3DX12_CPU_DESCRIPTOR_HANDLE cbvSrvCpuHandle(cbvSrvHeap->GetCPUDescriptorHandleForHeapStart());
	CD3DX12_GPU_DESCRIPTOR_HANDLE cbvSrvGpuHandle(cbvSrvHeap->GetGPUDescriptorHandleForHeapStart());
	cbvSrvCpuHandle.Offset(
        (frameResourceIndex * (textureCount)) + nullSrvCount,
        cbvSrvDescriptorSize
    );
	cbvSrvGpuHandle.Offset(
        (frameResourceIndex * (textureCount)) + nullSrvCount,
        cbvSrvDescriptorSize
    );

	// Create the constant buffers.
	const UINT constantBufferSize = (sizeof(SceneConstantBuffer) 
        + (D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1)) 
        & ~(D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1); // must be a multiple 256 bytes
	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(constantBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&sceneConstantBuffer))
    );

	// Map the constant buffers and cache their heap pointers.
	ThrowIfFailed(sceneConstantBuffer->Map(0, 
        &zeroReadRange,
        reinterpret_cast<void**>(&sceneConstantBufferWO)
        )
    );

	// Create the constant buffer view for scene pass
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.SizeInBytes = constantBufferSize;

	// Describe and create the scene constant buffer view (CBV) and 
	// cache the GPU descriptor handle.
	cbvDesc.BufferLocation = sceneConstantBuffer->GetGPUVirtualAddress();
	device->CreateConstantBufferView(&cbvDesc, cbvSrvCpuHandle);
	sceneCbvHandle = cbvSrvGpuHandle;

	//combine all of our command lists
	const UINT batchSize = _countof(sceneCommandLists) + LV_COMMAND_LIST_COUNT; // _countof(m_shadowCommandLists) + 3;
	batchedCommandList[0] = commandLists[LV_COMMAND_LIST_PRE].Get();
	memcpy(batchedCommandList + 1, sceneCommandLists, _countof(sceneCommandLists) * sizeof(ID3D12CommandList*));
	batchedCommandList[_countof(sceneCommandLists) + 1] = commandLists[LV_COMMAND_LIST_MID].Get();
	batchedCommandList[_countof(sceneCommandLists) + 2] = commandLists[LV_COMMAND_LIST_POST].Get();
}

FrameResource::~FrameResource()
{
	for (int i = 0; i < LV_COMMAND_LIST_COUNT; ++i)
	{
		commandAllocators[i] = nullptr;
		commandLists[i] = nullptr;
	}

	sceneConstantBuffer = nullptr;

	for (int i = 0; i < LV_NUM_CONTEXTS; ++i)
	{
		sceneCommandLists[i] = nullptr;
		sceneCommandAllocators[i] = nullptr;
	}
}

void FrameResource::Bind(
    ID3D12GraphicsCommandList * commandList, 
    BOOL scenePass, 
    D3D12_CPU_DESCRIPTOR_HANDLE * rtvHandle, 
    D3D12_CPU_DESCRIPTOR_HANDLE * dsvHandle
)
{
	//set-up before rendering
    commandList->SetGraphicsRootShaderResourceView(0, instanceUploadBuffer->GetGPUVirtualAddress());
    commandList->SetGraphicsRootDescriptorTable(2, sceneCbvHandle);
    //commandList->SetGraphicsRootDescriptorTable(1, sceneCbvHandle);
	commandList->OMSetRenderTargets(1, rtvHandle, FALSE, dsvHandle);
}

void FrameResource::Init()
{
	// Reset the command allocators and lists for the main thread.
	for (int i = 0; i < LV_COMMAND_LIST_COUNT; ++i)
	{
		ThrowIfFailed(commandAllocators[i]->Reset());
		ThrowIfFailed(commandLists[i]->Reset(
            commandAllocators[i].Get(), 
            pso.Get())
        );
	}

	// Reset the worker command allocators and lists.
	for (int i = 0; i < LV_NUM_CONTEXTS; ++i)
	{
		ThrowIfFailed(sceneCommandAllocators[i]->Reset());
		ThrowIfFailed(sceneCommandLists[i]->Reset(
            sceneCommandAllocators[i].Get(),
            pso.Get())
        );
	}
}

void FrameResource::SwapBarriers()
{
	//nothing yet
}

void FrameResource::Finish()
{
	//nothing yet
}

void FrameResource::WriteConstantBuffers(
    DirectX::XMFLOAT4X4 transforms[],
    D3D12_VIEWPORT * viewport, 
    Camera * camera)
{
	SceneConstantBuffer sceneConsts = {};
	camera->GetViewProjMatrix(
        &sceneConsts.view, 
        &sceneConsts.projection, 
        viewport->Width,
        viewport->Height
    );

    for (int i = 0; i < 100; i++)
    {
        instanceBufferWO[i].model = transforms[i];
    }

	//copy over
	memcpy(sceneConstantBufferWO, &sceneConsts, sizeof(SceneConstantBuffer));
}
