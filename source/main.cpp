#include <SDL3/SDL.h>
#include <iostream>

#ifdef D3D11_SUPPORTED
#include "EngineFactoryD3D11.h"
#endif
#ifdef D3D12_SUPPORTED
#include "EngineFactoryD3D12.h"
#endif
#ifdef GL_SUPPORTED
#include "EngineFactoryOpenGL.h"
#endif
#ifdef VULKAN_SUPPORTED
#include "EngineFactoryVk.h"
#endif
#include "RenderDevice.h"
#include "DeviceContext.h"
#include "SwapChain.h"
#include "RefCntAutoPtr.hpp"

using namespace Diligent;

IRenderDevice *pDevice = nullptr;
IDeviceContext *pImmediateContext = nullptr;
ISwapChain *pSwapChain = nullptr; 
RENDER_DEVICE_TYPE device_type = RENDER_DEVICE_TYPE_VULKAN;
//RENDER_DEVICE_TYPE device_type = RENDER_DEVICE_TYPE_D3D12;
RefCntAutoPtr<IPipelineState> m_pPSO;

void DiligentInitDevice()
{
	switch (device_type)
	{

#ifdef D3D12_SUPPORTED
	case RENDER_DEVICE_TYPE_D3D12: {
#if ENGINE_DLL
		auto GetEngineFactoryD3D12 = LoadGraphicsEngineD3D12();
#endif
		auto *pFactoryD3D12 = GetEngineFactoryD3D12();
		EngineD3D12CreateInfo EngineCI;
		pFactoryD3D12->CreateDeviceAndContextsD3D12(EngineCI, &pDevice, &pImmediateContext);
	} break;
#endif

#ifdef D3D11_SUPPORTED
	case RENDER_DEVICE_TYPE_D3D11: {
#if ENGINE_DLL
		auto GetEngineFactoryD3D11 = LoadGraphicsEngineD3D11();
#endif
		auto *pFactoryD3D11 = GetEngineFactoryD3D11();
		EngineD3D11CreateInfo EngineCI;
		pFactoryD3D11->CreateDeviceAndContextsD3D11(EngineCI, &pDevice, &pImmediateContext);
	} break;
#endif

#ifdef GL_SUPPORTED
	case RENDER_DEVICE_TYPE_GL: {
		// device will be init with swapchain
	} break;
#endif

#ifdef VULKAN_SUPPORTED
	case RENDER_DEVICE_TYPE_VULKAN: {
#if ENGINE_DLL
		auto GetEngineFactoryVk = LoadGraphicsEngineVk();
#endif
		auto *pFactoryVk = GetEngineFactoryVk();
		EngineVkCreateInfo EngineCI;
		pFactoryVk->CreateDeviceAndContextsVk(EngineCI, &pDevice, &pImmediateContext);
	} break;
#endif

	default:
		// todo: log error
		break;
	}
}

bool DiligentCreateSwapChain(SDL_Window *wnd, Uint32 buffer_count)
{
	SwapChainDesc sc_desc;
	sc_desc.IsPrimary = true;
	sc_desc.BufferCount = (Uint32)buffer_count;

	Win32NativeWindow Window{ SDL_GetPointerProperty(SDL_GetWindowProperties(wnd), SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL) };

	switch (device_type)
	{

#ifdef D3D12_SUPPORTED
	case RENDER_DEVICE_TYPE_D3D12: {
#if ENGINE_DLL
		static auto GetEngineFactoryD3D12 = LoadGraphicsEngineD3D12();
#endif
		static auto *pFactoryD3D12 = GetEngineFactoryD3D12();
		pFactoryD3D12->CreateSwapChainD3D12(pDevice, pImmediateContext, sc_desc, FullScreenModeDesc{}, Window, &pSwapChain);
	} break;
#endif

#ifdef D3D11_SUPPORTED
	case RENDER_DEVICE_TYPE_D3D11: {
#if ENGINE_DLL
		static auto GetEngineFactoryD3D11 = LoadGraphicsEngineD3D11();
#endif
		static auto *pFactoryD3D11 = GetEngineFactoryD3D11();
		pFactoryD3D11->CreateSwapChainD3D11(pDevice, pImmediateContext, sc_desc, FullScreenModeDesc{}, Window, &pSwapChain);
	} break;
#endif

#ifdef GL_SUPPORTED
	case RENDER_DEVICE_TYPE_GL: {
		// TODO: implement
		return false;
	} break;
#endif

#ifdef VULKAN_SUPPORTED
	case RENDER_DEVICE_TYPE_VULKAN: {
#if ENGINE_DLL
		static auto GetEngineFactoryVk = LoadGraphicsEngineVk();
#endif
		static auto *pFactoryVk = GetEngineFactoryVk();
		pFactoryVk->CreateSwapChainVk(pDevice, pImmediateContext, sc_desc, Window, &pSwapChain);
	} break;
#endif

	default:
		return false;
		break;
	}
	return true;
}

