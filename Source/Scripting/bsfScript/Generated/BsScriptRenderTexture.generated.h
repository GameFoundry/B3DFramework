//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "BsScriptRenderTarget.generated.h"
#include "../../../Foundation/bsfCore/RenderAPI/BsRenderTexture.h"
#include "../../../Foundation/bsfCore/Image/BsPixelData.h"

namespace bs
{
	class RenderTexture;
}

namespace bs
{
	class RenderTextureEx;
}

namespace bs
{
	class BS_SCR_BE_EXPORT ScriptRenderTexture : public TScriptReflectable<ScriptRenderTexture, RenderTexture, ScriptRenderTargetBase>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "RenderTexture")

		ScriptRenderTexture(MonoObject* managedInstance, const SPtr<RenderTexture>& value);

		static MonoObject* Create(const SPtr<RenderTexture>& value);

	private:
		static void InternalCreate(MonoObject* managedInstance, PixelFormat format, int32_t width, int32_t height, int32_t numSamples, bool gammaCorrection, bool createDepth, PixelFormat depthStencilFormat);
		static void InternalCreate0(MonoObject* managedInstance, MonoObject* colorSurface);
		static void InternalCreate1(MonoObject* managedInstance, MonoObject* colorSurface, MonoObject* depthStencilSurface);
		static void InternalCreate2(MonoObject* managedInstance, MonoArray* colorSurface);
		static void InternalCreate3(MonoObject* managedInstance, MonoArray* colorSurface, MonoObject* depthStencilSurface);
		static MonoObject* InternalGetColorSurface(ScriptRenderTexture* thisPtr);
		static MonoArray* InternalGetColorSurfaces(ScriptRenderTexture* thisPtr);
		static MonoObject* InternalGetDepthStencilSurface(ScriptRenderTexture* thisPtr);
	};
} // namespace bs
