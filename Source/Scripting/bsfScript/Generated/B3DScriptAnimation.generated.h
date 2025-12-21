//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "Wrappers/B3DScriptComponent.h"
#include "../../../Foundation/Utility/Math/B3DAABox.h"
#include "../../../Foundation/Core/Components/B3DAnimation.h"
#include "../../../Foundation/Core/Components/B3DAnimation.h"
#include "../../../Foundation/Utility/Math/B3DVector2.h"
#include "../../../Foundation/Core/Components/B3DAnimation.h"
#include "../../../Foundation/Core/Components/B3DAnimation.h"

namespace b3d { class Animation; }
namespace b3d { struct __Blend1DInfoInterop; }
namespace b3d { struct __Blend2DInfoInterop; }
namespace b3d { struct __TAABox_float_Interop; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptAnimation : public TScriptGameObjectWrapper<Animation, ScriptAnimation>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "Animation")

		ScriptAnimation(const GameObjectHandle<Animation>& nativeObject);
		~ScriptAnimation();

		static void SetupScriptBindings();

		virtual void RegisterEvents();
		virtual void UnregisterEvents();
		static MonoObject* CreateScriptObject(bool construct);

	private:
		void ScriptRebuildFloatPropertiesInternal(const TResourceHandle<AnimationClip>& p0);
		void ScriptUpdateFloatPropertiesInternal();
		void ScriptOnEventTriggeredInternal(const TResourceHandle<AnimationClip>& p0, const String& p1);

		typedef void(B3D_THUNKCALL *ScriptRebuildFloatPropertiesInternalThunkDefinition) (MonoObject*, MonoObject* p0, MonoException**);
		static ScriptRebuildFloatPropertiesInternalThunkDefinition ScriptRebuildFloatPropertiesInternalThunk;
		typedef void(B3D_THUNKCALL *ScriptUpdateFloatPropertiesInternalThunkDefinition) (MonoObject*, MonoException**);
		static ScriptUpdateFloatPropertiesInternalThunkDefinition ScriptUpdateFloatPropertiesInternalThunk;
		typedef void(B3D_THUNKCALL *ScriptOnEventTriggeredInternalThunkDefinition) (MonoObject*, MonoObject* p0, MonoString* p1, MonoException**);
		static ScriptOnEventTriggeredInternalThunkDefinition ScriptOnEventTriggeredInternalThunk;

		static void InternalSetDefaultClip(ScriptAnimation* self, MonoObject* clip);
		static MonoObject* InternalGetDefaultClip(ScriptAnimation* self);
		static void InternalSetWrapMode(ScriptAnimation* self, AnimationWrapMode wrapMode);
		static AnimationWrapMode InternalGetWrapMode(ScriptAnimation* self);
		static void InternalSetSpeed(ScriptAnimation* self, float speed);
		static float InternalGetSpeed(ScriptAnimation* self);
		static void InternalPlay(ScriptAnimation* self, MonoObject* clip);
		static void InternalBlendAdditive(ScriptAnimation* self, MonoObject* clip, float weight, float fadeLength, uint32_t layer);
		static void InternalBlend1D(ScriptAnimation* self, __Blend1DInfoInterop* info, float alpha);
		static void InternalBlend2D(ScriptAnimation* self, __Blend2DInfoInterop* info, TVector2<float>* alpha);
		static void InternalCrossFade(ScriptAnimation* self, MonoObject* clip, float fadeLength);
		static void InternalSample(ScriptAnimation* self, MonoObject* clip, float time);
		static void InternalStop(ScriptAnimation* self, uint32_t layer);
		static void InternalStopAll(ScriptAnimation* self);
		static bool InternalIsPlaying(ScriptAnimation* self);
		static bool InternalGetState(ScriptAnimation* self, MonoObject* clip, AnimationClipState* outState);
		static void InternalSetState(ScriptAnimation* self, MonoObject* clip, AnimationClipState* state);
		static void InternalSetMorphChannelWeight(ScriptAnimation* self, MonoString* name, float weight);
		static void InternalSetCustomBounds(ScriptAnimation* self, __TAABox_float_Interop* bounds);
		static void InternalGetCustomBounds(ScriptAnimation* self, __TAABox_float_Interop* __output);
		static void InternalSetUseCustomBounds(ScriptAnimation* self, bool enable);
		static bool InternalGetUseCustomBounds(ScriptAnimation* self);
		static void InternalSetEnableCull(ScriptAnimation* self, bool enable);
		static bool InternalGetEnableCull(ScriptAnimation* self);
		static uint32_t InternalGetClipCount(ScriptAnimation* self);
		static MonoObject* InternalGetClip(ScriptAnimation* self, uint32_t index);
		static void InternalRefreshClipMappingsInternal(ScriptAnimation* self);
		static bool InternalGetGenericCurveValueInternal(ScriptAnimation* self, uint32_t curveIndex, float* outValue);
		static bool InternalTogglePreviewModeInternal(ScriptAnimation* self, bool enabled);
	};
}
