//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptReflectableWrapper.h"
#include "../../../Engine/Core/Importer/B3DMeshImportOptions.h"

namespace b3d { struct AnimationSplitInfo; }
namespace b3d
{
#if !B3D_IS_ENGINE
	class B3D_SCRIPT_INTEROP_EXPORT ScriptAnimationSplitInfo : public TScriptReflectableWrapper<AnimationSplitInfo, ScriptAnimationSplitInfo>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "AnimationSplitInfo")

		ScriptAnimationSplitInfo(const TShared<AnimationSplitInfo>& nativeObject);
		~ScriptAnimationSplitInfo();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalAnimationSplitInfo(MonoObject* scriptObject);
		static void InternalAnimationSplitInfo0(MonoObject* scriptObject, MonoString* name, uint32_t startFrame, uint32_t endFrame, bool isAdditive);
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
