//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"

#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }

#if BS_DEBUG_MODE
#	define D3D_DEBUG_INFO
#endif

#define WIN32_LEAN_AND_MEAN
#if !defined(NOMINMAX) && defined(_MSC_VER)
#	define NOMINMAX // Required to stop windows.h messing up std::min
#endif

#include <d3d11.h>
#include <d3d11shader.h>
#include <D3Dcompiler.h>

/** @addtogroup Plugins
 *  @{
 */

/** @defgroup D3D11 BansheeD3D11RenderAPI
 *	Wrapper around the DirectX 11 render API.
 */

/** @} */

namespace bs { namespace ct
{
	class D3D11RenderAPI;
	class D3D11Driver;
	class D3D11DriverList;
	class D3D11VideoMode;
	class D3D11VideoModeList;
	class D3D11GpuProgramManager;
	class D3D11HLSLProgramFactory;
	class D3D11Device;
	class D3D11HardwareBuffer;
	class D3D11DepthStencilState;
	class D3D11InputLayoutManager;
	class D3D11GpuBuffer;
	class D3D11RenderUtility;
	class D3D11GpuProgram;
	class D3D11TextureView;
	class D3D11RenderWindow;
	class GpuBufferView;

	/**	DirectX 11 specific types to track resource statistics for. */
	enum D3D11RenderStatResourceType
	{
		RenderStatObject_DepthStencilState = 100,
		RenderStatObject_RasterizerState,
		RenderStatObject_BlendState,
		RenderStatObject_SamplerState,
		RenderStatObject_InputLayout,
		RenderStatObject_ResourceView,
		RenderStatObject_SwapChain
	};

	typedef Vector<char*> HLSLMicroCode;
}}