//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptPackageResourceMetaData.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "Reflection/B3DRTTIType.h"
#include "B3DScriptResourceWrapper.h"
#include "B3DScriptPackageResourceUserMetaData.generated.h"
#include "B3DScriptResourceMetaData.generated.h"
#include "../Extensions/B3DPackageResourceMetaDataExtension.h"

namespace b3d
{
	ScriptPackageResourceMetaData::ScriptPackageResourceMetaData(const TShared<PackageResourceMetaData>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptPackageResourceMetaData::~ScriptPackageResourceMetaData()
	{
		UnregisterEvents();
	}

	void ScriptPackageResourceMetaData::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetResourceName", (void*)&ScriptPackageResourceMetaData::InternalGetResourceName);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetPath", (void*)&ScriptPackageResourceMetaData::InternalGetPath);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetPath", (void*)&ScriptPackageResourceMetaData::InternalSetPath);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetId", (void*)&ScriptPackageResourceMetaData::InternalGetId);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetId", (void*)&ScriptPackageResourceMetaData::InternalSetId);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetTypeId", (void*)&ScriptPackageResourceMetaData::InternalGetTypeId);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetTypeId", (void*)&ScriptPackageResourceMetaData::InternalSetTypeId);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetDependencies", (void*)&ScriptPackageResourceMetaData::InternalGetDependencies);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetDependencies", (void*)&ScriptPackageResourceMetaData::InternalSetDependencies);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetCompressionType", (void*)&ScriptPackageResourceMetaData::InternalGetCompressionType);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetCompressionType", (void*)&ScriptPackageResourceMetaData::InternalSetCompressionType);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetFlags", (void*)&ScriptPackageResourceMetaData::InternalGetFlags);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetFlags", (void*)&ScriptPackageResourceMetaData::InternalSetFlags);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetAdditionalMetaData", (void*)&ScriptPackageResourceMetaData::InternalGetAdditionalMetaData);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetAdditionalMetaData", (void*)&ScriptPackageResourceMetaData::InternalSetAdditionalMetaData);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetResourceMetaData", (void*)&ScriptPackageResourceMetaData::InternalGetResourceMetaData);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetResourceMetaData", (void*)&ScriptPackageResourceMetaData::InternalSetResourceMetaData);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetResourceType", (void*)&ScriptPackageResourceMetaData::InternalGetResourceType);

	}

	MonoObject* ScriptPackageResourceMetaData::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	MonoString* ScriptPackageResourceMetaData::InternalGetResourceName(ScriptPackageResourceMetaData* self)
	{
		String tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<PackageResourceMetaData*>(self->GetNativeObject())->GetResourceName();

		MonoString* __output;
		__output = MonoUtil::StringToMono(tmp__output);

		return __output;
	}

	MonoReflectionType* ScriptPackageResourceMetaData::InternalGetResourceType(ScriptPackageResourceMetaData* self)
	{
		_MonoReflectionType* tmp__output = nullptr;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = PackageResourceMetaDataExtension::GetResourceType(std::static_pointer_cast<PackageResourceMetaData>(self->GetBaseNativeObjectAsShared()));

		MonoReflectionType* __output;
		__output = tmp__output;

		return __output;
	}
	MonoString* ScriptPackageResourceMetaData::InternalGetPath(ScriptPackageResourceMetaData* self)
	{
		Path tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<PackageResourceMetaData*>(self->GetNativeObject())->Path;

		MonoString* __output;
		__output = MonoUtil::StringToMono(tmp__output.ToString());

		return __output;
	}

	void ScriptPackageResourceMetaData::InternalSetPath(ScriptPackageResourceMetaData* self, MonoString* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		Path tmpvalue;
		tmpvalue = MonoUtil::MonoToString(value);
		static_cast<PackageResourceMetaData*>(self->GetNativeObject())->Path = tmpvalue;
	}

	void ScriptPackageResourceMetaData::InternalGetId(ScriptPackageResourceMetaData* self, UUID* __output)
	{
		UUID tmp__output;
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		tmp__output = static_cast<PackageResourceMetaData*>(self->GetNativeObject())->Id;

		*__output = tmp__output;


	}

	void ScriptPackageResourceMetaData::InternalSetId(ScriptPackageResourceMetaData* self, UUID* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<PackageResourceMetaData*>(self->GetNativeObject())->Id = *value;
	}

	uint32_t ScriptPackageResourceMetaData::InternalGetTypeId(ScriptPackageResourceMetaData* self)
	{
		uint32_t tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<PackageResourceMetaData*>(self->GetNativeObject())->TypeId;

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptPackageResourceMetaData::InternalSetTypeId(ScriptPackageResourceMetaData* self, uint32_t value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<PackageResourceMetaData*>(self->GetNativeObject())->TypeId = value;
	}

	MonoArray* ScriptPackageResourceMetaData::InternalGetDependencies(ScriptPackageResourceMetaData* self)
	{
		Vector<UUID> nativeArray__output;
		if(!self->IsNativeObjectValid())
			return {};

		nativeArray__output = static_cast<PackageResourceMetaData*>(self->GetNativeObject())->Dependencies;

		MonoArray* __output;
		int elementCount__output = (int)nativeArray__output.size();
		ScriptArray scriptArray__output = ScriptArray::Create<ScriptUUID>(elementCount__output);
		for(int elementIndex = 0; elementIndex < elementCount__output; elementIndex++)
		{
			scriptArray__output.Set(elementIndex, nativeArray__output[elementIndex]);
		}
		__output = scriptArray__output.GetInternal();

		return __output;
	}

	void ScriptPackageResourceMetaData::InternalSetDependencies(ScriptPackageResourceMetaData* self, MonoArray* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		Vector<UUID> nativeArrayvalue;
		if(value != nullptr)
		{
			ScriptArray scriptArrayvalue(value);
			nativeArrayvalue.resize(scriptArrayvalue.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArrayvalue.Size(); elementIndex++)
			{
				nativeArrayvalue[elementIndex] = scriptArrayvalue.Get<UUID>(elementIndex);
			}

		}
		static_cast<PackageResourceMetaData*>(self->GetNativeObject())->Dependencies = nativeArrayvalue;
	}

	CompressionType ScriptPackageResourceMetaData::InternalGetCompressionType(ScriptPackageResourceMetaData* self)
	{
		CompressionType tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<PackageResourceMetaData*>(self->GetNativeObject())->CompressionType;

		CompressionType __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptPackageResourceMetaData::InternalSetCompressionType(ScriptPackageResourceMetaData* self, CompressionType value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<PackageResourceMetaData*>(self->GetNativeObject())->CompressionType = value;
	}

	PackageResourceFlag ScriptPackageResourceMetaData::InternalGetFlags(ScriptPackageResourceMetaData* self)
	{
		Flags<PackageResourceFlag> tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<PackageResourceMetaData*>(self->GetNativeObject())->Flags;

		PackageResourceFlag __output;
		__output = (PackageResourceFlag)(uint32_t)tmp__output;

		return __output;
	}

	void ScriptPackageResourceMetaData::InternalSetFlags(ScriptPackageResourceMetaData* self, PackageResourceFlag value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<PackageResourceMetaData*>(self->GetNativeObject())->Flags = value;
	}

	MonoObject* ScriptPackageResourceMetaData::InternalGetAdditionalMetaData(ScriptPackageResourceMetaData* self)
	{
		TShared<PackageResourceUserMetaData> tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<PackageResourceMetaData*>(self->GetNativeObject())->AdditionalMetaData;

		MonoObject* __output;
		__output = ScriptPackageResourceUserMetaData::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	void ScriptPackageResourceMetaData::InternalSetAdditionalMetaData(ScriptPackageResourceMetaData* self, MonoObject* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		TShared<PackageResourceUserMetaData> tmpvalue;
		ScriptPackageResourceUserMetaDataWrapperBase* scriptObjectWrappervalue;
		scriptObjectWrappervalue = (ScriptPackageResourceUserMetaDataWrapperBase*)ScriptPackageResourceUserMetaData::GetScriptObjectWrapper(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = std::static_pointer_cast<PackageResourceUserMetaData>(scriptObjectWrappervalue->GetBaseNativeObjectAsShared());
		static_cast<PackageResourceMetaData*>(self->GetNativeObject())->AdditionalMetaData = tmpvalue;
	}

	MonoObject* ScriptPackageResourceMetaData::InternalGetResourceMetaData(ScriptPackageResourceMetaData* self)
	{
		TShared<ResourceMetaData> tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<PackageResourceMetaData*>(self->GetNativeObject())->ResourceMetaData;

		MonoObject* __output;
		__output = ScriptResourceMetaData::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	void ScriptPackageResourceMetaData::InternalSetResourceMetaData(ScriptPackageResourceMetaData* self, MonoObject* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		TShared<ResourceMetaData> tmpvalue;
		ScriptResourceMetaDataWrapperBase* scriptObjectWrappervalue;
		scriptObjectWrappervalue = (ScriptResourceMetaDataWrapperBase*)ScriptResourceMetaData::GetScriptObjectWrapper(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = std::static_pointer_cast<ResourceMetaData>(scriptObjectWrappervalue->GetBaseNativeObjectAsShared());
		static_cast<PackageResourceMetaData*>(self->GetNativeObject())->ResourceMetaData = tmpvalue;
	}
}
