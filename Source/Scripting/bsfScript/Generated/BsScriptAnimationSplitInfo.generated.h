//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "../../../Foundation/bsfCore/Importer/BsMeshImportOptions.h"

namespace bs { struct AnimationSplitInfo; }
namespace bs
{
#if !B3D_IS_ENGINE
	class B3D_SCRIPT_INTEROP_EXPORT ScriptAnimationSplitInfo : public TScriptReflectable<ScriptAnimationSplitInfo, AnimationSplitInfo>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "AnimationSplitInfo")

		ScriptAnimationSplitInfo(MonoObject* managedInstance, const SPtr<AnimationSplitInfo>& value);

		static MonoObject* Create(const SPtr<AnimationSplitInfo>& value);

	private:
		static void InternalAnimationSplitInfo(MonoObject* managedInstance);
		static void InternalAnimationSplitInfo0(MonoObject* managedInstance, MonoString* name, uint32_t startFrame, uint32_t endFrame, bool isAdditive);
		static MonoString* InternalGetName(ScriptAnimationSplitInfo* self);
		static void InternalSetName(ScriptAnimationSplitInfo* self, MonoString* value);
		static uint32_t InternalGetStartFrame(ScriptAnimationSplitInfo* self);
		static void InternalSetStartFrame(ScriptAnimationSplitInfo* self, uint32_t value);
		static uint32_t InternalGetEndFrame(ScriptAnimationSplitInfo* self);
		static void InternalSetEndFrame(ScriptAnimationSplitInfo* self, uint32_t value);
		static bool InternalGetIsAdditive(ScriptAnimationSplitInfo* self);
		static void InternalSetIsAdditive(ScriptAnimationSplitInfo* self, bool value);
	};
#endif
}