static const char *VSSource = R"(
struct PSInput 
{ 
    float4 Pos   : SV_POSITION; 
    float3 Color : COLOR; 
};

void main(in  uint    VertId : SV_VertexID,
          out PSInput PSIn) 
{
    float4 Pos[3];
    Pos[0] = float4(-0.5, -0.5, 0.0, 1.0);
    Pos[1] = float4( 0.0, +0.5, 0.0, 1.0);
    Pos[2] = float4(+0.5, -0.5, 0.0, 1.0);

    float3 Col[3];
    Col[0] = float3(1.0, 0.0, 0.0); // red
    Col[1] = float3(0.0, 1.0, 0.0); // green
    Col[2] = float3(0.0, 0.0, 1.0); // blue

    PSIn.Pos   = Pos[VertId];
    PSIn.Color = Col[VertId];
}
)";

// Pixel shader simply outputs interpolated vertex color
static const char *PSSource = R"(
struct PSInput 
{ 
    float4 Pos   : SV_POSITION; 
    float3 Color : COLOR; 
};

struct PSOutput
{ 
    float4 Color : SV_TARGET; 
};

void main(in  PSInput  PSIn,
          out PSOutput PSOut)
{
    PSOut.Color = float4(PSIn.Color.rgb, 1.0);
}
)";

IPipelineState *CreatePipeline(Diligent::ISwapChain *sc, Diligent::IRenderDevice *dev)
{
	GraphicsPipelineStateCreateInfo PSOCreateInfo;

	// Pipeline state name is used by the engine to report issues.
	// It is always a good idea to give objects descriptive names.
	PSOCreateInfo.PSODesc.Name = "Simple triangle PSO";

	// This is a graphics pipeline
	PSOCreateInfo.PSODesc.PipelineType = PIPELINE_TYPE_GRAPHICS;

	// clang-format off
	// This tutorial will render to a single render target
	PSOCreateInfo.GraphicsPipeline.NumRenderTargets = 1;
	// Set render target format which is the format of the swap chain's color buffer
	PSOCreateInfo.GraphicsPipeline.RTVFormats[0] = sc->GetDesc().ColorBufferFormat;
	// Use the depth buffer format from the swap chain
	PSOCreateInfo.GraphicsPipeline.DSVFormat = sc->GetDesc().DepthBufferFormat;
	// Primitive topology defines what kind of primitives will be rendered by this pipeline state
	PSOCreateInfo.GraphicsPipeline.PrimitiveTopology = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	// No back face culling for this tutorial
	PSOCreateInfo.GraphicsPipeline.RasterizerDesc.CullMode = CULL_MODE_NONE;
	// Disable depth testing
	PSOCreateInfo.GraphicsPipeline.DepthStencilDesc.DepthEnable = False;
	// clang-format on

	ShaderCreateInfo ShaderCI;
	// Tell the system that the shader source code is in HLSL.
	// For OpenGL, the engine will convert this into GLSL under the hood.
	ShaderCI.SourceLanguage = SHADER_SOURCE_LANGUAGE_HLSL;
	// OpenGL backend requires emulated combined HLSL texture samplers (g_Texture + g_Texture_sampler combination)
	ShaderCI.Desc.UseCombinedTextureSamplers = true;
	// Create a vertex shader
	RefCntAutoPtr<IShader> pVS;
	{
		ShaderCI.Desc.ShaderType = SHADER_TYPE_VERTEX;
		ShaderCI.EntryPoint = "main";
		ShaderCI.Desc.Name = "Triangle vertex shader";
		ShaderCI.Source = VSSource;
		dev->CreateShader(ShaderCI, &pVS);
	}

	// Create a pixel shader
	RefCntAutoPtr<IShader> pPS;
	{
		ShaderCI.Desc.ShaderType = SHADER_TYPE_PIXEL;
		ShaderCI.EntryPoint = "main";
		ShaderCI.Desc.Name = "Triangle pixel shader";
		ShaderCI.Source = PSSource;
		dev->CreateShader(ShaderCI, &pPS);
	}

	// Finally, create the pipeline state
	PSOCreateInfo.pVS = pVS;
	PSOCreateInfo.pPS = pPS;

	Diligent::IPipelineState *pso = nullptr;
	dev->CreateGraphicsPipelineState(PSOCreateInfo, &pso);
	return pso;
}

