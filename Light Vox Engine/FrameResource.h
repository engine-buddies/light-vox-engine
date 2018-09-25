#pragma once
#include "stdafx.h"		//this is needed here because our 'configs' are in here

class Camera;

//Whatever matches our constant buffer in the shader
struct SceneConstantBuffer
{
	DirectX::XMFLOAT4X4 model;
	DirectX::XMFLOAT4X4 viewProj;
};

/*
Wraps around data that persists through one scene

TODO: write proper getters/setters, pack data correctly
*/
class FrameResource
{
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

	//binds the frame resource for rendering
	void Bind(
		ID3D12GraphicsCommandList *commandList,
		BOOL scenePass,
		D3D12_CPU_DESCRIPTOR_HANDLE* rtvHandle,
		D3D12_CPU_DESCRIPTOR_HANDLE* dsvHandle
	);

	//refreshes the frame resourcce
	void Init();

	//for swapping shadow texture from write to read (not used rn)
	void SwapBarriers();

	//cleans up any resource state for next use
	void Finish();

	//updates the cbuffer data
	void WriteConstantBuffers(D3D12_VIEWPORT* viewport, Camera* camera);

	//culmination of all of the command lists
	ID3D12CommandList* batchedCommandList[1 + LV_COMMAND_LIST_COUNT];

	//command list/allocator for the stages (pre, mid, post)
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocators[LV_COMMAND_LIST_COUNT];
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandLists[LV_COMMAND_LIST_COUNT];

	//command list/allocator for 'per scene' stuff (split between cores)
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> sceneCommandAllocators[LV_NUM_CONTEXTS];
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> sceneCommandLists[LV_NUM_CONTEXTS];

	//used to synchronize instructions
	UINT64 fenceValue;
private:
	//pointer to things we need to properly render
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pso;
	Microsoft::WRL::ComPtr<ID3D12Resource> sceneConstantBuffer;

	SceneConstantBuffer* sceneConstantBufferWO;		//write-only buffer for scene-level cbuffer stuff
	D3D12_GPU_DESCRIPTOR_HANDLE nullSrvHandle;	// Null SRV for out of bounds behavior (for shadow?)
	D3D12_GPU_DESCRIPTOR_HANDLE sceneCbvHandle;
};