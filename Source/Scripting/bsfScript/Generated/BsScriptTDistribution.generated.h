//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleDistribution.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleDistribution.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleDistribution.h"
#include "../../../Foundation/bsfUtility/Math/BsRandom.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleDistribution.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimationCurve.h"
#include "Math/BsVector3.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimationCurve.h"
#include "Math/BsVector2.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimationCurve.h"

namespace bs { template<class T0> struct TDistribution; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptFloatDistribution : public ScriptObject<ScriptFloatDistribution>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "FloatDistribution")

		ScriptFloatDistribution(MonoObject* managedInstance, const SPtr<TDistribution<float>>& value);

		SPtr<TDistribution<float>> GetInternal() const { return mInternal; }
		static MonoObject* Create(const SPtr<TDistribution<float>>& value);

	private:
		SPtr<TDistribution<float>> mInternal;

		static void InternalTDistribution(MonoObject* managedInstance);
		static void InternalTDistribution0(MonoObject* managedInstance, float value);
		static void InternalTDistribution1(MonoObject* managedInstance, float minValue, float maxValue);
		static void InternalTDistribution2(MonoObject* managedInstance, MonoObject* curve);
		static void InternalTDistribution3(MonoObject* managedInstance, MonoObject* minCurve, MonoObject* maxCurve);
		static PropertyDistributionType InternalGetType(ScriptFloatDistribution* thisPtr);
		static float InternalGetMinConstant(ScriptFloatDistribution* thisPtr);
		static float InternalGetMaxConstant(ScriptFloatDistribution* thisPtr);
		static MonoObject* InternalGetMinCurve(ScriptFloatDistribution* thisPtr);
		static MonoObject* InternalGetMaxCurve(ScriptFloatDistribution* thisPtr);
		static float InternalEvaluate(ScriptFloatDistribution* thisPtr, float t, float factor);
		static float InternalEvaluate0(ScriptFloatDistribution* thisPtr, float t, MonoObject* factor);
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptVector3Distribution : public ScriptObject<ScriptVector3Distribution>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "Vector3Distribution")

		ScriptVector3Distribution(MonoObject* managedInstance, const SPtr<TDistribution<TVector3<float>>>& value);

		SPtr<TDistribution<TVector3<float>>> GetInternal() const { return mInternal; }
		static MonoObject* Create(const SPtr<TDistribution<TVector3<float>>>& value);

	private:
		SPtr<TDistribution<TVector3<float>>> mInternal;

		static void InternalTDistribution(MonoObject* managedInstance);
		static void InternalTDistribution0(MonoObject* managedInstance, TVector3<float>* value);
		static void InternalTDistribution1(MonoObject* managedInstance, TVector3<float>* minValue, TVector3<float>* maxValue);
		static void InternalTDistribution2(MonoObject* managedInstance, MonoObject* curve);
		static void InternalTDistribution3(MonoObject* managedInstance, MonoObject* minCurve, MonoObject* maxCurve);
		static PropertyDistributionType InternalGetType(ScriptVector3Distribution* thisPtr);
		static void InternalGetMinConstant(ScriptVector3Distribution* thisPtr, TVector3<float>* __output);
		static void InternalGetMaxConstant(ScriptVector3Distribution* thisPtr, TVector3<float>* __output);
		static MonoObject* InternalGetMinCurve(ScriptVector3Distribution* thisPtr);
		static MonoObject* InternalGetMaxCurve(ScriptVector3Distribution* thisPtr);
		static void InternalEvaluate(ScriptVector3Distribution* thisPtr, float t, float factor, TVector3<float>* __output);
		static void InternalEvaluate0(ScriptVector3Distribution* thisPtr, float t, MonoObject* factor, TVector3<float>* __output);
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptVector2Distribution : public ScriptObject<ScriptVector2Distribution>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "Vector2Distribution")

		ScriptVector2Distribution(MonoObject* managedInstance, const SPtr<TDistribution<TVector2<float>>>& value);

		SPtr<TDistribution<TVector2<float>>> GetInternal() const { return mInternal; }
		static MonoObject* Create(const SPtr<TDistribution<TVector2<float>>>& value);

	private:
		SPtr<TDistribution<TVector2<float>>> mInternal;

		static void InternalTDistribution(MonoObject* managedInstance);
		static void InternalTDistribution0(MonoObject* managedInstance, TVector2<float>* value);
		static void InternalTDistribution1(MonoObject* managedInstance, TVector2<float>* minValue, TVector2<float>* maxValue);
		static void InternalTDistribution2(MonoObject* managedInstance, MonoObject* curve);
		static void InternalTDistribution3(MonoObject* managedInstance, MonoObject* minCurve, MonoObject* maxCurve);
		static PropertyDistributionType InternalGetType(ScriptVector2Distribution* thisPtr);
		static void InternalGetMinConstant(ScriptVector2Distribution* thisPtr, TVector2<float>* __output);
		static void InternalGetMaxConstant(ScriptVector2Distribution* thisPtr, TVector2<float>* __output);
		static MonoObject* InternalGetMinCurve(ScriptVector2Distribution* thisPtr);
		static MonoObject* InternalGetMaxCurve(ScriptVector2Distribution* thisPtr);
		static void InternalEvaluate(ScriptVector2Distribution* thisPtr, float t, float factor, TVector2<float>* __output);
		static void InternalEvaluate0(ScriptVector2Distribution* thisPtr, float t, MonoObject* factor, TVector2<float>* __output);
	};
}
