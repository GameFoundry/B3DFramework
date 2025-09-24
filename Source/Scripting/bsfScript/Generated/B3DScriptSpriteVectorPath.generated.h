//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptResourceWrapper.h"
#include "BsScriptSpriteImage.generated.h"
#include "../../../Foundation/bsfUtility/Math/BsSize2.h"
#include "../../../Foundation/bsfCore/Image/BsSpriteVectorPath.h"

namespace b3d { class SpriteVectorPath; }
namespace b3d { struct __SpriteVectorPathCreateInformationInterop; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptSpriteVectorPath : public TScriptResourceWrapper<SpriteVectorPath, ScriptSpriteVectorPath, ScriptSpriteImageWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "SpriteVectorPath")

		ScriptSpriteVectorPath(const TResourceHandle<SpriteVectorPath>& nativeObject);
		~ScriptSpriteVectorPath();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static MonoObject* InternalGetRef(ScriptSpriteVectorPath* self);

		static void InternalCreate(MonoObject* scriptObject, MonoObject* vectorPath, TSize2<int32_t>* defaultSize);
		static void InternalCreate0(MonoObject* scriptObject, __SpriteVectorPathCreateInformationInterop* createInformation);
	};
}
