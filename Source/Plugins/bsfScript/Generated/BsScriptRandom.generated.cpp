//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptRandom.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../../../Foundation/bsfUtility/Math/BsRandom.h"
#include "Wrappers/BsScriptVector.h"
#include "Wrappers/BsScriptVector.h"

namespace bs
{
	ScriptRandom::ScriptRandom(MonoObject* managedInstance, const SPtr<Random>& value)
		:ScriptObject(managedInstance), mInternal(value)
	{
	}

	void ScriptRandom::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_Random", (void*)&ScriptRandom::Internal_Random);
		metaData.scriptClass->addInternalCall("Internal_setSeed", (void*)&ScriptRandom::Internal_setSeed);
		metaData.scriptClass->addInternalCall("Internal_get", (void*)&ScriptRandom::Internal_get);
		metaData.scriptClass->addInternalCall("Internal_getRange", (void*)&ScriptRandom::Internal_getRange);
		metaData.scriptClass->addInternalCall("Internal_getUNorm", (void*)&ScriptRandom::Internal_getUNorm);
		metaData.scriptClass->addInternalCall("Internal_getSNorm", (void*)&ScriptRandom::Internal_getSNorm);
		metaData.scriptClass->addInternalCall("Internal_getUnitVector", (void*)&ScriptRandom::Internal_getUnitVector);
		metaData.scriptClass->addInternalCall("Internal_getUnitVector2D", (void*)&ScriptRandom::Internal_getUnitVector2D);
		metaData.scriptClass->addInternalCall("Internal_getPointInSphere", (void*)&ScriptRandom::Internal_getPointInSphere);
		metaData.scriptClass->addInternalCall("Internal_getPointInSphereShell", (void*)&ScriptRandom::Internal_getPointInSphereShell);
		metaData.scriptClass->addInternalCall("Internal_getPointInCircle", (void*)&ScriptRandom::Internal_getPointInCircle);
		metaData.scriptClass->addInternalCall("Internal_getPointInCircleShell", (void*)&ScriptRandom::Internal_getPointInCircleShell);
		metaData.scriptClass->addInternalCall("Internal_getPointInArc", (void*)&ScriptRandom::Internal_getPointInArc);
		metaData.scriptClass->addInternalCall("Internal_getPointInArcShell", (void*)&ScriptRandom::Internal_getPointInArcShell);
		metaData.scriptClass->addInternalCall("Internal_getBarycentric", (void*)&ScriptRandom::Internal_getBarycentric);

	}

	MonoObject* ScriptRandom::create(const SPtr<Random>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.scriptClass->createInstance("bool", ctorParams);
		new (bs_alloc<ScriptRandom>()) ScriptRandom(managedInstance, value);
		return managedInstance;
	}
	void ScriptRandom::Internal_Random(MonoObject* managedInstance, uint32_t seed)
	{
		SPtr<Random> instance = bs_shared_ptr_new<Random>(seed);
		new (bs_alloc<ScriptRandom>())ScriptRandom(managedInstance, instance);
	}

	void ScriptRandom::Internal_setSeed(ScriptRandom* thisPtr, uint32_t seed)
	{
		thisPtr->getInternal()->setSeed(seed);
	}

	uint32_t ScriptRandom::Internal_get(ScriptRandom* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->getInternal()->get();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	int32_t ScriptRandom::Internal_getRange(ScriptRandom* thisPtr, int32_t min, int32_t max)
	{
		int32_t tmp__output;
		tmp__output = thisPtr->getInternal()->getRange(min, max);

		int32_t __output;
		__output = tmp__output;

		return __output;
	}

	float ScriptRandom::Internal_getUNorm(ScriptRandom* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->getInternal()->getUNorm();

		float __output;
		__output = tmp__output;

		return __output;
	}

	float ScriptRandom::Internal_getSNorm(ScriptRandom* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->getInternal()->getSNorm();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRandom::Internal_getUnitVector(ScriptRandom* thisPtr, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->getInternal()->getUnitVector();

		*__output = tmp__output;
	}

	void ScriptRandom::Internal_getUnitVector2D(ScriptRandom* thisPtr, Vector2* __output)
	{
		Vector2 tmp__output;
		tmp__output = thisPtr->getInternal()->getUnitVector2D();

		*__output = tmp__output;
	}

	void ScriptRandom::Internal_getPointInSphere(ScriptRandom* thisPtr, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->getInternal()->getPointInSphere();

		*__output = tmp__output;
	}

	void ScriptRandom::Internal_getPointInSphereShell(ScriptRandom* thisPtr, float thickness, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->getInternal()->getPointInSphereShell(thickness);

		*__output = tmp__output;
	}

	void ScriptRandom::Internal_getPointInCircle(ScriptRandom* thisPtr, Vector2* __output)
	{
		Vector2 tmp__output;
		tmp__output = thisPtr->getInternal()->getPointInCircle();

		*__output = tmp__output;
	}

	void ScriptRandom::Internal_getPointInCircleShell(ScriptRandom* thisPtr, float thickness, Vector2* __output)
	{
		Vector2 tmp__output;
		tmp__output = thisPtr->getInternal()->getPointInCircleShell(thickness);

		*__output = tmp__output;
	}

	void ScriptRandom::Internal_getPointInArc(ScriptRandom* thisPtr, Degree* angle, Vector2* __output)
	{
		Vector2 tmp__output;
		tmp__output = thisPtr->getInternal()->getPointInArc(*angle);

		*__output = tmp__output;
	}

	void ScriptRandom::Internal_getPointInArcShell(ScriptRandom* thisPtr, Degree* angle, float thickness, Vector2* __output)
	{
		Vector2 tmp__output;
		tmp__output = thisPtr->getInternal()->getPointInArcShell(*angle, thickness);

		*__output = tmp__output;
	}

	void ScriptRandom::Internal_getBarycentric(ScriptRandom* thisPtr, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->getInternal()->getBarycentric();

		*__output = tmp__output;
	}
}
