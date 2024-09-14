//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptResourceWrapper.h"
#include "BsScriptSpriteImage.generated.h"
#include "Utility/BsUtil.h"
#include "../../../Foundation/bsfCore/Image/BsSpriteVectorPath.h"

namespace bs { class SpriteVectorPath; }
namespace bs { struct __SpriteVectorPathCreateInformationInterop; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptSpriteVectorPath : public TScriptResourceWrapper<SpriteVectorPath, ScriptSpriteVectorPath, ScriptSpriteImageWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "SpriteVectorPath")

		ScriptSpriteVectorPath(const TResourceHandle<SpriteVectorPath>& nativeObject);

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static MonoObject* InternalGetRef(ScriptSpriteVectorPath* self);

		static void InternalSetVectorPath(ScriptSpriteVectorPath* self, MonoObject* vectorPath);
		static void InternalCreate(MonoObject* scriptObject, MonoObject* vectorPath, TSize2<uint32_t>* size);
		static void InternalCreate0(MonoObject* scriptObject, __SpriteVectorPathCreateInformationInterop* createInformation);
	};
}
