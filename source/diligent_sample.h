#pragma once

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

class DiligentSample
{

public:

	DiligentSample(RENDER_DEVICE_TYPE dev_type, void *native_wnd);
	~DiligentSample();

	bool InitPipeline();
	void Render();

private:

	IRenderDevice *m_pDevice = nullptr;
	IDeviceContext *m_pImmediateContext = nullptr;
	ISwapChain *m_pSwapChain = nullptr;
	RENDER_DEVICE_TYPE m_DeviceType = RENDER_DEVICE_TYPE_UNDEFINED;
	IPipelineState *m_pPSO = nullptr;

};
