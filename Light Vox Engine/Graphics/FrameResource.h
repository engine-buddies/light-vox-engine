#pragma once
#include "ShaderDefinitions.h"
#include "DebugRenderer.h"

namespace Graphics
{
    class Camera;
    class LightingManager;
    struct LightingSceneConstantBuffer;

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
            uint32_t frameResourceIndex
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

        /// Wrapper for data that persists through one scene
        /// TODO: proper getters/setters, pack data correctly
        /// </summary>
        /// <param name="viewport">The viewport</param>
        /// <param name="camera">The camera of the scene</param>
        void WriteConstantBuffers(
            glm::mat4x4_packed* transforms[],
            D3D12_VIEWPORT* viewport,
            Camera* camera
        );

#ifdef _DEBUG
        void BindDebug( D3D12_CPU_DESCRIPTOR_HANDLE * rtvHandle );

        void WriteDebugInstanceBuffers(
            CubeInstanceData instanceData[],
            size_t count
        );
#endif

        //a batched command list for g-buffer stuff and scene stuff
        ID3D12CommandList* batchedCommandList[ LV_NUM_CONTEXTS * 1 + LV_COMMAND_LIST_COUNT ];
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> geometryCmdLists[ LV_NUM_CONTEXTS ];
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandLists[ LV_COMMAND_LIST_COUNT ];

        //used to synchronize instructions
        uint64_t fenceValue;

    private:
        Microsoft::WRL::ComPtr<ID3D12CommandAllocator> geometryCmdAllocators[ LV_NUM_CONTEXTS ];
        Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocators[ LV_COMMAND_LIST_COUNT ];

        inline void InitCmdAllocatorsAndLists( ID3D12Device * device, uint32_t frameResourceIndex );
        inline void InitDescriptorHandles( ID3D12Device * device, ID3D12DescriptorHeap * dsvHeap,
            ID3D12DescriptorHeap * rtvHeap, ID3D12DescriptorHeap * cbvSrvHeap, uint32_t frameResourceIndex );
        inline void InitGraphicsResources( ID3D12Device * device, ID3D12DescriptorHeap * dsvHeap, ID3D12DescriptorHeap * rtvHeap,
            ID3D12DescriptorHeap * cbvSrvHeap, D3D12_VIEWPORT * viewport, uint32_t frameResourceIndex );
        inline void InitCBV( ID3D12Device * device, ID3D12DescriptorHeap * cbvSrvHeap, uint32_t frameResourceIndex );

        //pointer to things we need to properly render
        Microsoft::WRL::ComPtr<ID3D12PipelineState> geometryBufferPso;
        Microsoft::WRL::ComPtr<ID3D12PipelineState> deferredPso;

        //the gpu side of the scene constant buffer
        Microsoft::WRL::ComPtr<ID3D12Resource> sceneConstantBuffer;
        Microsoft::WRL::ComPtr<ID3D12Resource> instanceUploadBuffer;
        Microsoft::WRL::ComPtr<ID3D12Resource> lightConstantBuffer;

        Microsoft::WRL::ComPtr<ID3D12Resource> rtvTextures[ LV_NUM_GBUFFER_RTV ];

        //write-only buffer for scene-level cbuffer stuff
        SceneConstantBuffer* sceneConstantBufferWO;
        InstanceBuffer* instanceBufferWO;
        LightingSceneConstantBuffer* lightingConstantBufferWO;
        LightingManager* lightingManager = nullptr;

        //handle to cbv required for the scene
        D3D12_GPU_DESCRIPTOR_HANDLE nullHandle;
        D3D12_GPU_DESCRIPTOR_HANDLE sceneCbvHandle;
        D3D12_GPU_DESCRIPTOR_HANDLE lightingCbvHandle;
        D3D12_GPU_DESCRIPTOR_HANDLE gBufferSrvHandle;
        D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle;

        D3D12_CPU_DESCRIPTOR_HANDLE rtvGbufferHandles[ LV_NUM_GBUFFER_RTV ];
        D3D12_CPU_DESCRIPTOR_HANDLE rtvbackBufferHandle;

#ifdef _DEBUG
        Microsoft::WRL::ComPtr<ID3D12Resource> instanceDebugUploadBuffer;
        CubeInstanceData* debugInstanceBufferWO;
#endif
    };
}