//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimationCurve.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimationCurve.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimationCurve.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimationCurve.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimationCurve.h"
#include "Math/BsVector3.h"
#include "Math/BsVector2.h"
#include "Math/BsQuaternion.h"

namespace bs
{
	class BS_SCR_BE_EXPORT ScriptTKeyframeint32_t : public ScriptObject<ScriptTKeyframeint32_t>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "KeyFrameInt")

		static MonoObject* Box(const TKeyframe<int32_t>& value);
		static TKeyframe<int32_t> Unbox(MonoObject* value);

	private:
		ScriptTKeyframeint32_t(MonoObject* managedInstance);
	};

	class BS_SCR_BE_EXPORT ScriptTKeyframefloat : public ScriptObject<ScriptTKeyframefloat>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "KeyFrame")

		static MonoObject* Box(const TKeyframe<float>& value);
		static TKeyframe<float> Unbox(MonoObject* value);

	private:
		ScriptTKeyframefloat(MonoObject* managedInstance);
	};

	struct __TKeyframeVector3Interop
	{
		Vector3 Value;
		Vector3 InTangent;
		Vector3 OutTangent;
		float Time;
	};

	class BS_SCR_BE_EXPORT ScriptTKeyframeVector3 : public ScriptObject<ScriptTKeyframeVector3>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "KeyFrameVec3")

		static MonoObject* Box(const __TKeyframeVector3Interop& value);
		static __TKeyframeVector3Interop Unbox(MonoObject* value);
		static TKeyframe<Vector3> FromInterop(const __TKeyframeVector3Interop& value);
		static __TKeyframeVector3Interop ToInterop(const TKeyframe<Vector3>& value);

	private:
		ScriptTKeyframeVector3(MonoObject* managedInstance);
	};

	struct __TKeyframeVector2Interop
	{
		Vector2 Value;
		Vector2 InTangent;
		Vector2 OutTangent;
		float Time;
	};

	class BS_SCR_BE_EXPORT ScriptTKeyframeVector2 : public ScriptObject<ScriptTKeyframeVector2>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "KeyFrameVec2")

		static MonoObject* Box(const __TKeyframeVector2Interop& value);
		static __TKeyframeVector2Interop Unbox(MonoObject* value);
		static TKeyframe<Vector2> FromInterop(const __TKeyframeVector2Interop& value);
		static __TKeyframeVector2Interop ToInterop(const TKeyframe<Vector2>& value);

	private:
		ScriptTKeyframeVector2(MonoObject* managedInstance);
	};

	struct __TKeyframeQuaternionInterop
	{
		Quaternion Value;
		Quaternion InTangent;
		Quaternion OutTangent;
		float Time;
	};

	class BS_SCR_BE_EXPORT ScriptTKeyframeQuaternion : public ScriptObject<ScriptTKeyframeQuaternion>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "KeyFrameQuat")

		static MonoObject* Box(const __TKeyframeQuaternionInterop& value);
		static __TKeyframeQuaternionInterop Unbox(MonoObject* value);
		static TKeyframe<Quaternion> FromInterop(const __TKeyframeQuaternionInterop& value);
		static __TKeyframeQuaternionInterop ToInterop(const TKeyframe<Quaternion>& value);

	private:
		ScriptTKeyframeQuaternion(MonoObject* managedInstance);
	};
} // namespace bs
