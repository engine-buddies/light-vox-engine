#pragma once
#include "stdafx.h"

class Camera;
class FrameResource;
struct ObjectData;

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
	GraphicsCore(HWND hWindow, UINT windowW, UINT windowH);

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
	void Update(DirectX::XMFLOAT4X4 transforms[], Camera* camera);

    /// <summary>
    /// Where all the render logic lives
    /// </summary>
	void Render();

    /// <summary>
    /// Adjusts assets to the new window size
    /// </summary>
    /// <param name="width">New width of the window</param>
    /// <param name="height">New Height of the window</param>
        void OnResize(UINT width, UINT height);
private:

    /*INITIALIZATION HELPERS*/

    /// <summary>
    /// Initializes everything that is required by the IDXGIFactory
    /// (this function should rarely change)
    /// </summary>
	inline HRESULT InitDeviceCommandQueueSwapChain();

    /// <summary>
    /// Initializes the root signature and the descriptor tables that
    /// goes into its parameters (this changes as we add more functionality)
    /// </summary>
	inline HRESULT InitRootSignature();			

    /// <summary>
    /// Loads in our shaders and builds out a PSO
    /// </summary>
	inline HRESULT InitPSO();	

    /// <summary>
    /// Loads in our shaders and builds out a PSO for the Second Pass
    /// </summary>
    inline HRESULT InitLightPassPSO();

    /// <summary>
    /// Create the Render Target View Heap and initialize our render
    /// targets for the back buffers we'll need
    /// </summary>
	inline HRESULT InitRtvHeap();						

    /// <summary>
    /// Create the Depth Stencil View Heap and initialize our depth stencil 
    /// buffer and how it will clear out our values 
    /// </summary>
	inline HRESULT InitDepthStencil();		

    /// <summary>
    /// Creates the viewport and scissor rectangle
    /// </summary>
	inline HRESULT InitViewportScissorRectangle();	

    /// <summary>
    /// Creates the vertex buffer, the index buffer, shader resource view, and
    /// samplers.
    /// </summary>
    inline HRESULT InitInputShaderResources();

    /// <summary>
    /// Creates our frame resouces for all the rendering
    /// </summary>
	inline HRESULT InitFrameResources();	

    /// <summary>
    /// Builds the fences we need to synchronize between CPU and GPU
    /// </summary>
	inline HRESULT InitSynchronizationObjects();	

    /*RENDER HELPERS*/

    /// <summary>
    /// Clears our depth & color buffer and transitions the back buffer to be
    /// used for rendering
    /// </summary>
	inline void PrepareForGeometryPass();

    /// <summary>
    /// Transitions our resources for our 2nd pass
    /// </summary>
	inline void MidFrame();

    /// <summary>
    /// Transitions our resources for presenting to screen, and cleans up
    /// </summary>
	inline void EndFrame();

    /// <summary>
    /// Sets the pipeline with common stuff for rendering to the first pass
    /// </summary>
    /// <param name="commandList">The main rendering command list</param>
	inline void SetGBufferPSO(ID3D12GraphicsCommandList* commandList);

    /// <summary>
    /// Sets the pipeline with second pass stuff
    /// </summary>
    /// <param name="commandList">The main rendering command list</param>
    inline void SetLightPassPSO(ID3D12GraphicsCommandList* commandList);

	HWND hWindow;		//handle to window
	UINT windowWidth;	//width of window
	UINT windowHeight;  //height of window

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
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pso;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> lightPso;

    //frame resources
    Microsoft::WRL::ComPtr<ID3D12Resource> renderTargets[ LV_FRAME_COUNT ];
    FrameResource* frameResources[LV_FRAME_COUNT];
    FrameResource* currentFrameResource;
    int currentFrameResourceIndex;

    //cached size of rtv descriptor (used to index through the heap)
    UINT rtvDescriptorSize;
    UINT cbvSrvDescriptorSize;

	//vaiables for rendering one mesh
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW indexBufferView;	
	Microsoft::WRL::ComPtr<ID3D12Resource> indexBuffer;
	Microsoft::WRL::ComPtr<ID3D12Resource> indexBufferUpload;
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexBufferUpload;
	//Microsoft::WRL::ComPtr<ID3D12Resource> textures[1];           
	//Microsoft::WRL::ComPtr<ID3D12Resource> texturesUploads[1];
    UINT verticesCount;

	//fence & synch related vars
	UINT frameIndex;
	HANDLE fenceEvent;
	Microsoft::WRL::ComPtr<ID3D12Fence> fence;
	UINT64 fenceValue;

    //Deferred Rendering members
    Microsoft::WRL::ComPtr<ID3D12Resource> fsqVertexBuffer; //For Full Screen Quad
    Microsoft::WRL::ComPtr<ID3D12Resource> fsqVertexBufferUpload;
    D3D12_VERTEX_BUFFER_VIEW fsqVertexBufferView;
};
