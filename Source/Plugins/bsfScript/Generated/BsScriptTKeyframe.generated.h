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
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "KeyFrameInt")

		static MonoObject* box(const TKeyframe<int32_t>& value);
		static TKeyframe<int32_t> unbox(MonoObject* value);

	private:
		ScriptTKeyframeint32_t(MonoObject* managedInstance);

	};

	class BS_SCR_BE_EXPORT ScriptTKeyframefloat : public ScriptObject<ScriptTKeyframefloat>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "KeyFrame")

		static MonoObject* box(const TKeyframe<float>& value);
		static TKeyframe<float> unbox(MonoObject* value);

	private:
		ScriptTKeyframefloat(MonoObject* managedInstance);

	};

	struct __TKeyframeVector3Interop
	{
		Vector3 value;
		Vector3 inTangent;
		Vector3 outTangent;
		float time;
	};

	class BS_SCR_BE_EXPORT ScriptTKeyframeVector3 : public ScriptObject<ScriptTKeyframeVector3>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "KeyFrameVec3")

		static MonoObject* box(const __TKeyframeVector3Interop& value);
		static __TKeyframeVector3Interop unbox(MonoObject* value);
		static TKeyframe<Vector3> fromInterop(const __TKeyframeVector3Interop& value);
		static __TKeyframeVector3Interop toInterop(const TKeyframe<Vector3>& value);

	private:
		ScriptTKeyframeVector3(MonoObject* managedInstance);

	};

	struct __TKeyframeVector2Interop
	{
		Vector2 value;
		Vector2 inTangent;
		Vector2 outTangent;
		float time;
	};

	class BS_SCR_BE_EXPORT ScriptTKeyframeVector2 : public ScriptObject<ScriptTKeyframeVector2>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "KeyFrameVec2")

		static MonoObject* box(const __TKeyframeVector2Interop& value);
		static __TKeyframeVector2Interop unbox(MonoObject* value);
		static TKeyframe<Vector2> fromInterop(const __TKeyframeVector2Interop& value);
		static __TKeyframeVector2Interop toInterop(const TKeyframe<Vector2>& value);

	private:
		ScriptTKeyframeVector2(MonoObject* managedInstance);

	};

	struct __TKeyframeQuaternionInterop
	{
		Quaternion value;
		Quaternion inTangent;
		Quaternion outTangent;
		float time;
	};

	class BS_SCR_BE_EXPORT ScriptTKeyframeQuaternion : public ScriptObject<ScriptTKeyframeQuaternion>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "KeyFrameQuat")

		static MonoObject* box(const __TKeyframeQuaternionInterop& value);
		static __TKeyframeQuaternionInterop unbox(MonoObject* value);
		static TKeyframe<Quaternion> fromInterop(const __TKeyframeQuaternionInterop& value);
		static __TKeyframeQuaternionInterop toInterop(const TKeyframe<Quaternion>& value);

	private:
		ScriptTKeyframeQuaternion(MonoObject* managedInstance);

	};
}
