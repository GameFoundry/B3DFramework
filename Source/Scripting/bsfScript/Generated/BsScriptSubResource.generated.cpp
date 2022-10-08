//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptSubResource.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptResourceManager.h"
#include "Resources/BsResource.h"
#include "Wrappers/BsScriptResource.h"

namespace bs
{
#if !BS_IS_BANSHEE3D
	ScriptSubResource::ScriptSubResource(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptSubResource::InitRuntimeData()
	{ }

	MonoObject*ScriptSubResource::Box(const __SubResourceInterop& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	__SubResourceInterop ScriptSubResource::Unbox(MonoObject* value)
	{
		return *(__SubResourceInterop*)MonoUtil::Unbox(value);
	}

	SubResource ScriptSubResource::FromInterop(const __SubResourceInterop& value)
	{
		SubResource output;
		String tmpName;
		tmpName = MonoUtil::MonoToString(value.Name);
		output.Name = tmpName;
		ResourceHandle<Resource> tmpValue;
		ScriptResource* scriptValue;
		scriptValue = ScriptResource::ToNative(value.Value);
		if(scriptValue != nullptr)
			tmpValue = static_resource_cast<Resource>(scriptValue->GetGenericHandle());
		output.Value = tmpValue;

		return output;
	}

	__SubResourceInterop ScriptSubResource::ToInterop(const SubResource& value)
	{
		__SubResourceInterop output;
		MonoString* tmpName;
		tmpName = MonoUtil::StringToMono(value.Name);
		output.Name = tmpName;
		MonoObject* tmpValue;
		ScriptResourceBase* scriptValue;
		scriptValue = ScriptResourceManager::Instance().GetScriptResource(value.Value, true);
		if(scriptValue != nullptr)
			tmpValue = scriptValue->GetManagedInstance();
		else
			tmpValue = nullptr;
		output.Value = tmpValue;

		return output;
	}

#endif
}
