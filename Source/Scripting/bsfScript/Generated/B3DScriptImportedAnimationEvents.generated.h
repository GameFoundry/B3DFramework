//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptReflectableWrapper.h"
#include "../../../Foundation/Core/Importer/B3DMeshImportOptions.h"
#include "../../../Foundation/Core/Animation/B3DAnimationClip.h"

namespace b3d { struct __AnimationEventInterop; }
namespace b3d { struct ImportedAnimationEvents; }
namespace b3d
{
#if !B3D_IS_ENGINE
	class B3D_SCRIPT_INTEROP_EXPORT ScriptImportedAnimationEvents : public TScriptReflectableWrapper<ImportedAnimationEvents, ScriptImportedAnimationEvents>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ImportedAnimationEvents")

		ScriptImportedAnimationEvents(const SPtr<ImportedAnimationEvents>& nativeObject);
		~ScriptImportedAnimationEvents();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalImportedAnimationEvents(MonoObject* scriptObject);
		static MonoString* InternalGetName(ScriptImportedAnimationEvents* self);
		static void InternalSetName(ScriptImportedAnimationEvents* self, MonoString* value);
		static MonoArray* InternalGetEvents(ScriptImportedAnimationEvents* self);
		static void InternalSetEvents(ScriptImportedAnimationEvents* self, MonoArray* value);
	};
#endif
}
