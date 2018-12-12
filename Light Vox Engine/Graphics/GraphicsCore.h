#pragma once
#include "../stdafx.h"
#include "DebugRenderer.h"
#include "Camera.h"

namespace Graphics
{
    class FrameResource;

    /// <summary>
    /// This is the core of all our graphics lives
    /// </summary>
    class GraphicsCore
    {
    public:
        /// <summary>
        /// Designated constructor
        /// </summary>
        /// <param name="hWindow">Handle to the window</param>
        /// <param name="windowW">Width of the window</param>
        /// <param name="windowH">Height of the window</param>
        GraphicsCore( HWND hWindow, uint32_t windowW, uint32_t windowH );

        /// <summary>
        /// Destructorino
        /// </summary>
        ~GraphicsCore();

        /// <summary>
        /// Initializes the graphics core for all that rendering
        /// </summary>
        /// <returns>Wether initialization was successful</returns>
        HRESULT Init();

        /// <summary>
        /// Handles our event fences and makes sure we're not writing to a frame
        /// that the GPU is still processing
        /// </summary>
        void Update( glm::mat4x4_packed transforms[], Camera* camera );

        /// <summary>
        /// Where all the render logic lives
        /// </summary>
        void Render();

        /// <summary>
        /// Adjusts assets to the new window size
        /// </summary>
        /// <param name="width">New width of the window</param>
        /// <param name="height">New Height of the window</param>
        void OnResize( uint32_t width, uint32_t height );
    private:

        /*INITIALIZATION HELPERS*/

        /// <summary>
        /// Initializes everything that is required by the IDXGIFactory
        /// (this function should rarely change)
        /// </summary>
        HRESULT InitDeviceCommandQueueSwapChain();

        /// <summary>
        /// Initializes the root signature and the descriptor tables that
        /// goes into its parameters (this changes as we add more functionality)
        /// </summary>
        HRESULT InitRootSignature();

        /// <summary>
        /// Loads in our shaders and builds out a PSO
        /// </summary>
        HRESULT InitGeometryPSO();

        /// <summary>
        /// Loads in our shaders and builds out a PSO for the Second Pass
        /// </summary>
        HRESULT InitLightPassPSO();

		/// <summary>
		/// Loads in our shaders and builds out a PSO for the Second Pass
		/// </summary>
		HRESULT InitSkyboxPSO();

        /// <summary>
        /// Create the Render Target View Heap and initialize our render
        /// targets for the back buffers we'll need
        /// </summary>
        HRESULT InitRtvHeap();

        /// <summary>
        /// Create the Depth Stencil View Heap and initialize our depth stencil 
        /// buffer and how it will clear out our values 
        /// </summary>
        HRESULT InitDepthStencil();

        /// <summary>
        /// Creates the viewport and scissor rectangle
        /// </summary>
        HRESULT InitViewportScissorRectangle();

        /// <summary>
        /// Creates the vertex buffer, the index buffer, shader resource view, and
        /// samplers.
        /// </summary>
        HRESULT InitInputShaderResources();

        /// <summary>
        /// Creates our frame resouces for all the rendering
        /// </summary>
        HRESULT InitFrameResources();

        /// <summary>
        /// Builds the fences we need to synchronize between CPU and GPU
        /// </summary>
        HRESULT InitSynchronizationObjects();

        /*RENDER HELPERS*/

        /// <summary>
        /// Sets the pipeline with common stuff for rendering to the first pass
        /// </summary>
        /// <param name="commandList">The main rendering command list</param>
        inline void SetGBufferPSO( ID3D12GraphicsCommandList* commandList );

        /// <summary>
        /// Sets the pipeline with second pass stuff
        /// </summary>
        /// <param name="commandList">The main rendering command list</param>
        inline void SetLightPassPSO( ID3D12GraphicsCommandList* commandList );

