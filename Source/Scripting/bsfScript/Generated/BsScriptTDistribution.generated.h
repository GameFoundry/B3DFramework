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

namespace bs { template<class T0> struct TDistribution; }
namespace bs
{
	class BS_SCR_BE_EXPORT ScriptTDistributionfloat : public ScriptObject<ScriptTDistributionfloat>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "FloatDistribution")

		ScriptTDistributionfloat(MonoObject* managedInstance, const SPtr<TDistribution<float>>& value);

		SPtr<TDistribution<float>> getInternal() const { return mInternal; }
		static MonoObject* create(const SPtr<TDistribution<float>>& value);

	private:
		SPtr<TDistribution<float>> mInternal;

		static void Internal_TDistribution(MonoObject* managedInstance);
		static void Internal_TDistribution0(MonoObject* managedInstance, float value);
		static void Internal_TDistribution1(MonoObject* managedInstance, float minValue, float maxValue);
		static void Internal_TDistribution2(MonoObject* managedInstance, MonoObject* curve);
		static void Internal_TDistribution3(MonoObject* managedInstance, MonoObject* minCurve, MonoObject* maxCurve);
		static PropertyDistributionType Internal_getType(ScriptTDistributionfloat* thisPtr);
		static float Internal_getMinConstant(ScriptTDistributionfloat* thisPtr);
		static float Internal_getMaxConstant(ScriptTDistributionfloat* thisPtr);
		static MonoObject* Internal_getMinCurve(ScriptTDistributionfloat* thisPtr);
		static MonoObject* Internal_getMaxCurve(ScriptTDistributionfloat* thisPtr);
		static float Internal_evaluate(ScriptTDistributionfloat* thisPtr, float t, float factor);
		static float Internal_evaluate0(ScriptTDistributionfloat* thisPtr, float t, MonoObject* factor);
	};

	class BS_SCR_BE_EXPORT ScriptTDistributionVector3 : public ScriptObject<ScriptTDistributionVector3>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "Vector3Distribution")

		ScriptTDistributionVector3(MonoObject* managedInstance, const SPtr<TDistribution<Vector3>>& value);

		SPtr<TDistribution<Vector3>> getInternal() const { return mInternal; }
		static MonoObject* create(const SPtr<TDistribution<Vector3>>& value);

	private:
		SPtr<TDistribution<Vector3>> mInternal;

		static void Internal_TDistribution(MonoObject* managedInstance);
		static void Internal_TDistribution0(MonoObject* managedInstance, Vector3* value);
		static void Internal_TDistribution1(MonoObject* managedInstance, Vector3* minValue, Vector3* maxValue);
		static void Internal_TDistribution2(MonoObject* managedInstance, MonoObject* curve);
		static void Internal_TDistribution3(MonoObject* managedInstance, MonoObject* minCurve, MonoObject* maxCurve);
		static PropertyDistributionType Internal_getType(ScriptTDistributionVector3* thisPtr);
		static void Internal_getMinConstant(ScriptTDistributionVector3* thisPtr, Vector3* __output);
		static void Internal_getMaxConstant(ScriptTDistributionVector3* thisPtr, Vector3* __output);
		static MonoObject* Internal_getMinCurve(ScriptTDistributionVector3* thisPtr);
		static MonoObject* Internal_getMaxCurve(ScriptTDistributionVector3* thisPtr);
		static void Internal_evaluate(ScriptTDistributionVector3* thisPtr, float t, float factor, Vector3* __output);
		static void Internal_evaluate0(ScriptTDistributionVector3* thisPtr, float t, MonoObject* factor, Vector3* __output);
	};

	class BS_SCR_BE_EXPORT ScriptTDistributionVector2 : public ScriptObject<ScriptTDistributionVector2>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "Vector2Distribution")

		ScriptTDistributionVector2(MonoObject* managedInstance, const SPtr<TDistribution<Vector2>>& value);

		SPtr<TDistribution<Vector2>> getInternal() const { return mInternal; }
		static MonoObject* create(const SPtr<TDistribution<Vector2>>& value);

	private:
		SPtr<TDistribution<Vector2>> mInternal;

		static void Internal_TDistribution(MonoObject* managedInstance);
		static void Internal_TDistribution0(MonoObject* managedInstance, Vector2* value);
		static void Internal_TDistribution1(MonoObject* managedInstance, Vector2* minValue, Vector2* maxValue);
		static void Internal_TDistribution2(MonoObject* managedInstance, MonoObject* curve);
		static void Internal_TDistribution3(MonoObject* managedInstance, MonoObject* minCurve, MonoObject* maxCurve);
		static PropertyDistributionType Internal_getType(ScriptTDistributionVector2* thisPtr);
		static void Internal_getMinConstant(ScriptTDistributionVector2* thisPtr, Vector2* __output);
		static void Internal_getMaxConstant(ScriptTDistributionVector2* thisPtr, Vector2* __output);
		static MonoObject* Internal_getMinCurve(ScriptTDistributionVector2* thisPtr);
		static MonoObject* Internal_getMaxCurve(ScriptTDistributionVector2* thisPtr);
		static void Internal_evaluate(ScriptTDistributionVector2* thisPtr, float t, float factor, Vector2* __output);
		static void Internal_evaluate0(ScriptTDistributionVector2* thisPtr, float t, MonoObject* factor, Vector2* __output);
	};
}
