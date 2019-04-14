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

	void ScriptSubResource::initRuntimeData()
	{ }

	MonoObject*ScriptSubResource::box(const __SubResourceInterop& value)
	{
		return MonoUtil::box(metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	__SubResourceInterop ScriptSubResource::unbox(MonoObject* value)
	{
		return *(__SubResourceInterop*)MonoUtil::unbox(value);
	}

	SubResource ScriptSubResource::fromInterop(const __SubResourceInterop& value)
	{
		SubResource output;
		String tmpname;
		tmpname = MonoUtil::monoToString(value.name);
		output.name = tmpname;
		ResourceHandle<Resource> tmpvalue;
		ScriptResource* scriptvalue;
		scriptvalue = ScriptResource::toNative(value.value);
		if(scriptvalue != nullptr)
			tmpvalue = static_resource_cast<Resource>(scriptvalue->getGenericHandle());
		output.value = tmpvalue;

		return output;
	}

	__SubResourceInterop ScriptSubResource::toInterop(const SubResource& value)
	{
		__SubResourceInterop output;
		MonoString* tmpname;
		tmpname = MonoUtil::stringToMono(value.name);
		output.name = tmpname;
		ScriptResourceBase* scriptvalue;
		scriptvalue = ScriptResourceManager::instance().getScriptResource(value.value, true);
		MonoObject* tmpvalue;
		if(scriptvalue != nullptr)
			tmpvalue = scriptvalue->getManagedInstance();
		else
			tmpvalue = nullptr;
		output.value = tmpvalue;

		return output;
	}

#endif
}
