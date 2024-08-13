//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptResource.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimationClip.h"

namespace bs { class AnimationClip; }
namespace bs { struct __AnimationEventInterop; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptAnimationClip : public TScriptResource<ScriptAnimationClip, AnimationClip>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "AnimationClip")

		ScriptAnimationClip(MonoObject* managedInstance, const TResourceHandle<AnimationClip>& value);

		static MonoObject* CreateInstance();

	private:
		static MonoObject* InternalGetRef(ScriptAnimationClip* self);

		static MonoObject* InternalGetCurves(ScriptAnimationClip* self);
		static void InternalSetCurves(ScriptAnimationClip* self, MonoObject* curves);
		static MonoArray* InternalGetEvents(ScriptAnimationClip* self);
		static void InternalSetEvents(ScriptAnimationClip* self, MonoArray* events);
		static MonoObject* InternalGetRootMotion(ScriptAnimationClip* self);
		static bool InternalHasRootMotion(ScriptAnimationClip* self);
		static bool InternalIsAdditive(ScriptAnimationClip* self);
		static float InternalGetLength(ScriptAnimationClip* self);
		static uint32_t InternalGetSampleRate(ScriptAnimationClip* self);
		static void InternalSetSampleRate(ScriptAnimationClip* self, uint32_t sampleRate);
		static void InternalCreate(MonoObject* managedInstance, bool isAdditive);
		static void InternalCreate0(MonoObject* managedInstance, MonoObject* curves, bool isAdditive, uint32_t sampleRate, MonoObject* rootMotion);
	};
}
