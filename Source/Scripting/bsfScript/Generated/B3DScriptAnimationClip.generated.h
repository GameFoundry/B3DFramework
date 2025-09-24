//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptResourceWrapper.h"
#include "../../../Foundation/Core/Animation/B3DAnimationClip.h"

namespace b3d { class AnimationClip; }
namespace b3d { struct __AnimationEventInterop; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptAnimationClip : public TScriptResourceWrapper<AnimationClip, ScriptAnimationClip>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "AnimationClip")

		ScriptAnimationClip(const TResourceHandle<AnimationClip>& nativeObject);
		~ScriptAnimationClip();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

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
		static void InternalCreate(MonoObject* scriptObject, bool isAdditive);
		static void InternalCreate0(MonoObject* scriptObject, MonoObject* curves, bool isAdditive, uint32_t sampleRate, MonoObject* rootMotion);
	};
}
