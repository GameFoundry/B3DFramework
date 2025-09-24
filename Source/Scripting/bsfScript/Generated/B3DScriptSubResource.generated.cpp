//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptSubResource.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "Resources/B3DResource.h"
#include "B3DScriptResourceWrapper.h"

namespace b3d
{
#if !B3D_IS_ENGINE
	ScriptSubResource::ScriptSubResource()
	{ }

	MonoObject* ScriptSubResource::Box(const __SubResourceInterop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
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
		TResourceHandle<Resource> tmpValue;
		ScriptResource* scriptObjectWrapperValue;
		scriptObjectWrapperValue = ScriptResource::GetScriptObjectWrapper(value.Value);
		if(scriptObjectWrapperValue != nullptr)
			tmpValue = B3DStaticResourceCast<Resource>(scriptObjectWrapperValue->GetBaseNativeObjectAsHandle());
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
		MonoObject* temptmpValue = nullptr;
		if(value.Value)
		temptmpValue = ScriptResourceWrapper::GetOrCreateScriptObject(value.Value);
		tmpValue = temptmpValue;
		output.Value = tmpValue;

		return output;
	}

#endif
}
