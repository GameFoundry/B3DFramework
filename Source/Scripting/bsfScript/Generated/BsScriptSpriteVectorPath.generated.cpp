//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptSpriteVectorPath.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Image/BsSpriteVectorPath.h"
#include "BsScriptResourceManager.h"
#include "Wrappers/BsScriptRRefBase.h"
#include "../../../Foundation/bsfCore/VectorGraphics/BsVectorGraphics.h"
#include "../../../Foundation/bsfCore/Image/BsSpriteVectorPath.h"
#include "Wrappers/BsScriptSize.h"
#include "BsScriptSpriteVectorPathCreateInformation.generated.h"

namespace bs
{
	ScriptSpriteVectorPath::ScriptSpriteVectorPath(MonoObject* managedInstance, const TResourceHandle<SpriteVectorPath>& value)
		:TScriptResource(managedInstance, value)
	{
	}

	void ScriptSpriteVectorPath::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_GetRef", (void*)&ScriptSpriteVectorPath::InternalGetRef);
		metaData.ScriptClass->AddInternalCall("Internal_SetVectorPath", (void*)&ScriptSpriteVectorPath::InternalSetVectorPath);
		metaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptSpriteVectorPath::InternalCreate);
		metaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptSpriteVectorPath::InternalCreate0);

	}

	 MonoObject*ScriptSpriteVectorPath::CreateInstance()
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		return metaData.ScriptClass->CreateInstance("bool", ctorParams);
	}
	MonoObject* ScriptSpriteVectorPath::InternalGetRef(ScriptSpriteVectorPath* thisPtr)
	{
		return thisPtr->GetRRef();
	}

	void ScriptSpriteVectorPath::InternalSetVectorPath(ScriptSpriteVectorPath* thisPtr, MonoObject* vectorPath)
	{
		TResourceHandle<VectorPath> tmpvectorPath;
		ScriptRRefBase* scriptvectorPath;
		scriptvectorPath = ScriptRRefBase::ToNative(vectorPath);
		if(scriptvectorPath != nullptr)
			tmpvectorPath = B3DStaticResourceCast<VectorPath>(scriptvectorPath->GetHandle());
		thisPtr->GetHandle()->SetVectorPath(tmpvectorPath);
	}

	void ScriptSpriteVectorPath::InternalCreate(MonoObject* managedInstance, MonoObject* vectorPath, TSize2<uint32_t>* size)
	{
		TResourceHandle<VectorPath> tmpvectorPath;
		ScriptRRefBase* scriptvectorPath;
		scriptvectorPath = ScriptRRefBase::ToNative(vectorPath);
		if(scriptvectorPath != nullptr)
			tmpvectorPath = B3DStaticResourceCast<VectorPath>(scriptvectorPath->GetHandle());
		TResourceHandle<SpriteVectorPath> instance = SpriteVectorPath::Create(tmpvectorPath, *size);
		ScriptResourceManager::Instance().CreateBuiltinScriptResource(instance, managedInstance);
	}

	void ScriptSpriteVectorPath::InternalCreate0(MonoObject* managedInstance, __SpriteVectorPathCreateInformationInterop* createInformation)
	{
		SpriteVectorPathCreateInformation tmpcreateInformation;
		tmpcreateInformation = ScriptSpriteVectorPathCreateInformation::FromInterop(*createInformation);
		TResourceHandle<SpriteVectorPath> instance = SpriteVectorPath::Create(tmpcreateInformation);
		ScriptResourceManager::Instance().CreateBuiltinScriptResource(instance, managedInstance);
	}
}
