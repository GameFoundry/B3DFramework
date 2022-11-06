//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
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
		metaData.ScriptClass->AddInternalCall("Internal_Random", (void*)&ScriptRandom::InternalRandom);
		metaData.ScriptClass->AddInternalCall("Internal_SetSeed", (void*)&ScriptRandom::InternalSetSeed);
		metaData.ScriptClass->AddInternalCall("Internal_Get", (void*)&ScriptRandom::InternalGet);
		metaData.ScriptClass->AddInternalCall("Internal_GetRange", (void*)&ScriptRandom::InternalGetRange);
		metaData.ScriptClass->AddInternalCall("Internal_GetUNorm", (void*)&ScriptRandom::InternalGetUNorm);
		metaData.ScriptClass->AddInternalCall("Internal_GetSNorm", (void*)&ScriptRandom::InternalGetSNorm);
		metaData.ScriptClass->AddInternalCall("Internal_GetUnitVector", (void*)&ScriptRandom::InternalGetUnitVector);
		metaData.ScriptClass->AddInternalCall("Internal_GetUnitVector2D", (void*)&ScriptRandom::InternalGetUnitVector2D);
		metaData.ScriptClass->AddInternalCall("Internal_GetPointInSphere", (void*)&ScriptRandom::InternalGetPointInSphere);
		metaData.ScriptClass->AddInternalCall("Internal_GetPointInSphereShell", (void*)&ScriptRandom::InternalGetPointInSphereShell);
		metaData.ScriptClass->AddInternalCall("Internal_GetPointInCircle", (void*)&ScriptRandom::InternalGetPointInCircle);
		metaData.ScriptClass->AddInternalCall("Internal_GetPointInCircleShell", (void*)&ScriptRandom::InternalGetPointInCircleShell);
		metaData.ScriptClass->AddInternalCall("Internal_GetPointInArc", (void*)&ScriptRandom::InternalGetPointInArc);
		metaData.ScriptClass->AddInternalCall("Internal_GetPointInArcShell", (void*)&ScriptRandom::InternalGetPointInArcShell);
		metaData.ScriptClass->AddInternalCall("Internal_GetBarycentric", (void*)&ScriptRandom::InternalGetBarycentric);

	}

	MonoObject* ScriptRandom::Create(const SPtr<Random>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
		new (B3DAllocate<ScriptRandom>()) ScriptRandom(managedInstance, value);
		return managedInstance;
	}
	void ScriptRandom::InternalRandom(MonoObject* managedInstance, uint32_t seed)
	{
		SPtr<Random> instance = B3DMakeShared<Random>(seed);
		new (B3DAllocate<ScriptRandom>())ScriptRandom(managedInstance, instance);
	}

	void ScriptRandom::InternalSetSeed(ScriptRandom* thisPtr, uint32_t seed)
	{
		thisPtr->GetInternal()->SetSeed(seed);
	}

	uint32_t ScriptRandom::InternalGet(ScriptRandom* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->GetInternal()->Get();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	int32_t ScriptRandom::InternalGetRange(ScriptRandom* thisPtr, int32_t min, int32_t max)
	{
		int32_t tmp__output;
		tmp__output = thisPtr->GetInternal()->GetRange(min, max);

		int32_t __output;
		__output = tmp__output;

		return __output;
	}

	float ScriptRandom::InternalGetUNorm(ScriptRandom* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->GetUNorm();

		float __output;
		__output = tmp__output;

		return __output;
	}

	float ScriptRandom::InternalGetSNorm(ScriptRandom* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->GetSNorm();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRandom::InternalGetUnitVector(ScriptRandom* thisPtr, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->GetInternal()->GetUnitVector();

		*__output = tmp__output;
	}

	void ScriptRandom::InternalGetUnitVector2D(ScriptRandom* thisPtr, Vector2* __output)
	{
		Vector2 tmp__output;
		tmp__output = thisPtr->GetInternal()->GetUnitVector2D();

		*__output = tmp__output;
	}

	void ScriptRandom::InternalGetPointInSphere(ScriptRandom* thisPtr, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->GetInternal()->GetPointInSphere();

		*__output = tmp__output;
	}

	void ScriptRandom::InternalGetPointInSphereShell(ScriptRandom* thisPtr, float thickness, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->GetInternal()->GetPointInSphereShell(thickness);

		*__output = tmp__output;
	}

	void ScriptRandom::InternalGetPointInCircle(ScriptRandom* thisPtr, Vector2* __output)
	{
		Vector2 tmp__output;
		tmp__output = thisPtr->GetInternal()->GetPointInCircle();

		*__output = tmp__output;
	}

	void ScriptRandom::InternalGetPointInCircleShell(ScriptRandom* thisPtr, float thickness, Vector2* __output)
	{
		Vector2 tmp__output;
		tmp__output = thisPtr->GetInternal()->GetPointInCircleShell(thickness);

		*__output = tmp__output;
	}

	void ScriptRandom::InternalGetPointInArc(ScriptRandom* thisPtr, Degree* angle, Vector2* __output)
	{
		Vector2 tmp__output;
		tmp__output = thisPtr->GetInternal()->GetPointInArc(*angle);

		*__output = tmp__output;
	}

	void ScriptRandom::InternalGetPointInArcShell(ScriptRandom* thisPtr, Degree* angle, float thickness, Vector2* __output)
	{
		Vector2 tmp__output;
		tmp__output = thisPtr->GetInternal()->GetPointInArcShell(*angle, thickness);

		*__output = tmp__output;
	}

	void ScriptRandom::InternalGetBarycentric(ScriptRandom* thisPtr, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->GetInternal()->GetBarycentric();

		*__output = tmp__output;
	}
}
