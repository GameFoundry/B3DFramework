//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "../../../Foundation/bsfCore/Importer/BsMeshImportOptions.h"

namespace bs { struct AnimationSplitInfo; }
namespace bs
{
#if !BS_IS_BANSHEE3D
	class BS_SCR_BE_EXPORT ScriptAnimationSplitInfo : public TScriptReflectable<ScriptAnimationSplitInfo, AnimationSplitInfo>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "AnimationSplitInfo")

		ScriptAnimationSplitInfo(MonoObject* managedInstance, const SPtr<AnimationSplitInfo>& value);

		static MonoObject* create(const SPtr<AnimationSplitInfo>& value);

	private:
		static void Internal_AnimationSplitInfo(MonoObject* managedInstance);
		static void Internal_AnimationSplitInfo0(MonoObject* managedInstance, MonoString* name, uint32_t startFrame, uint32_t endFrame, bool isAdditive);
		static MonoString* Internal_getname(ScriptAnimationSplitInfo* thisPtr);
		static void Internal_setname(ScriptAnimationSplitInfo* thisPtr, MonoString* value);
		static uint32_t Internal_getstartFrame(ScriptAnimationSplitInfo* thisPtr);
		static void Internal_setstartFrame(ScriptAnimationSplitInfo* thisPtr, uint32_t value);
		static uint32_t Internal_getendFrame(ScriptAnimationSplitInfo* thisPtr);
		static void Internal_setendFrame(ScriptAnimationSplitInfo* thisPtr, uint32_t value);
		static bool Internal_getisAdditive(ScriptAnimationSplitInfo* thisPtr);
		static void Internal_setisAdditive(ScriptAnimationSplitInfo* thisPtr, bool value);
	};
#endif
}
