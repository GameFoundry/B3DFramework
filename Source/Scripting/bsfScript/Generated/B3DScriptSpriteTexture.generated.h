//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptResourceWrapper.h"
#include "BsScriptSpriteImage.generated.h"
#include "../../../Foundation/bsfUtility/Math/BsArea2.h"
#include "../../../Foundation/bsfCore/Image/BsSpriteTexture.h"

namespace b3d { class SpriteTexture; }
namespace b3d { struct __SpriteTextureCreateInformationInterop; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptSpriteTexture : public TScriptResourceWrapper<SpriteTexture, ScriptSpriteTexture, ScriptSpriteImageWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "SpriteTexture")

		ScriptSpriteTexture(const TResourceHandle<SpriteTexture>& nativeObject);
		~ScriptSpriteTexture();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static MonoObject* InternalGetRef(ScriptSpriteTexture* self);

		static MonoObject* InternalGetAtlasTexture(ScriptSpriteTexture* self);
		static void InternalGetUVRange(ScriptSpriteTexture* self, TArea2<float, float>* __output);
		static void InternalCreate(MonoObject* scriptObject, MonoObject* texture);
		static void InternalCreate0(MonoObject* scriptObject, __SpriteTextureCreateInformationInterop* createInformation);
	};
}
