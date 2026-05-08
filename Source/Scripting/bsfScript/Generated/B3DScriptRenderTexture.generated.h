//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptReflectableWrapper.h"
#include "B3DScriptRenderTarget.generated.h"
#include "../../../Engine/Core/GpuBackend/B3DRenderTexture.h"
#include "../../../Engine/Core/Image/B3DPixelData.h"

namespace b3d { class RenderTexture; }
namespace b3d { class RenderTextureEx; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptRenderTexture : public TScriptReflectableWrapper<RenderTexture, ScriptRenderTexture, ScriptRenderTargetWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "RenderTexture")

		ScriptRenderTexture(const TShared<RenderTexture>& nativeObject);
		~ScriptRenderTexture();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalCreate(MonoObject* scriptObject, PixelFormat format, int32_t width, int32_t height, int32_t numSamples, bool gammaCorrection, bool createDepth, PixelFormat depthStencilFormat);
		static void InternalCreate0(MonoObject* scriptObject, MonoObject* colorSurface);
		static void InternalCreate1(MonoObject* scriptObject, MonoObject* colorSurface, MonoObject* depthStencilSurface);
		static void InternalCreate2(MonoObject* scriptObject, MonoArray* colorSurface);
		static void InternalCreate3(MonoObject* scriptObject, MonoArray* colorSurface, MonoObject* depthStencilSurface);
		static MonoObject* InternalGetColorSurface(ScriptRenderTexture* self);
		static MonoArray* InternalGetColorSurfaces(ScriptRenderTexture* self);
		static MonoObject* InternalGetDepthStencilSurface(ScriptRenderTexture* self);
	};
}
