//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "../../../Foundation/bsfCore/Importer/BsMeshImportOptions.h"

namespace bs
{
	struct AnimationSplitInfo;
}

namespace bs
{
#if !BS_IS_BANSHEE3D
	class BS_SCR_BE_EXPORT ScriptAnimationSplitInfo : public TScriptReflectable<ScriptAnimationSplitInfo, AnimationSplitInfo>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "AnimationSplitInfo")

		ScriptAnimationSplitInfo(MonoObject* managedInstance, const SPtr<AnimationSplitInfo>& value);

		static MonoObject* Create(const SPtr<AnimationSplitInfo>& value);

	private:
		static void InternalAnimationSplitInfo(MonoObject* managedInstance);
		static void InternalAnimationSplitInfo0(MonoObject* managedInstance, MonoString* name, uint32_t startFrame, uint32_t endFrame, bool isAdditive);
		static MonoString* InternalGetName(ScriptAnimationSplitInfo* thisPtr);
		static void InternalSetName(ScriptAnimationSplitInfo* thisPtr, MonoString* value);
		static uint32_t InternalGetStartFrame(ScriptAnimationSplitInfo* thisPtr);
		static void InternalSetStartFrame(ScriptAnimationSplitInfo* thisPtr, uint32_t value);
		static uint32_t InternalGetEndFrame(ScriptAnimationSplitInfo* thisPtr);
		static void InternalSetEndFrame(ScriptAnimationSplitInfo* thisPtr, uint32_t value);
		static bool InternalGetIsAdditive(ScriptAnimationSplitInfo* thisPtr);
		static void InternalSetIsAdditive(ScriptAnimationSplitInfo* thisPtr, bool value);
	};
#endif
} // namespace bs
