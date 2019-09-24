//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "../../../Foundation/bsfCore/Importer/BsMeshImportOptions.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimationClip.h"

namespace bs { struct ImportedAnimationEvents; }
namespace bs { struct __AnimationEventInterop; }
namespace bs
{
#if !BS_IS_BANSHEE3D
	class BS_SCR_BE_EXPORT ScriptImportedAnimationEvents : public TScriptReflectable<ScriptImportedAnimationEvents, ImportedAnimationEvents>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "ImportedAnimationEvents")

		ScriptImportedAnimationEvents(MonoObject* managedInstance, const SPtr<ImportedAnimationEvents>& value);

		static MonoObject* create(const SPtr<ImportedAnimationEvents>& value);

	private:
		static void Internal_ImportedAnimationEvents(MonoObject* managedInstance);
		static MonoString* Internal_getname(ScriptImportedAnimationEvents* thisPtr);
		static void Internal_setname(ScriptImportedAnimationEvents* thisPtr, MonoString* value);
		static MonoArray* Internal_getevents(ScriptImportedAnimationEvents* thisPtr);
		static void Internal_setevents(ScriptImportedAnimationEvents* thisPtr, MonoArray* value);
	};
#endif
}
