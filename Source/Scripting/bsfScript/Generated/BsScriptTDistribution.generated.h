//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
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
#include "Math/BsVector2.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimationCurve.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimationCurve.h"

namespace bs
{
	template <class T0>
	struct TDistribution;
}

namespace bs
{
	class BS_SCR_BE_EXPORT ScriptTDistributionfloat : public ScriptObject<ScriptTDistributionfloat>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "FloatDistribution")

		ScriptTDistributionfloat(MonoObject* managedInstance, const SPtr<TDistribution<float>>& value);

		SPtr<TDistribution<float>> GetInternal() const { return mInternal; }

		static MonoObject* Create(const SPtr<TDistribution<float>>& value);

	private:
		SPtr<TDistribution<float>> mInternal;

		static void InternalTDistribution(MonoObject* managedInstance);
		static void InternalTDistribution0(MonoObject* managedInstance, float value);
		static void InternalTDistribution1(MonoObject* managedInstance, float minValue, float maxValue);
		static void InternalTDistribution2(MonoObject* managedInstance, MonoObject* curve);
		static void InternalTDistribution3(MonoObject* managedInstance, MonoObject* minCurve, MonoObject* maxCurve);
		static PropertyDistributionType InternalGetType(ScriptTDistributionfloat* thisPtr);
		static float InternalGetMinConstant(ScriptTDistributionfloat* thisPtr);
		static float InternalGetMaxConstant(ScriptTDistributionfloat* thisPtr);
		static MonoObject* InternalGetMinCurve(ScriptTDistributionfloat* thisPtr);
		static MonoObject* InternalGetMaxCurve(ScriptTDistributionfloat* thisPtr);
		static float InternalEvaluate(ScriptTDistributionfloat* thisPtr, float t, float factor);
		static float InternalEvaluate0(ScriptTDistributionfloat* thisPtr, float t, MonoObject* factor);
	};

	class BS_SCR_BE_EXPORT ScriptTDistributionVector3 : public ScriptObject<ScriptTDistributionVector3>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "Vector3Distribution")

		ScriptTDistributionVector3(MonoObject* managedInstance, const SPtr<TDistribution<Vector3>>& value);

		SPtr<TDistribution<Vector3>> GetInternal() const { return mInternal; }

		static MonoObject* Create(const SPtr<TDistribution<Vector3>>& value);

	private:
		SPtr<TDistribution<Vector3>> mInternal;

		static void InternalTDistribution(MonoObject* managedInstance);
		static void InternalTDistribution0(MonoObject* managedInstance, Vector3* value);
		static void InternalTDistribution1(MonoObject* managedInstance, Vector3* minValue, Vector3* maxValue);
		static void InternalTDistribution2(MonoObject* managedInstance, MonoObject* curve);
		static void InternalTDistribution3(MonoObject* managedInstance, MonoObject* minCurve, MonoObject* maxCurve);
		static PropertyDistributionType InternalGetType(ScriptTDistributionVector3* thisPtr);
		static void InternalGetMinConstant(ScriptTDistributionVector3* thisPtr, Vector3* __output);
		static void InternalGetMaxConstant(ScriptTDistributionVector3* thisPtr, Vector3* __output);
		static MonoObject* InternalGetMinCurve(ScriptTDistributionVector3* thisPtr);
		static MonoObject* InternalGetMaxCurve(ScriptTDistributionVector3* thisPtr);
		static void InternalEvaluate(ScriptTDistributionVector3* thisPtr, float t, float factor, Vector3* __output);
		static void InternalEvaluate0(ScriptTDistributionVector3* thisPtr, float t, MonoObject* factor, Vector3* __output);
	};

	class BS_SCR_BE_EXPORT ScriptTDistributionVector2 : public ScriptObject<ScriptTDistributionVector2>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "Vector2Distribution")

		ScriptTDistributionVector2(MonoObject* managedInstance, const SPtr<TDistribution<Vector2>>& value);

		SPtr<TDistribution<Vector2>> GetInternal() const { return mInternal; }

		static MonoObject* Create(const SPtr<TDistribution<Vector2>>& value);

	private:
		SPtr<TDistribution<Vector2>> mInternal;

		static void InternalTDistribution(MonoObject* managedInstance);
		static void InternalTDistribution0(MonoObject* managedInstance, Vector2* value);
		static void InternalTDistribution1(MonoObject* managedInstance, Vector2* minValue, Vector2* maxValue);
		static void InternalTDistribution2(MonoObject* managedInstance, MonoObject* curve);
		static void InternalTDistribution3(MonoObject* managedInstance, MonoObject* minCurve, MonoObject* maxCurve);
		static PropertyDistributionType InternalGetType(ScriptTDistributionVector2* thisPtr);
		static void InternalGetMinConstant(ScriptTDistributionVector2* thisPtr, Vector2* __output);
		static void InternalGetMaxConstant(ScriptTDistributionVector2* thisPtr, Vector2* __output);
		static MonoObject* InternalGetMinCurve(ScriptTDistributionVector2* thisPtr);
		static MonoObject* InternalGetMaxCurve(ScriptTDistributionVector2* thisPtr);
		static void InternalEvaluate(ScriptTDistributionVector2* thisPtr, float t, float factor, Vector2* __output);
		static void InternalEvaluate0(ScriptTDistributionVector2* thisPtr, float t, MonoObject* factor, Vector2* __output);
	};
} // namespace bs
