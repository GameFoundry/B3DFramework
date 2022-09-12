//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptRandom.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfUtility/Math/BsRandom.h"
#include "Wrappers/BsScriptVector.h"
#include "Wrappers/BsScriptVector.h"

namespace bs
{
	ScriptRandom::ScriptRandom(MonoObject* managedInstance, const SPtr<Random>& value)
		:ScriptObject(managedInstance), mInternal(value)
	{
	}

	void ScriptRandom::InitRuntimeData()
	{
		metaData.scriptClass->AddInternalCall("Internal_Random", (void*)&ScriptRandom::Internal_Random);
		metaData.scriptClass->AddInternalCall("Internal_setSeed", (void*)&ScriptRandom::Internal_setSeed);
		metaData.scriptClass->AddInternalCall("Internal_get", (void*)&ScriptRandom::Internal_get);
		metaData.scriptClass->AddInternalCall("Internal_getRange", (void*)&ScriptRandom::Internal_getRange);
		metaData.scriptClass->AddInternalCall("Internal_getUNorm", (void*)&ScriptRandom::Internal_getUNorm);
		metaData.scriptClass->AddInternalCall("Internal_getSNorm", (void*)&ScriptRandom::Internal_getSNorm);
		metaData.scriptClass->AddInternalCall("Internal_getUnitVector", (void*)&ScriptRandom::Internal_getUnitVector);
		metaData.scriptClass->AddInternalCall("Internal_getUnitVector2D", (void*)&ScriptRandom::Internal_getUnitVector2D);
		metaData.scriptClass->AddInternalCall("Internal_getPointInSphere", (void*)&ScriptRandom::Internal_getPointInSphere);
		metaData.scriptClass->AddInternalCall("Internal_getPointInSphereShell", (void*)&ScriptRandom::Internal_getPointInSphereShell);
		metaData.scriptClass->AddInternalCall("Internal_getPointInCircle", (void*)&ScriptRandom::Internal_getPointInCircle);
		metaData.scriptClass->AddInternalCall("Internal_getPointInCircleShell", (void*)&ScriptRandom::Internal_getPointInCircleShell);
		metaData.scriptClass->AddInternalCall("Internal_getPointInArc", (void*)&ScriptRandom::Internal_getPointInArc);
		metaData.scriptClass->AddInternalCall("Internal_getPointInArcShell", (void*)&ScriptRandom::Internal_getPointInArcShell);
		metaData.scriptClass->AddInternalCall("Internal_getBarycentric", (void*)&ScriptRandom::Internal_getBarycentric);

	}

	MonoObject* ScriptRandom::create(const SPtr<Random>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.scriptClass->CreateInstance("bool", ctorParams);
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
		thisPtr->GetInternal()->setSeed(seed);
	}

	uint32_t ScriptRandom::Internal_get(ScriptRandom* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->GetInternal()->get();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	int32_t ScriptRandom::Internal_getRange(ScriptRandom* thisPtr, int32_t min, int32_t max)
	{
		int32_t tmp__output;
		tmp__output = thisPtr->GetInternal()->getRange(min, max);

		int32_t __output;
		__output = tmp__output;

		return __output;
	}

	float ScriptRandom::Internal_getUNorm(ScriptRandom* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->getUNorm();

		float __output;
		__output = tmp__output;

		return __output;
	}

	float ScriptRandom::Internal_getSNorm(ScriptRandom* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->getSNorm();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRandom::Internal_getUnitVector(ScriptRandom* thisPtr, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->GetInternal()->getUnitVector();

		*__output = tmp__output;
	}

	void ScriptRandom::Internal_getUnitVector2D(ScriptRandom* thisPtr, Vector2* __output)
	{
		Vector2 tmp__output;
		tmp__output = thisPtr->GetInternal()->getUnitVector2D();

		*__output = tmp__output;
	}

	void ScriptRandom::Internal_getPointInSphere(ScriptRandom* thisPtr, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->GetInternal()->getPointInSphere();

		*__output = tmp__output;
	}

	void ScriptRandom::Internal_getPointInSphereShell(ScriptRandom* thisPtr, float thickness, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->GetInternal()->getPointInSphereShell(thickness);

		*__output = tmp__output;
	}

	void ScriptRandom::Internal_getPointInCircle(ScriptRandom* thisPtr, Vector2* __output)
	{
		Vector2 tmp__output;
		tmp__output = thisPtr->GetInternal()->getPointInCircle();

		*__output = tmp__output;
	}

	void ScriptRandom::Internal_getPointInCircleShell(ScriptRandom* thisPtr, float thickness, Vector2* __output)
	{
		Vector2 tmp__output;
		tmp__output = thisPtr->GetInternal()->getPointInCircleShell(thickness);

		*__output = tmp__output;
	}

	void ScriptRandom::Internal_getPointInArc(ScriptRandom* thisPtr, Degree* angle, Vector2* __output)
	{
		Vector2 tmp__output;
		tmp__output = thisPtr->GetInternal()->getPointInArc(*angle);

		*__output = tmp__output;
	}

	void ScriptRandom::Internal_getPointInArcShell(ScriptRandom* thisPtr, Degree* angle, float thickness, Vector2* __output)
	{
		Vector2 tmp__output;
		tmp__output = thisPtr->GetInternal()->getPointInArcShell(*angle, thickness);

		*__output = tmp__output;
	}

	void ScriptRandom::Internal_getBarycentric(ScriptRandom* thisPtr, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->GetInternal()->getBarycentric();

		*__output = tmp__output;
	}
}
