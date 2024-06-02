//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptResource.h"
#include "BsScriptSpriteImage.generated.h"
#include "../../../Foundation/bsfCore/Image/BsSpriteTexture.h"

namespace bs { class SpriteTexture; }
namespace bs { struct __SpriteTextureCreateInformationInterop; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptSpriteTexture : public TScriptResource<ScriptSpriteTexture, SpriteTexture, ScriptSpriteImageBase>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "SpriteTexture")

		ScriptSpriteTexture(MonoObject* managedInstance, const TResourceHandle<SpriteTexture>& value);

		static MonoObject* CreateInstance();

	private:
		static MonoObject* InternalGetRef(ScriptSpriteTexture* thisPtr);

		static void InternalSetAtlasTexture(ScriptSpriteTexture* thisPtr, MonoObject* texture);
		static void InternalCreate(MonoObject* managedInstance, MonoObject* texture);
		static void InternalCreate0(MonoObject* managedInstance, __SpriteTextureCreateInformationInterop* createInformation);
	};
}
