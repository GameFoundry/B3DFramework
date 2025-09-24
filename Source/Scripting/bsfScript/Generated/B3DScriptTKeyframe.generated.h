//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimationCurve.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimationCurve.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimationCurve.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimationCurve.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimationCurve.h"
#include "../../../Foundation/bsfUtility/Math/BsVector3.h"
#include "../../../Foundation/bsfUtility/Math/BsQuaternion.h"
#include "../../../Foundation/bsfUtility/Math/BsVector2.h"

namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptKeyFrameInt : public TScriptTypeDefinition<ScriptKeyFrameInt>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "KeyFrameInt")

		static MonoObject* Box(const TKeyframe<int32_t>& value);
		static TKeyframe<int32_t> Unbox(MonoObject* value);

	private:
		ScriptKeyFrameInt();

	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptKeyFrame : public TScriptTypeDefinition<ScriptKeyFrame>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "KeyFrame")

		static MonoObject* Box(const TKeyframe<float>& value);
		static TKeyframe<float> Unbox(MonoObject* value);

	private:
		ScriptKeyFrame();

	};

	struct __TKeyframe_TVector3_float__Interop
	{
		TVector3<float> Value;
		TVector3<float> InTangent;
		TVector3<float> OutTangent;
		float Time;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptKeyFrameVec3 : public TScriptTypeDefinition<ScriptKeyFrameVec3>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "KeyFrameVec3")

		static MonoObject* Box(const __TKeyframe_TVector3_float__Interop& value);
		static __TKeyframe_TVector3_float__Interop Unbox(MonoObject* value);
		static TKeyframe<TVector3<float>> FromInterop(const __TKeyframe_TVector3_float__Interop& value);
		static __TKeyframe_TVector3_float__Interop ToInterop(const TKeyframe<TVector3<float>>& value);

	private:
		ScriptKeyFrameVec3();

	};

	struct __TKeyframe_TVector2_float__Interop
	{
		TVector2<float> Value;
		TVector2<float> InTangent;
		TVector2<float> OutTangent;
		float Time;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptKeyFrameVec2 : public TScriptTypeDefinition<ScriptKeyFrameVec2>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "KeyFrameVec2")

		static MonoObject* Box(const __TKeyframe_TVector2_float__Interop& value);
		static __TKeyframe_TVector2_float__Interop Unbox(MonoObject* value);
		static TKeyframe<TVector2<float>> FromInterop(const __TKeyframe_TVector2_float__Interop& value);
		static __TKeyframe_TVector2_float__Interop ToInterop(const TKeyframe<TVector2<float>>& value);

	private:
		ScriptKeyFrameVec2();

	};

	struct __TKeyframe_TQuaternion_float__Interop
	{
		TQuaternion<float> Value;
		TQuaternion<float> InTangent;
		TQuaternion<float> OutTangent;
		float Time;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptKeyFrameQuat : public TScriptTypeDefinition<ScriptKeyFrameQuat>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "KeyFrameQuat")

		static MonoObject* Box(const __TKeyframe_TQuaternion_float__Interop& value);
		static __TKeyframe_TQuaternion_float__Interop Unbox(MonoObject* value);
		static TKeyframe<TQuaternion<float>> FromInterop(const __TKeyframe_TQuaternion_float__Interop& value);
		static __TKeyframe_TQuaternion_float__Interop ToInterop(const TKeyframe<TQuaternion<float>>& value);

	private:
		ScriptKeyFrameQuat();

	};
}
