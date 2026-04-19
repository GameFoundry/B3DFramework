//********************************* B3D Framework - Copyright 2026 Marko Pintera *****************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Extensions/B3DPackageResourceMetaDataExtension.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "Serialization/B3DScriptAssemblyManager.h"
#include "B3DManagedResourceMetaData.h"

using namespace b3d;

MonoReflectionType* PackageResourceMetaDataExtension::GetResourceType(const SPtr<PackageResourceMetaData>& self)
{
	if(!B3D_ENSURE(self != nullptr))
		return nullptr;

	const u32 typeId = self->TypeId;
	if(typeId == TID_ManagedResource)
	{
		const auto metaData = B3DRTTICast<ManagedResourceMetaData>(self->ResourceMetaData);

		if(metaData)
		{
			MonoClass* providedClass = MonoManager::Instance().FindClass(metaData->TypeNamespace, metaData->TypeName);

			if(providedClass)
				return MonoUtil::GetType(providedClass->GetInternalClass());
		}
	}
	else
	{
		const ScriptTypeMetaData* const scriptWrapperObjectMetaData = ScriptAssemblyManager::Instance().GetScriptWrapperMetaData(self->TypeId);

		if(scriptWrapperObjectMetaData)
			return MonoUtil::GetType(scriptWrapperObjectMetaData->ScriptClass->GetInternalClass());
	}

	return nullptr;
}