void DiligentRender()
{
	ITextureView *pRTV = pSwapChain->GetCurrentBackBufferRTV();
	ITextureView *pDSV = pSwapChain->GetDepthBufferDSV();
	pImmediateContext->SetRenderTargets(1, &pRTV, pDSV, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

	const float ClearColor[] = { 0.350f,  0.350f,  0.350f, 1.0f };
	pImmediateContext->ClearRenderTarget(pRTV, ClearColor, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
	pImmediateContext->ClearDepthStencil(pDSV, CLEAR_DEPTH_FLAG, 1.f, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

	pImmediateContext->SetPipelineState(m_pPSO);
	// Typically we should now call CommitShaderResources(), however shaders in this example don't
	// use any resources.
	DrawAttribs drawAttrs;
	drawAttrs.NumVertices = 3; // We will render 3 vertices
	pImmediateContext->Draw(drawAttrs);

	pSwapChain->Present(1);
}

int main(int argc, char *argv[]) 
{
	// Initialize the SDL video subsystem.
	if (!SDL_Init(SDL_INIT_VIDEO)) 
	{
		std::cerr << "SDL_Init Error: " << SDL_GetError() << "\n";
		return 1;
	}

	// Initialize the Diligent engine.
	DiligentInitDevice();

	// Create the window
	SDL_Window *window = SDL_CreateWindow("SdlSandbox", 800, 600, device_type == RENDER_DEVICE_TYPE_VULKAN ? SDL_WINDOW_VULKAN : 0);
	if (!window) 
	{
		std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << "\n";
		SDL_Quit();
		return 1;
	}
	
	// Create the swap chain
	DiligentCreateSwapChain(window, 2);

	// Create the pipeline state object
	m_pPSO = CreatePipeline(pSwapChain, pDevice);

	// Main loop
	bool running = true;
	SDL_Event event;
	while (running) 
	{
		while (SDL_PollEvent(&event)) 
		{
			if (event.type == SDL_EVENT_QUIT) 
				running = false;
			else if (event.type == SDL_EVENT_KEY_DOWN) 
			{
				if (event.key.key == SDLK_ESCAPE) 
					running = false;
			}
			else if (event.type == SDL_EVENT_WINDOW_RESIZED)
			{
				if (pSwapChain)
					pSwapChain->Resize(event.window.data1, event.window.data2);
			}
		}

		DiligentRender();
		SDL_Delay(10);
	}

	// Clean up resources.
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}