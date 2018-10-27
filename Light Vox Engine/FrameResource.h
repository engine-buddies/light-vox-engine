#pragma once
#include "stdafx.h"		

class Camera;

/// <summary>
/// Whatever goes into the constant buffer for our main 'scene' render
/// </summary>
struct SceneConstantBuffer
{
    DirectX::XMFLOAT4X4 model;
    DirectX::XMFLOAT4X4 view;
    DirectX::XMFLOAT4X4 projection;
};

/// <summary>
/// Constant Buffer for info needed for lighting calculations
/// Unused at the moment
/// </summary>
struct LightConstantBuffer
{
    DirectX::XMFLOAT3 pos;
};

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
        ID3D12Device * device,
        ID3D12PipelineState* geometryBufferPso,
        ID3D12PipelineState* scenePso,
        ID3D12DescriptorHeap * dsvHeap,
        ID3D12DescriptorHeap * rtvHeap,
        ID3D12DescriptorHeap * cbvSrvHeap,
        D3D12_VIEWPORT * viewport,
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
    void BindGBuffer();

    void BindDeferred(
        D3D12_CPU_DESCRIPTOR_HANDLE* rtvHandle,
        D3D12_GPU_DESCRIPTOR_HANDLE samplerHandle
    );


    void ResetCommandListsAndAllocators();

    /// <summary>
    /// For swapping between render passes (not used right now)
    /// </summary>
    void SwapBarriers();

    /// <summary>
    /// Cleans up the resource states for next use
    /// </summary>
    void Cleanup();

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

    //a batched command list for g-buffer stuff and scene stuff
    ID3D12CommandList* batchedCommandList[ LV_NUM_CONTEXTS * 1 + LV_COMMAND_LIST_COUNT ];
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> geometryCmdLists[ LV_NUM_CONTEXTS ];
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandLists[ LV_COMMAND_LIST_COUNT ];

    //command list/allocator for 'per scene' stuff (split between cores)
#ifdef _DEBUG
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> debugCmdLists[ LV_NUM_CONTEXTS ];
#endif

    //used to synchronize instructions
    UINT64 fenceValue;
private:
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> geometryCmdAllocators[ LV_NUM_CONTEXTS ];
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocators[ LV_COMMAND_LIST_COUNT ];

#ifdef _DEBUG
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> debugCommandAllocator;
#endif

    inline void InitCmdAllocatorsAndLists( ID3D12Device * device, UINT frameResourceIndex );
    inline void InitDescriptorHandles( ID3D12Device * device, ID3D12DescriptorHeap * dsvHeap,
        ID3D12DescriptorHeap * rtvHeap, ID3D12DescriptorHeap * cbvSrvHeap, UINT frameResourceIndex );
    inline void InitGraphicsResources( ID3D12Device * device, ID3D12DescriptorHeap * dsvHeap, ID3D12DescriptorHeap * rtvHeap,
        ID3D12DescriptorHeap * cbvSrvHeap, D3D12_VIEWPORT * viewport, UINT frameResourceIndex );
    inline void InitCBV( ID3D12Device * device, ID3D12DescriptorHeap * cbvSrvHeap, UINT frameResourceIndex );

    //pointer to things we need to properly render
    Microsoft::WRL::ComPtr<ID3D12PipelineState> geometryBufferPso;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> deferredPso;

    //the gpu side of the scene constant buffer
    Microsoft::WRL::ComPtr<ID3D12Resource> sceneConstantBuffer;
    //Microsoft::WRL::ComPtr<ID3D12Resource> lightConstantBuffer;

    Microsoft::WRL::ComPtr<ID3D12Resource> rtvTextures[ LV_NUM_GBUFFER_RTV ];

    //write-only buffer for scene-level cbuffer stuff
    SceneConstantBuffer* sceneConstantBufferWO;
    //LightConstantBuffer* lightConstantBufferWO;

    //handle to cbv required for the scene
    D3D12_GPU_DESCRIPTOR_HANDLE nullHandle;
    D3D12_GPU_DESCRIPTOR_HANDLE sceneCbvHandle;
    D3D12_GPU_DESCRIPTOR_HANDLE gBufferSrvHandle;
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle;

    D3D12_CPU_DESCRIPTOR_HANDLE rtvGbufferHandles[ LV_NUM_GBUFFER_RTV ];
    D3D12_CPU_DESCRIPTOR_HANDLE rtvbackBufferHandle;

};
