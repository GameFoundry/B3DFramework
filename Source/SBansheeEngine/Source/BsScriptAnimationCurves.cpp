//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptAnimationCurves.h"
#include "BsScriptAnimationCurve.h"
#include "BsMonoUtil.h"
#include "BsMonoClass.h"
#include "BsMonoField.h"
#include "BsAnimationCurve.h"
#include "BsAnimationClip.h"
#include "BsAnimationUtility.h"
#include "BsMath.h"
#include "BsVector3.h"

using namespace std::placeholders;

namespace BansheeEngine
{
	MonoField* ScriptAnimationCurves::sPositionCurvesField = nullptr;
	MonoField* ScriptAnimationCurves::sRotationCurvesField = nullptr;
	MonoField* ScriptAnimationCurves::sScaleCurvesField = nullptr;
	MonoField* ScriptAnimationCurves::sFloatCurvesField = nullptr;

	ScriptAnimationCurves::ScriptAnimationCurves(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptAnimationCurves::initRuntimeData()
	{
		sPositionCurvesField = metaData.scriptClass->getField("PositionCurves");
		sRotationCurvesField = metaData.scriptClass->getField("RotationCurves");
		sScaleCurvesField = metaData.scriptClass->getField("ScaleCurves");
		sFloatCurvesField = metaData.scriptClass->getField("FloatCurves");
	}

	SPtr<AnimationCurves> ScriptAnimationCurves::toNative(MonoObject* instance)
	{
		SPtr<AnimationCurves> output = bs_shared_ptr_new<AnimationCurves>();

		MonoArray* monoPosCurves;
		sPositionCurvesField->getValue(instance, &monoPosCurves);

		if (monoPosCurves != nullptr)
		{
			ScriptArray scriptCurves(monoPosCurves);
			for(UINT32 i = 0; i < scriptCurves.size(); i++)
			{
				MonoObject* monoCurve = scriptCurves.get<MonoObject*>(i);
				output->position.push_back(ScriptNamedVector3Curve::toNative(monoCurve));
			}
		}

		MonoArray* monoRotCurves;
		sRotationCurvesField->getValue(instance, &monoRotCurves);

		if (monoRotCurves != nullptr)
		{
			ScriptArray scriptCurves(monoRotCurves);
			for (UINT32 i = 0; i < scriptCurves.size(); i++)
			{
				MonoObject* monoCurve = scriptCurves.get<MonoObject*>(i);
				TNamedAnimationCurve<Vector3> eulerRotation = ScriptNamedVector3Curve::toNative(monoCurve);

				TNamedAnimationCurve<Quaternion> quatRotation;
				quatRotation.name = eulerRotation.name;
				quatRotation.curve = AnimationUtility::eulerToQuaternionCurve(eulerRotation.curve);

				// DEBUG ONLY
				TAnimationCurve<Vector3> eulerRotation2 = AnimationUtility::quaternionToEulerCurve(quatRotation.curve);

				output->rotation.push_back(quatRotation);
			}
		}

		MonoArray* monoScaleCurves;
		sScaleCurvesField->getValue(instance, &monoScaleCurves);

		if (monoScaleCurves != nullptr)
		{
			ScriptArray scriptCurves(monoScaleCurves);
			for (UINT32 i = 0; i < scriptCurves.size(); i++)
			{
				MonoObject* monoCurve = scriptCurves.get<MonoObject*>(i);
				output->scale.push_back(ScriptNamedVector3Curve::toNative(monoCurve));
			}
		}

		MonoArray* monoFloatCurves;
		sFloatCurvesField->getValue(instance, &monoFloatCurves);

		if (monoFloatCurves != nullptr)
		{
			ScriptArray scriptCurves(monoFloatCurves);
			for (UINT32 i = 0; i < scriptCurves.size(); i++)
			{
				MonoObject* monoCurve = scriptCurves.get<MonoObject*>(i);
				output->generic.push_back(ScriptNamedFloatCurve::toNative(monoCurve));
			}
		}

		return output;
	}

	MonoObject* ScriptAnimationCurves::toManaged(const SPtr<AnimationCurves>& curves)
	{
		UINT32 numPosCurves = (UINT32)curves->position.size();
		ScriptArray scriptPositionCurves = ScriptArray::create<ScriptNamedVector3Curve>(numPosCurves);

		for(UINT32 i = 0; i < numPosCurves; i++)
		{
			MonoObject* monoCurve = ScriptNamedVector3Curve::toManaged(curves->position[i]);
			scriptPositionCurves.set(i, monoCurve);
		}

		UINT32 numRotCurves = (UINT32)curves->rotation.size();
		ScriptArray scriptRotationCurves = ScriptArray::create<ScriptNamedVector3Curve>(numRotCurves);

		for (UINT32 i = 0; i < numRotCurves; i++)
		{
			TNamedAnimationCurve<Vector3> eulerRotationCurve;
			eulerRotationCurve.name = curves->rotation[i].name;
			eulerRotationCurve.curve = AnimationUtility::quaternionToEulerCurve(curves->rotation[i].curve);

			MonoObject* monoCurve = ScriptNamedVector3Curve::toManaged(eulerRotationCurve);
			scriptRotationCurves.set(i, monoCurve);
		}

		UINT32 numScaleCurves = (UINT32)curves->scale.size();
		ScriptArray scriptScaleCurves = ScriptArray::create<ScriptNamedVector3Curve>(numScaleCurves);

		for (UINT32 i = 0; i < numScaleCurves; i++)
		{
			MonoObject* monoCurve = ScriptNamedVector3Curve::toManaged(curves->scale[i]);
			scriptScaleCurves.set(i, monoCurve);
		}

		UINT32 numFloatCurves = (UINT32)curves->generic.size();
		ScriptArray scriptFloatCurves = ScriptArray::create<ScriptNamedFloatCurve>(numFloatCurves);

		for (UINT32 i = 0; i < numFloatCurves; i++)
		{
			MonoObject* monoCurve = ScriptNamedFloatCurve::toManaged(curves->generic[i]);
			scriptFloatCurves.set(i, monoCurve);
		}

		MonoObject* instance = metaData.scriptClass->createInstance();
		sPositionCurvesField->setValue(instance, scriptPositionCurves.getInternal());
		sRotationCurvesField->setValue(instance, scriptRotationCurves.getInternal());
		sScaleCurvesField->setValue(instance, scriptScaleCurves.getInternal());
		sFloatCurvesField->setValue(instance, scriptFloatCurves.getInternal());

		return instance;
	}

	/** Converts compound animation curves into multiple float (single value) curves. */
	template<class T, int C>
	void convertCurves(const TAnimationCurve<T>& curve, TAnimationCurve<float>(&output)[C])
	{
		UINT32 numKeyFrames = curve.getNumKeyFrames();
		Vector<TKeyframe<float>> keyFrames[C];

		for (UINT32 i = 0; i < numKeyFrames; i++)
		{
			const TKeyframe<T>& key = curve.getKeyFrame(i);

			TKeyframe<float> newKey;
			newKey.time = key.time;

			for (UINT32 j = 0; j < C; j++)
			{
				bool addNew = true;
				if (i > 0)
				{
					const TKeyframe<float>& prevKey = keyFrames[j].back();

					bool isEqual = Math::approxEquals(prevKey.value, key.value[j]) &&
						Math::approxEquals(prevKey.outTangent, key.inTangent[j]);

					addNew = !isEqual;
				}

				if (addNew)
				{
					newKey.value = key.value[j];
					newKey.inTangent = key.inTangent[j];
					newKey.outTangent = key.outTangent[j];

					keyFrames[j].push_back(newKey);
				}
			}
		}

		for (UINT32 i = 0; i < C; i++)
			output[i] = TAnimationCurve<float>(keyFrames[i]);
	}

	MonoField* ScriptNamedVector3Curve::sNameField = nullptr;
	MonoField* ScriptNamedVector3Curve::sFlagsField = nullptr;
	MonoField* ScriptNamedVector3Curve::sXCurveField = nullptr;
	MonoField* ScriptNamedVector3Curve::sYCurveField = nullptr;
	MonoField* ScriptNamedVector3Curve::sZCurveField = nullptr;

	ScriptNamedVector3Curve::ScriptNamedVector3Curve(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptNamedVector3Curve::initRuntimeData()
	{
		sNameField = metaData.scriptClass->getField("Name");
		sFlagsField = metaData.scriptClass->getField("Flags");
		sXCurveField = metaData.scriptClass->getField("X");
		sYCurveField = metaData.scriptClass->getField("Y");
		sZCurveField = metaData.scriptClass->getField("Z");
	}

	TNamedAnimationCurve<Vector3> ScriptNamedVector3Curve::toNative(MonoObject* instance)
	{
		TNamedAnimationCurve<Vector3> output;

		MonoString* monoName = nullptr;
		sNameField->getValue(instance, &monoName);

		output.name = MonoUtil::monoToString(monoName);

		UINT32 flags;
		sFlagsField->getValue(instance, &flags);
		output.flags = (AnimationCurveFlags)flags;

		// Convert from three separate floating point curves, to a Vector3 curve
		MonoObject* monoCurves[3];
		sXCurveField->getValue(instance, &monoCurves[0]);
		sYCurveField->getValue(instance, &monoCurves[1]);
		sZCurveField->getValue(instance, &monoCurves[2]);

		SPtr<TAnimationCurve<float>> curves[3];

		// Find unique keyframe times
		Map<float, TKeyframe<Vector3>> keyFrames;
		for(UINT32 i = 0; i < 3; i++)
		{
			if (monoCurves[i] == nullptr)
			{
				curves[i] = bs_shared_ptr_new<TAnimationCurve<float>>();
				continue;
			}

			ScriptAnimationCurve* scriptCurve = ScriptAnimationCurve::toNative(monoCurves[i]);
			curves[i] = scriptCurve->getInternal();

			UINT32 numKeyFrames = curves[i]->getNumKeyFrames();
			for (UINT32 j = 0; j < numKeyFrames; j++)
			{
				const TKeyframe<float>& keyFrame = curves[i]->getKeyFrame(j);

				auto iterFind = keyFrames.find(keyFrame.time);
				if (iterFind == keyFrames.end())
				{
					TKeyframe<Vector3> newKeyFrame;
					newKeyFrame.time = keyFrame.time;

					keyFrames.insert(std::make_pair(keyFrame.time, newKeyFrame));
				}
			}
		}

		// Populate keyframe values
		Vector<TKeyframe<Vector3>> keyframeList(keyFrames.size());
		UINT32 idx = 0;
		for(auto& entry : keyFrames)
		{
			TKeyframe<Vector3>& keyFrame = entry.second;
			
			for(UINT32 j = 0; j < 3; j++)
			{
				TKeyframe<float> currentKey = curves[j]->evaluateKey(keyFrame.time, false);
				keyFrame.value[j] = currentKey.value;
				keyFrame.inTangent[j] = currentKey.inTangent;
				keyFrame.outTangent[j] = currentKey.outTangent;
			}

			keyframeList[idx] = keyFrame;
			idx++;
		}

		output.curve = TAnimationCurve<Vector3>(keyframeList);
		return output;
	}

	MonoObject* ScriptNamedVector3Curve::toManaged(const TNamedAnimationCurve<Vector3>& curve)
	{
		MonoString* monoString = MonoUtil::stringToMono(curve.name);

		TAnimationCurve<float> curves[3];
		convertCurves(curve.curve, curves);

		MonoObject* monoXCurve = ScriptAnimationCurve::create(curves[0]);
		MonoObject* monoYCurve = ScriptAnimationCurve::create(curves[1]);
		MonoObject* monoZCurve = ScriptAnimationCurve::create(curves[2]);

		UINT32 flags = curve.flags;

		void* params[5] = { monoString, &flags, monoXCurve, monoYCurve, monoZCurve };
		return metaData.scriptClass->createInstance("string,int,AnimationCurve,AnimationCurve,AnimationCurve", params);
	}

	MonoField* ScriptNamedFloatCurve::sNameField = nullptr;
	MonoField* ScriptNamedFloatCurve::sFlagsField = nullptr;
	MonoField* ScriptNamedFloatCurve::sCurveField = nullptr;

	ScriptNamedFloatCurve::ScriptNamedFloatCurve(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptNamedFloatCurve::initRuntimeData()
	{
		sNameField = metaData.scriptClass->getField("Name");
		sFlagsField = metaData.scriptClass->getField("Flags");
		sCurveField = metaData.scriptClass->getField("Curve");
	}

	TNamedAnimationCurve<float> ScriptNamedFloatCurve::toNative(MonoObject* instance)
	{
		TNamedAnimationCurve<float> output;

		MonoString* monoName = nullptr;
		sNameField->getValue(instance, &monoName);

		output.name = MonoUtil::monoToString(monoName);

		UINT32 flags;
		sFlagsField->getValue(instance, &flags);
		output.flags = (AnimationCurveFlags)flags;

		MonoObject* monoCurve = nullptr;
		sCurveField->getValue(instance, &monoCurve);

		if(monoCurve != nullptr)
		{
			ScriptAnimationCurve* scriptCurve = ScriptAnimationCurve::toNative(monoCurve);
			output.curve = *scriptCurve->getInternal();
		}

		return output;
	}

	MonoObject* ScriptNamedFloatCurve::toManaged(const TNamedAnimationCurve<float>& curve)
	{
		MonoString* monoString = MonoUtil::stringToMono(curve.name);
		MonoObject* monoCurve = ScriptAnimationCurve::create(curve.curve);

		UINT32 flags = curve.flags;
		void* params[3] = { monoString, &flags, monoCurve };
		return metaData.scriptClass->createInstance("string,int,AnimationCurve", params);
	}

	ScriptVector3Curve::ScriptVector3Curve(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptVector3Curve::initRuntimeData()
	{ }

	MonoObject* ScriptVector3Curve::toManaged(const TAnimationCurve<Vector3>& curve)
	{
		TAnimationCurve<float> curves[3];
		convertCurves(curve, curves);

		MonoObject* monoXCurve = ScriptAnimationCurve::create(curves[0]);
		MonoObject* monoYCurve = ScriptAnimationCurve::create(curves[1]);
		MonoObject* monoZCurve = ScriptAnimationCurve::create(curves[2]);

		void* params[3] = { monoXCurve, monoYCurve, monoZCurve };
		return metaData.scriptClass->createInstance("AnimationCurve,AnimationCurve,AnimationCurve", params);
	}

	ScriptQuaternionCurve::ScriptQuaternionCurve(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptQuaternionCurve::initRuntimeData()
	{ }

	MonoObject* ScriptQuaternionCurve::toManaged(const TAnimationCurve<Quaternion>& curve)
	{
		TAnimationCurve<float> curves[4];
		convertCurves(curve, curves);

		MonoObject* monoXCurve = ScriptAnimationCurve::create(curves[0]);
		MonoObject* monoYCurve = ScriptAnimationCurve::create(curves[1]);
		MonoObject* monoZCurve = ScriptAnimationCurve::create(curves[2]);
		MonoObject* monoWCurve = ScriptAnimationCurve::create(curves[3]);

		void* params[4] = { monoXCurve, monoYCurve, monoZCurve, monoWCurve };
		return metaData.scriptClass->createInstance("AnimationCurve,AnimationCurve,AnimationCurve,AnimationCurve", params);
	}

	MonoField* ScriptRootMotion::sPositionField = nullptr;
	MonoField* ScriptRootMotion::sRotationField = nullptr;

	ScriptRootMotion::ScriptRootMotion(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptRootMotion::initRuntimeData()
	{
		sPositionField = metaData.scriptClass->getField("Position");
		sRotationField = metaData.scriptClass->getField("Rotation");
	}

	MonoObject* ScriptRootMotion::toManaged(const SPtr<RootMotion>& rootMotion)
	{
		MonoObject* monoPositionCurve = ScriptVector3Curve::toManaged(rootMotion->position);
		MonoObject* monoRotationCurve = ScriptQuaternionCurve::toManaged(rootMotion->rotation);

		MonoObject* instance = metaData.scriptClass->createInstance();
		sPositionField->setValue(instance, monoPositionCurve);
		sRotationField->setValue(instance, monoRotationCurve);

		return instance;
	}
}