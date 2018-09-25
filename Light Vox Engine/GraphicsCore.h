#pragma once
#include "stdafx.h"
#include "Camera.h"	//it wouldn't let me forward declare?

class FrameResource;

//this should probably be a namespace instead of a class
class GraphicsCore
{
public:
	GraphicsCore(HWND hWindow, UINT windowW, UINT windowH);
	~GraphicsCore();

	/*Call back for resizing window	*/
	void OnResize(UINT width, UINT height);

	/*initializes core system that's shared between all cores*/
	HRESULT Init();
	void Update();
	void Render();

private:
	static UINT rtvDescriptorSize;		//size of RTV descriptor (used to index through RTV heap)

	
	inline HRESULT InitDeviceCommandQueueSwapChain();	//Initializes everything that requires a IDXGIFactory
	inline HRESULT InitRootSignature();					//Inits the root signature and its descriptor tables
	inline HRESULT InitPSO();							//Loads shaders and builds out the PSO
	inline HRESULT InitRTV();							//Buid RTV heap and the back buffers we need
	inline HRESULT InitDepthStencil();					//Build the DSV heap and how it will clear values
	inline HRESULT InitViewportScissorRectangle();		//Build the viewport and scissor rectangles
	inline HRESULT InitInputShaderRsources();			//Inits vertex buffer, index buffer, srv, samplers
	inline HRESULT InitFrameResources();				//Initializes frame resources (cbv)
	inline HRESULT InitSyncObjects();					//fences?? (tm)


	inline void BeginFrame();
	inline void MidFrame();
	inline void EndFrame();

	inline void SetCommonPipelineState(ID3D12GraphicsCommandList* commandList);

	HWND hWindow;		//handle to window
	UINT windowWidth;	
	UINT windowHeight;

	CD3DX12_VIEWPORT viewport;
	CD3DX12_RECT scissorRect;

	Camera camera;

	//pipeline stuff that shouldn't change much
	Microsoft::WRL::ComPtr<ID3D12Device> device;
	Microsoft::WRL::ComPtr<IDXGISwapChain3> swapChain;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue;
	Microsoft::WRL::ComPtr<ID3D12Resource> renderTargets[LV_FRAME_COUNT];
	Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilView;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvHeap;

	//shader & pipeline specific stuff
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> cbvSrvHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> samplerHeap;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pso;

	//vars for one 'rendering object'
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW indexBufferView;	
	Microsoft::WRL::ComPtr<ID3D12Resource> indexBuffer;
	Microsoft::WRL::ComPtr<ID3D12Resource> indexBufferUpload;
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexBufferUpload;
	//Microsoft::WRL::ComPtr<ID3D12Resource> textures[1];
	//Microsoft::WRL::ComPtr<ID3D12Resource> texturesUploads[1];

	//fence & synch related vars
	UINT frameIndex;
	HANDLE fenceEvent;
	Microsoft::WRL::ComPtr<ID3D12Fence> fence;
	UINT64 fenceValue;

	//frame resources
	FrameResource* frameResources[LV_FRAME_COUNT];
	FrameResource* currentFrameResource;
	int currentFrameResourceIndex;
};

