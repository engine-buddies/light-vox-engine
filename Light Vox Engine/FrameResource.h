#pragma once

#include "stdafx.h"
#include "Camera.h"

struct SceneConstantBuffer
{
	DirectX::XMFLOAT4X4 model;
	DirectX::XMFLOAT4X4 viewProj;
};

struct FrameResource
{
public:
	ID3D12CommandList* batchSubmit[1 + kCOMMAND_LIST_COUNT];

	ComPtr<ID3D12CommandAllocator> commandAllocators[kCOMMAND_LIST_COUNT];
	ComPtr<ID3D12GraphicsCommandList> commandLists[kCOMMAND_LIST_COUNT];

	ComPtr<ID3D12CommandAllocator> sceneCommandAllocators[kNUM_CONTEXTS];
	ComPtr<ID3D12GraphicsCommandList> sceneCommandLists[kNUM_CONTEXTS];

	UINT64 fenceValue;

private:
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pso;
	Microsoft::WRL::ComPtr<ID3D12Resource> sceneConstantBuffer;
	SceneConstantBuffer* sceneConstantBufferWO;
	D3D12_GPU_DESCRIPTOR_HANDLE nullSrvHandle;	// Null SRV for out of bounds behavior.
	D3D12_GPU_DESCRIPTOR_HANDLE sceneCbvHandle;
public:

	FrameResource(
		ID3D12Device* device,
		ID3D12PipelineState* pso,
		ID3D12DescriptorHeap* dsvHeap,
		ID3D12DescriptorHeap* cbvSrvHeap,
		D3D12_VIEWPORT* viewport,
		UINT frameResourceIndex
	);
	~FrameResource();
	
	void Bind(
		ID3D12GraphicsCommandList *commandList,
		BOOL scenePass,
		D3D12_CPU_DESCRIPTOR_HANDLE* rtvHandle,
		D3D12_CPU_DESCRIPTOR_HANDLE* dsvHandle
	);
	void Init();
	void SwapBarriers();
	void Finish();
	void WriteConstantBuffers(D3D12_VIEWPORT* viewport, Camera* camera);
};