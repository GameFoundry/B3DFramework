//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleDistribution.h"
#include "BsScriptNonReflectableWrapper.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleDistribution.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleDistribution.h"
#include "BsScriptNonReflectableWrapper.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleDistribution.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleDistribution.h"
#include "BsScriptNonReflectableWrapper.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleDistribution.h"
#include "../../../Foundation/bsfUtility/Math/BsRandom.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleDistribution.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimationCurve.h"
#include "../../../Foundation/bsfUtility/Math/BsVector3.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimationCurve.h"
#include "../../../Foundation/bsfUtility/Math/BsVector2.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimationCurve.h"

namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptFloatDistribution : public TScriptNonReflectableWrapper<TDistribution<float>, ScriptFloatDistribution>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "FloatDistribution")

		ScriptFloatDistribution(const SPtr<TDistribution<float>>& nativeObject);
		~ScriptFloatDistribution();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalTDistribution(MonoObject* scriptObject);
		static void InternalTDistribution0(MonoObject* scriptObject, float value);
		static void InternalTDistribution1(MonoObject* scriptObject, float minValue, float maxValue);
		static void InternalTDistribution2(MonoObject* scriptObject, MonoObject* curve);
		static void InternalTDistribution3(MonoObject* scriptObject, MonoObject* minCurve, MonoObject* maxCurve);
		static PropertyDistributionType InternalGetType(ScriptFloatDistribution* self);
		static float InternalGetMinConstant(ScriptFloatDistribution* self);
		static float InternalGetMaxConstant(ScriptFloatDistribution* self);
		static MonoObject* InternalGetMinCurve(ScriptFloatDistribution* self);
		static MonoObject* InternalGetMaxCurve(ScriptFloatDistribution* self);
		static float InternalEvaluate(ScriptFloatDistribution* self, float t, float factor);
		static float InternalEvaluate0(ScriptFloatDistribution* self, float t, MonoObject* factor);
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptVector3Distribution : public TScriptNonReflectableWrapper<TDistribution<TVector3<float>>, ScriptVector3Distribution>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "Vector3Distribution")

		ScriptVector3Distribution(const SPtr<TDistribution<TVector3<float>>>& nativeObject);
		~ScriptVector3Distribution();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalTDistribution(MonoObject* scriptObject);
		static void InternalTDistribution0(MonoObject* scriptObject, TVector3<float>* value);
		static void InternalTDistribution1(MonoObject* scriptObject, TVector3<float>* minValue, TVector3<float>* maxValue);
		static void InternalTDistribution2(MonoObject* scriptObject, MonoObject* curve);
		static void InternalTDistribution3(MonoObject* scriptObject, MonoObject* minCurve, MonoObject* maxCurve);
		static PropertyDistributionType InternalGetType(ScriptVector3Distribution* self);
		static void InternalGetMinConstant(ScriptVector3Distribution* self, TVector3<float>* __output);
		static void InternalGetMaxConstant(ScriptVector3Distribution* self, TVector3<float>* __output);
		static MonoObject* InternalGetMinCurve(ScriptVector3Distribution* self);
		static MonoObject* InternalGetMaxCurve(ScriptVector3Distribution* self);
		static void InternalEvaluate(ScriptVector3Distribution* self, float t, float factor, TVector3<float>* __output);
		static void InternalEvaluate0(ScriptVector3Distribution* self, float t, MonoObject* factor, TVector3<float>* __output);
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptVector2Distribution : public TScriptNonReflectableWrapper<TDistribution<TVector2<float>>, ScriptVector2Distribution>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "Vector2Distribution")

		ScriptVector2Distribution(const SPtr<TDistribution<TVector2<float>>>& nativeObject);
		~ScriptVector2Distribution();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalTDistribution(MonoObject* scriptObject);
		static void InternalTDistribution0(MonoObject* scriptObject, TVector2<float>* value);
		static void InternalTDistribution1(MonoObject* scriptObject, TVector2<float>* minValue, TVector2<float>* maxValue);
		static void InternalTDistribution2(MonoObject* scriptObject, MonoObject* curve);
		static void InternalTDistribution3(MonoObject* scriptObject, MonoObject* minCurve, MonoObject* maxCurve);
		static PropertyDistributionType InternalGetType(ScriptVector2Distribution* self);
		static void InternalGetMinConstant(ScriptVector2Distribution* self, TVector2<float>* __output);
		static void InternalGetMaxConstant(ScriptVector2Distribution* self, TVector2<float>* __output);
		static MonoObject* InternalGetMinCurve(ScriptVector2Distribution* self);
		static MonoObject* InternalGetMaxCurve(ScriptVector2Distribution* self);
		static void InternalEvaluate(ScriptVector2Distribution* self, float t, float factor, TVector2<float>* __output);
		static void InternalEvaluate0(ScriptVector2Distribution* self, float t, MonoObject* factor, TVector2<float>* __output);
	};
}
