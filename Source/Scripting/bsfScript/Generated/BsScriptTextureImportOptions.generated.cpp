//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptTextureImportOptions.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptTextureImportOptions.generated.h"

namespace bs
{
#if !BS_IS_BANSHEE3D
	ScriptTextureImportOptions::ScriptTextureImportOptions(MonoObject* managedInstance, const SPtr<TextureImportOptions>& value)
		:TScriptReflectable(managedInstance, value)
	{
		mInternal = value;
	}

	void ScriptTextureImportOptions::InitRuntimeData()
	{
		metaData.scriptClass->AddInternalCall("Internal_getformat", (void*)&ScriptTextureImportOptions::Internal_getformat);
		metaData.scriptClass->AddInternalCall("Internal_setformat", (void*)&ScriptTextureImportOptions::Internal_setformat);
		metaData.scriptClass->AddInternalCall("Internal_getgenerateMips", (void*)&ScriptTextureImportOptions::Internal_getgenerateMips);
		metaData.scriptClass->AddInternalCall("Internal_setgenerateMips", (void*)&ScriptTextureImportOptions::Internal_setgenerateMips);
		metaData.scriptClass->AddInternalCall("Internal_getmaxMip", (void*)&ScriptTextureImportOptions::Internal_getmaxMip);
		metaData.scriptClass->AddInternalCall("Internal_setmaxMip", (void*)&ScriptTextureImportOptions::Internal_setmaxMip);
		metaData.scriptClass->AddInternalCall("Internal_getcpuCached", (void*)&ScriptTextureImportOptions::Internal_getcpuCached);
		metaData.scriptClass->AddInternalCall("Internal_setcpuCached", (void*)&ScriptTextureImportOptions::Internal_setcpuCached);
		metaData.scriptClass->AddInternalCall("Internal_getsRGB", (void*)&ScriptTextureImportOptions::Internal_getsRGB);
		metaData.scriptClass->AddInternalCall("Internal_setsRGB", (void*)&ScriptTextureImportOptions::Internal_setsRGB);
		metaData.scriptClass->AddInternalCall("Internal_getcubemap", (void*)&ScriptTextureImportOptions::Internal_getcubemap);
		metaData.scriptClass->AddInternalCall("Internal_setcubemap", (void*)&ScriptTextureImportOptions::Internal_setcubemap);
		metaData.scriptClass->AddInternalCall("Internal_getcubemapSourceType", (void*)&ScriptTextureImportOptions::Internal_getcubemapSourceType);
		metaData.scriptClass->AddInternalCall("Internal_setcubemapSourceType", (void*)&ScriptTextureImportOptions::Internal_setcubemapSourceType);
		metaData.scriptClass->AddInternalCall("Internal_create", (void*)&ScriptTextureImportOptions::Internal_create);

	}

	MonoObject* ScriptTextureImportOptions::create(const SPtr<TextureImportOptions>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.scriptClass->CreateInstance("bool", ctorParams);
		new (bs_alloc<ScriptTextureImportOptions>()) ScriptTextureImportOptions(managedInstance, value);
		return managedInstance;
	}
	void ScriptTextureImportOptions::Internal_create(MonoObject* managedInstance)
	{
		SPtr<TextureImportOptions> instance = TextureImportOptions::create();
		new (bs_alloc<ScriptTextureImportOptions>())ScriptTextureImportOptions(managedInstance, instance);
	}
	PixelFormat ScriptTextureImportOptions::Internal_getformat(ScriptTextureImportOptions* thisPtr)
	{
		PixelFormat tmp__output;
		tmp__output = thisPtr->GetInternal()->format;

		PixelFormat __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptTextureImportOptions::Internal_setformat(ScriptTextureImportOptions* thisPtr, PixelFormat value)
	{
		thisPtr->GetInternal()->format = value;
	}

	bool ScriptTextureImportOptions::Internal_getgenerateMips(ScriptTextureImportOptions* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->generateMips;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptTextureImportOptions::Internal_setgenerateMips(ScriptTextureImportOptions* thisPtr, bool value)
	{
		thisPtr->GetInternal()->generateMips = value;
	}

	uint32_t ScriptTextureImportOptions::Internal_getmaxMip(ScriptTextureImportOptions* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->GetInternal()->maxMip;

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptTextureImportOptions::Internal_setmaxMip(ScriptTextureImportOptions* thisPtr, uint32_t value)
	{
		thisPtr->GetInternal()->maxMip = value;
	}

	bool ScriptTextureImportOptions::Internal_getcpuCached(ScriptTextureImportOptions* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->cpuCached;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptTextureImportOptions::Internal_setcpuCached(ScriptTextureImportOptions* thisPtr, bool value)
	{
		thisPtr->GetInternal()->cpuCached = value;
	}

	bool ScriptTextureImportOptions::Internal_getsRGB(ScriptTextureImportOptions* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->sRGB;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptTextureImportOptions::Internal_setsRGB(ScriptTextureImportOptions* thisPtr, bool value)
	{
		thisPtr->GetInternal()->sRGB = value;
	}

	bool ScriptTextureImportOptions::Internal_getcubemap(ScriptTextureImportOptions* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->cubemap;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptTextureImportOptions::Internal_setcubemap(ScriptTextureImportOptions* thisPtr, bool value)
	{
		thisPtr->GetInternal()->cubemap = value;
	}

	CubemapSourceType ScriptTextureImportOptions::Internal_getcubemapSourceType(ScriptTextureImportOptions* thisPtr)
	{
		CubemapSourceType tmp__output;
		tmp__output = thisPtr->GetInternal()->cubemapSourceType;

		CubemapSourceType __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptTextureImportOptions::Internal_setcubemapSourceType(ScriptTextureImportOptions* thisPtr, CubemapSourceType value)
	{
		thisPtr->GetInternal()->cubemapSourceType = value;
	}
#endif
}
