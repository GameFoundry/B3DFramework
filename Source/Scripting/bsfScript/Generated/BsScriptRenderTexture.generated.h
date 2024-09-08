//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptReflectableWrapper.h"
#include "BsScriptRenderTarget.generated.h"
#include "../../../Foundation/bsfCore/RenderAPI/BsRenderTexture.h"
#include "../../../Foundation/bsfCore/Image/BsPixelData.h"

namespace bs { class RenderTexture; }
namespace bs { class RenderTextureEx; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptRenderTexture : public TScriptReflectableWrapper<RenderTexture, ScriptRenderTexture, ScriptRenderTargetWrapperBase>
	{
	public:
		B3D_SCRIPT_OBJECT_WRAPPER(kEngineAssembly, kEngineNs, "RenderTexture")

		ScriptRenderTexture(const SPtr<RenderTexture>& nativeObject);

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
