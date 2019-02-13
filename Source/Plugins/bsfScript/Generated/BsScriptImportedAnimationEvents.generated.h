//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimationClip.h"

namespace bs
{
	struct ImportedAnimationEvents;
	struct __AnimationEventInterop;

#if !BS_IS_BANSHEE3D
	class BS_SCR_BE_EXPORT ScriptImportedAnimationEvents : public ScriptObject<ScriptImportedAnimationEvents>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "ImportedAnimationEvents")

		ScriptImportedAnimationEvents(MonoObject* managedInstance, const SPtr<ImportedAnimationEvents>& value);

		SPtr<ImportedAnimationEvents> getInternal() const { return mInternal; }
		static MonoObject* create(const SPtr<ImportedAnimationEvents>& value);

	private:
		SPtr<ImportedAnimationEvents> mInternal;

		static void Internal_ImportedAnimationEvents(MonoObject* managedInstance);
		static MonoString* Internal_getname(ScriptImportedAnimationEvents* thisPtr);
		static void Internal_setname(ScriptImportedAnimationEvents* thisPtr, MonoString* value);
		static MonoArray* Internal_getevents(ScriptImportedAnimationEvents* thisPtr);
		static void Internal_setevents(ScriptImportedAnimationEvents* thisPtr, MonoArray* value);
	};
#endif
}
