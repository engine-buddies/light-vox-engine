#pragma once
#include "stdafx.h"		
#include "ShaderDefinitions.h"

class Camera;

/// <summary>
/// Wrapper for data that persists through one scene
/// TODO: proper getters/setters, pack data correctly
/// </summary>
class FrameResource
{
public:

    /// <summary>
    /// Designated constructor
    /// </summary>
    /// <param name="device">Rendering devicec</param>
    /// <param name="pso">Main pipeline state object</param>
    /// <param name="dsvHeap">Depth Stencil View heap</param>
    /// <param name="cbvSrvHeap">CBVSRV heap</param>
    /// <param name="viewport">Viewport</param>
    /// <param name="frameResourceIndex">The index of this frame</param>
    FrameResource(
        ID3D12Device* device,
        ID3D12PipelineState* pso,
        ID3D12DescriptorHeap* dsvHeap,
        ID3D12DescriptorHeap* cbvSrvHeap,
        D3D12_VIEWPORT* viewport,
        UINT frameResourceIndex
    );

    /// <summary>
    /// Destructs the object
    /// </summary>
    ~FrameResource();

    /// <summary>
    /// Binds the resources to prepare for rendering
    /// </summary>
    /// <param name="commandList">Commandlist in which to bind to</param>
    /// <param name="scenePass">Wether this is a scene 
    /// pass or shadow pass (not used right now)</param>
    /// <param name="rtvHandle">Handle to the RTV heap</param>
    /// <param name="dsvHandle">handle to the DSV heap</param>
    void Bind(
        ID3D12GraphicsCommandList *commandList,
        BOOL scenePass,
        D3D12_CPU_DESCRIPTOR_HANDLE* rtvHandle,
        D3D12_CPU_DESCRIPTOR_HANDLE* dsvHandle
    );

    /// <summary>
    /// Refreshes the resource for re-use
    /// </summary>
    void Init();

    /// <summary>
    /// For swapping between render passes (not used right now)
    /// </summary>
    void SwapBarriers();

    /// <summary>
    /// Cleans up the resource states for next use
    /// </summary>
    void Finish();

    /// <summary>
    /// Updates the constant buffer
    /// </summary>
    /// <param name="viewport">The viewport</param>
    /// <param name="camera">The camera of the scene</param>
    void WriteConstantBuffers(
        DirectX::XMFLOAT4X4 transforms[],
        D3D12_VIEWPORT* viewport,
        Camera* camera
    );

    //culmination of all of the command lists
    ID3D12CommandList* batchedCommandList[ 1 + LV_COMMAND_LIST_COUNT ];

    //command list/allocator for the stages (pre, mid, post)
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocators[ LV_COMMAND_LIST_COUNT ];
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandLists[ LV_COMMAND_LIST_COUNT ];

    //command list/allocator for 'per scene' stuff (split between cores)
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> sceneCommandAllocators[ LV_NUM_CONTEXTS ];
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> sceneCommandLists[ LV_NUM_CONTEXTS ];

    //used to synchronize instructions
    UINT64 fenceValue;
private:
    //pointer to things we need to properly render
    Microsoft::WRL::ComPtr<ID3D12PipelineState> pso;

    //the gpu side of the scene constant buffer
    Microsoft::WRL::ComPtr<ID3D12Resource> sceneConstantBuffer;

    //instance upload buffer
    Microsoft::WRL::ComPtr<ID3D12Resource> instanceUploadBuffer;

    //write-only buffer for scene-level cbuffer stuff
    SceneConstantBuffer* sceneConstantBufferWO;
    InstanceBuffer* instanceBufferWO;

    // Null SRV for out of bounds behavior (for shadow?)
    D3D12_GPU_DESCRIPTOR_HANDLE nullSrvHandle;
    D3D12_GPU_DESCRIPTOR_HANDLE sceneCbvHandle;
};