		/// <summary>
		/// Sets the pipeline with second pass stuff
		/// </summary>
		/// <param name="commandList">The main rendering command list</param>
		inline void SetSkyboxPSO(ID3D12GraphicsCommandList* commandList);

        HWND hWindow;		//handle to window
        uint32_t windowWidth;	//width of window
        uint32_t windowHeight;  //height of window

        CD3DX12_VIEWPORT viewport;  //viewport 
        CD3DX12_RECT scissorRect;   //scissor rectangle

        //Main pipeline stuff that shouldn't change up much
        Microsoft::WRL::ComPtr<ID3D12Device> device;
        Microsoft::WRL::ComPtr<IDXGISwapChain3> swapChain;
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue;
        Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilView;
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeap;
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvHeap;

        //Shader program and rendering pipeline specific stuff
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> cbvSrvHeap;
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> samplerHeap;
        Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
        Microsoft::WRL::ComPtr<ID3D12PipelineState> geometryPso;
        Microsoft::WRL::ComPtr<ID3D12PipelineState> lightPso;
		Microsoft::WRL::ComPtr<ID3D12PipelineState> skyboxPso;

        //frame resources
        Microsoft::WRL::ComPtr<ID3D12Resource> renderTargets[ LV_FRAME_COUNT ];
        FrameResource* frameResources[ LV_FRAME_COUNT ];
        FrameResource* currentFrameResource;
        int currentFrameResourceIndex;

        //cached size of rtv descriptor (used to index through the heap)
        uint32_t rtvDescriptorSize;
        uint32_t cbvSrvDescriptorSize;

        //vaiables for rendering one mesh
        D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
        D3D12_INDEX_BUFFER_VIEW indexBufferView;
        Microsoft::WRL::ComPtr<ID3D12Resource> indexBuffer;
        Microsoft::WRL::ComPtr<ID3D12Resource> indexBufferUpload;
        Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer;
        Microsoft::WRL::ComPtr<ID3D12Resource> vertexBufferUpload;

		//Skybox fields
		D3D12_VERTEX_BUFFER_VIEW skyboxVertexBufferView;
		D3D12_INDEX_BUFFER_VIEW skyboxIndexBufferView;
		Microsoft::WRL::ComPtr<ID3D12Resource> skyboxIndexBuffer;
		Microsoft::WRL::ComPtr<ID3D12Resource> skyboxIndexBufferUpload;
		Microsoft::WRL::ComPtr<ID3D12Resource> skyboxVertexBuffer;
		Microsoft::WRL::ComPtr<ID3D12Resource> skyboxVertexBufferUpload;
        Microsoft::WRL::ComPtr<ID3D12Resource> skyboxTexture;
        Microsoft::WRL::ComPtr<ID3D12Resource> skyboxTextureUpload;
        //Microsoft::WRL::ComPtr<ID3D12Resource> textures[1];           
        //Microsoft::WRL::ComPtr<ID3D12Resource> texturesUploads[1];
        uint32_t verticesCount;
        uint32_t skyboxVerticesCount;

        //fence & synch related vars
        uint32_t fenceFrameIndex;
        HANDLE fenceEvent;
        Microsoft::WRL::ComPtr<ID3D12Fence> fence;
        uint64_t fenceValue;

        //Deferred Rendering members
        Microsoft::WRL::ComPtr<ID3D12Resource> fsqVertexBuffer; //For Full Screen Quad
        Microsoft::WRL::ComPtr<ID3D12Resource> fsqVertexBufferUpload;
        D3D12_VERTEX_BUFFER_VIEW fsqVertexBufferView;

#ifdef _DEBUG
        Microsoft::WRL::ComPtr<ID3D12PipelineState> debugPso;

        /// <summary>
    /// Sets the pipeline with second pass stuff
    /// </summary>
    /// <param name="commandList">The main rendering command list</param>
        inline void SetDebugPSO( ID3D12GraphicsCommandList* commandList );

        HRESULT InitDebugPSO();
#endif
        DebugRenderer *debugRenderer;
    };
}
