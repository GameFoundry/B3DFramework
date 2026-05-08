//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptRandom.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Engine/Utility/Math/B3DRandom.h"
#include "B3DScriptTVector3.generated.h"
#include "B3DScriptTVector2.generated.h"

namespace b3d
{
	ScriptRandom::ScriptRandom(const TShared<Random>& nativeObject)
		:TScriptNonReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptRandom::~ScriptRandom()
	{
		UnregisterEvents();
	}

	void ScriptRandom::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Random", (void*)&ScriptRandom::InternalRandom);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetSeed", (void*)&ScriptRandom::InternalSetSeed);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Get", (void*)&ScriptRandom::InternalGet);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetRange", (void*)&ScriptRandom::InternalGetRange);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetUNorm", (void*)&ScriptRandom::InternalGetUNorm);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetSNorm", (void*)&ScriptRandom::InternalGetSNorm);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetUnitVector", (void*)&ScriptRandom::InternalGetUnitVector);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetUnitVector2D", (void*)&ScriptRandom::InternalGetUnitVector2D);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetPointInSphere", (void*)&ScriptRandom::InternalGetPointInSphere);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetPointInSphereShell", (void*)&ScriptRandom::InternalGetPointInSphereShell);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetPointInCircle", (void*)&ScriptRandom::InternalGetPointInCircle);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetPointInCircleShell", (void*)&ScriptRandom::InternalGetPointInCircleShell);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetPointInArc", (void*)&ScriptRandom::InternalGetPointInArc);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetPointInArcShell", (void*)&ScriptRandom::InternalGetPointInArcShell);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetBarycentric", (void*)&ScriptRandom::InternalGetBarycentric);

	}

	MonoObject* ScriptRandom::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptRandom::InternalRandom(MonoObject* scriptObject, uint32_t seed)
	{
		TShared<Random> nativeObject = B3DMakeShared<Random>(seed);
		ScriptObjectWrapper::Create<ScriptRandom>(nativeObject, scriptObject);
	}

	void ScriptRandom::InternalSetSeed(ScriptRandom* self, uint32_t seed)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<Random*>(self->GetNativeObject())->SetSeed(seed);
	}

	uint32_t ScriptRandom::InternalGet(ScriptRandom* self)
	{
		uint32_t tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<Random*>(self->GetNativeObject())->Get();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	int32_t ScriptRandom::InternalGetRange(ScriptRandom* self, int32_t min, int32_t max)
	{
		int32_t tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<Random*>(self->GetNativeObject())->GetRange(min, max);

		int32_t __output;
		__output = tmp__output;

		return __output;
	}

	float ScriptRandom::InternalGetUNorm(ScriptRandom* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<Random*>(self->GetNativeObject())->GetUNorm();

		float __output;
		__output = tmp__output;

		return __output;
	}

	float ScriptRandom::InternalGetSNorm(ScriptRandom* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<Random*>(self->GetNativeObject())->GetSNorm();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRandom::InternalGetUnitVector(ScriptRandom* self, TVector3<float>* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TVector3<float> tmp__output;
		tmp__output = static_cast<Random*>(self->GetNativeObject())->GetUnitVector();

		*__output = tmp__output;
	}

	void ScriptRandom::InternalGetUnitVector2D(ScriptRandom* self, TVector2<float>* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TVector2<float> tmp__output;
		tmp__output = static_cast<Random*>(self->GetNativeObject())->GetUnitVector2D();

		*__output = tmp__output;
	}

	void ScriptRandom::InternalGetPointInSphere(ScriptRandom* self, TVector3<float>* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TVector3<float> tmp__output;
		tmp__output = static_cast<Random*>(self->GetNativeObject())->GetPointInSphere();

		*__output = tmp__output;
	}

	void ScriptRandom::InternalGetPointInSphereShell(ScriptRandom* self, float thickness, TVector3<float>* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TVector3<float> tmp__output;
		tmp__output = static_cast<Random*>(self->GetNativeObject())->GetPointInSphereShell(thickness);

		*__output = tmp__output;
	}

	void ScriptRandom::InternalGetPointInCircle(ScriptRandom* self, TVector2<float>* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TVector2<float> tmp__output;
		tmp__output = static_cast<Random*>(self->GetNativeObject())->GetPointInCircle();

		*__output = tmp__output;
	}

	void ScriptRandom::InternalGetPointInCircleShell(ScriptRandom* self, float thickness, TVector2<float>* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TVector2<float> tmp__output;
		tmp__output = static_cast<Random*>(self->GetNativeObject())->GetPointInCircleShell(thickness);

		*__output = tmp__output;
	}

	void ScriptRandom::InternalGetPointInArc(ScriptRandom* self, TDegree<float>* angle, TVector2<float>* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TVector2<float> tmp__output;
		tmp__output = static_cast<Random*>(self->GetNativeObject())->GetPointInArc(*angle);

		*__output = tmp__output;
	}

	void ScriptRandom::InternalGetPointInArcShell(ScriptRandom* self, TDegree<float>* angle, float thickness, TVector2<float>* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TVector2<float> tmp__output;
		tmp__output = static_cast<Random*>(self->GetNativeObject())->GetPointInArcShell(*angle, thickness);

		*__output = tmp__output;
	}

	void ScriptRandom::InternalGetBarycentric(ScriptRandom* self, TVector3<float>* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TVector3<float> tmp__output;
		tmp__output = static_cast<Random*>(self->GetNativeObject())->GetBarycentric();

		*__output = tmp__output;
	}
}
