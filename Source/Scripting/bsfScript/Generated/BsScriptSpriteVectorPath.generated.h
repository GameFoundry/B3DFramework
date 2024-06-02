//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptResource.h"
#include "BsScriptSpriteImage.generated.h"
#include "Utility/BsUtil.h"
#include "../../../Foundation/bsfCore/Image/BsSpriteVectorPath.h"

namespace bs { class SpriteVectorPath; }
namespace bs { struct __SpriteVectorPathCreateInformationInterop; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptSpriteVectorPath : public TScriptResource<ScriptSpriteVectorPath, SpriteVectorPath, ScriptSpriteImageBase>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "SpriteVectorPath")

		ScriptSpriteVectorPath(MonoObject* managedInstance, const TResourceHandle<SpriteVectorPath>& value);

		static MonoObject* CreateInstance();

	private:
		static MonoObject* InternalGetRef(ScriptSpriteVectorPath* thisPtr);

		static void InternalSetVectorPath(ScriptSpriteVectorPath* thisPtr, MonoObject* vectorPath);
		static void InternalCreate(MonoObject* managedInstance, MonoObject* vectorPath, TSize2<uint32_t>* size);
		static void InternalCreate0(MonoObject* managedInstance, __SpriteVectorPathCreateInformationInterop* createInformation);
	};
}
