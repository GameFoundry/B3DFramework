//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptResourceWrapper.h"
#include "BsScriptSpriteImage.generated.h"
#include "../../../Foundation/bsfCore/Image/BsSpriteTexture.h"

namespace bs { class SpriteTexture; }
namespace bs { struct __SpriteTextureCreateInformationInterop; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptSpriteTexture : public TScriptResourceWrapper<SpriteTexture, ScriptSpriteTexture, ScriptSpriteImageWrapperBase>
	{
	public:
		B3D_SCRIPT_OBJECT_WRAPPER(kEngineAssembly, kEngineNs, "SpriteTexture")

		ScriptSpriteTexture(const TResourceHandle<SpriteTexture>& nativeObject);

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static MonoObject* InternalGetRef(ScriptSpriteTexture* self);

		static void InternalSetAtlasTexture(ScriptSpriteTexture* self, MonoObject* texture);
		static void InternalCreate(MonoObject* scriptObject, MonoObject* texture);
		static void InternalCreate0(MonoObject* scriptObject, __SpriteTextureCreateInformationInterop* createInformation);
	};
}
