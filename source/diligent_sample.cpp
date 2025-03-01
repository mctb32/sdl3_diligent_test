#include "diligent_sample.h"

DiligentSample::DiligentSample(RENDER_DEVICE_TYPE dev_type, void *native_wnd)
{
	m_DeviceType = dev_type;

	SwapChainDesc sc_desc;
	sc_desc.IsPrimary = true;
	sc_desc.BufferCount = 2;
	Win32NativeWindow Window{ native_wnd };

	switch (m_DeviceType)
	{

#ifdef D3D12_SUPPORTED
	case RENDER_DEVICE_TYPE_D3D12: {
#if ENGINE_DLL
		auto GetEngineFactoryD3D12 = LoadGraphicsEngineD3D12();
#endif
		auto *pFactoryD3D12 = GetEngineFactoryD3D12();
		EngineD3D12CreateInfo EngineCI;
		pFactoryD3D12->CreateDeviceAndContextsD3D12(EngineCI, &m_pDevice, &m_pImmediateContext);
		pFactoryD3D12->CreateSwapChainD3D12(m_pDevice, m_pImmediateContext, sc_desc, FullScreenModeDesc{}, Window, &m_pSwapChain);
	} break;
#endif

#ifdef D3D11_SUPPORTED
	case RENDER_DEVICE_TYPE_D3D11: {
#if ENGINE_DLL
		auto GetEngineFactoryD3D11 = LoadGraphicsEngineD3D11();
#endif
		auto *pFactoryD3D11 = GetEngineFactoryD3D11();
		EngineD3D11CreateInfo EngineCI;
		pFactoryD3D11->CreateDeviceAndContextsD3D11(EngineCI, &m_pDevice, &m_pImmediateContext);
		pFactoryD3D11->CreateSwapChainD3D11(m_pDevice, m_pImmediateContext, sc_desc, FullScreenModeDesc{}, Window, &m_pSwapChain);
	} break;
#endif

#ifdef GL_SUPPORTED
	case RENDER_DEVICE_TYPE_GL: {
#if ENGINE_DLL
		// Load the dll and import GetEngineFactoryOpenGL() function
		auto GetEngineFactoryOpenGL = LoadGraphicsEngineOpenGL();
#endif
		auto *pFactoryOpenGL = GetEngineFactoryOpenGL();
		EngineGLCreateInfo EngineCI;
		EngineCI.Window.hWnd = native_wnd;
		pFactoryOpenGL->CreateDeviceAndSwapChainGL(EngineCI, &m_pDevice, &m_pImmediateContext, sc_desc, &m_pSwapChain);
	} break;
#endif

#ifdef VULKAN_SUPPORTED
	case RENDER_DEVICE_TYPE_VULKAN: {
#if ENGINE_DLL
		auto GetEngineFactoryVk = LoadGraphicsEngineVk();
#endif
		auto *pFactoryVk = GetEngineFactoryVk();
		EngineVkCreateInfo EngineCI;
		pFactoryVk->CreateDeviceAndContextsVk(EngineCI, &m_pDevice, &m_pImmediateContext);
		pFactoryVk->CreateSwapChainVk(m_pDevice, m_pImmediateContext, sc_desc, Window, &m_pSwapChain);
	} break;
#endif

	default:
		// todo: log error
		break;
	}
}

DiligentSample::~DiligentSample()
{
	if (m_pPSO) m_pPSO->Release();
	if (m_pSwapChain) m_pSwapChain->Release();
	if (m_pImmediateContext) m_pImmediateContext->Release();
	if (m_pDevice) m_pDevice->Release();
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

bool DiligentSample::InitPipeline()
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
	PSOCreateInfo.GraphicsPipeline.RTVFormats[0] = m_pSwapChain->GetDesc().ColorBufferFormat;
	// Use the depth buffer format from the swap chain
	PSOCreateInfo.GraphicsPipeline.DSVFormat = m_pSwapChain->GetDesc().DepthBufferFormat;
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
		m_pDevice->CreateShader(ShaderCI, &pVS);
	}

	// Create a pixel shader
	RefCntAutoPtr<IShader> pPS;
	{
		ShaderCI.Desc.ShaderType = SHADER_TYPE_PIXEL;
		ShaderCI.EntryPoint = "main";
		ShaderCI.Desc.Name = "Triangle pixel shader";
		ShaderCI.Source = PSSource;
		m_pDevice->CreateShader(ShaderCI, &pPS);
	}

	// Finally, create the pipeline state
	PSOCreateInfo.pVS = pVS;
	PSOCreateInfo.pPS = pPS;

	m_pDevice->CreateGraphicsPipelineState(PSOCreateInfo, &m_pPSO);
	return m_pPSO != nullptr;
}

void DiligentSample::Render()
{
	ITextureView *pRTV = m_pSwapChain->GetCurrentBackBufferRTV();
	ITextureView *pDSV = m_pSwapChain->GetDepthBufferDSV();
	m_pImmediateContext->SetRenderTargets(1, &pRTV, pDSV, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

	const float ClearColor[] = { 0.350f,  0.350f,  0.350f, 1.0f };
	m_pImmediateContext->ClearRenderTarget(pRTV, ClearColor, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
	m_pImmediateContext->ClearDepthStencil(pDSV, CLEAR_DEPTH_FLAG, 1.f, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

	m_pImmediateContext->SetPipelineState(m_pPSO);
	// Typically we should now call CommitShaderResources(), however shaders in this example don't
	// use any resources.
	DrawAttribs drawAttrs;
	drawAttrs.NumVertices = 3; // We will render 3 vertices
	m_pImmediateContext->Draw(drawAttrs);

	m_pSwapChain->Present(1);
}
