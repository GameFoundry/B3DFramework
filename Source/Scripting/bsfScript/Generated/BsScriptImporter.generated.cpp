//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptImporter.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Importer/BsImporter.h"
#include "Wrappers/BsScriptAsyncOp.h"
#include "Reflection/BsRTTIType.h"
#include "../../../Foundation/bsfCore/Text/BsFontImportOptions.h"
#include "BsScriptFontImportOptions.generated.h"
#include "../../../Foundation/bsfCore/Importer/BsShaderImportOptions.h"
#include "BsScriptShaderImportOptions.generated.h"
#include "BsScriptResourceWrapper.h"
#include "BsScriptImportOptions.generated.h"
#include "../../../Foundation/bsfEngine/Resources/BsScriptCodeImportOptions.h"
#include "BsScriptScriptCodeImportOptions.generated.h"
#include "../../../Foundation/bsfCore/Audio/BsAudioClipImportOptions.h"
#include "BsScriptAudioClipImportOptions.generated.h"
#include "../../../Foundation/bsfCore/Importer/BsMeshImportOptions.h"
#include "BsScriptMeshImportOptions.generated.h"
#include "../../../Foundation/bsfCore/Importer/BsTextureImportOptions.h"
#include "BsScriptTextureImportOptions.generated.h"
#include "BsScriptMultiResource.generated.h"

namespace bs
{
#if !B3D_IS_ENGINE
	ScriptImporter::ScriptImporter(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{
	}

	void ScriptImporter::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_Import", (void*)&ScriptImporter::InternalImport);
		metaData.ScriptClass->AddInternalCall("Internal_ImportAsync", (void*)&ScriptImporter::InternalImportAsync);
		metaData.ScriptClass->AddInternalCall("Internal_ImportAll", (void*)&ScriptImporter::InternalImportAll);
		metaData.ScriptClass->AddInternalCall("Internal_ImportAllAsync", (void*)&ScriptImporter::InternalImportAllAsync);
		metaData.ScriptClass->AddInternalCall("Internal_SupportsFileType", (void*)&ScriptImporter::InternalSupportsFileType);

	}

	MonoObject* ScriptImporter::InternalImport(MonoString* inputFilePath, MonoObject* importOptions, UUID* UUID)
	{
		TResourceHandle<Resource> tmp__output;
		Path tmpinputFilePath;
		tmpinputFilePath = MonoUtil::MonoToString(inputFilePath);
		SPtr<ImportOptions> tmpimportOptions;
		ScriptImportOptionsWrapperBase* scriptObjectWrapperimportOptions;
		scriptObjectWrapperimportOptions = (ScriptImportOptionsWrapperBase*)ScriptImportOptions::GetScriptObjectWrapper(importOptions);
		if(scriptObjectWrapperimportOptions != nullptr)
			tmpimportOptions = std::static_pointer_cast<ImportOptions>(scriptObjectWrapperimportOptions->GetBaseNativeObjectAsShared());
		tmp__output = Importer::Instance().Import(tmpinputFilePath, tmpimportOptions, *UUID);

		MonoObject* __output;
		MonoObject* temp__output = nullptr;
		if(tmp__output)
		temp__output = ScriptResourceWrapper::GetOrCreateScriptObject(tmp__output);
		__output = temp__output;

		return __output;
	}

	MonoObject* ScriptImporter::InternalImportAsync(MonoString* inputFilePath, MonoObject* importOptions, UUID* UUID)
	{
		TAsyncOp<TResourceHandle<Resource>> tmp__output;
		Path tmpinputFilePath;
		tmpinputFilePath = MonoUtil::MonoToString(inputFilePath);
		SPtr<ImportOptions> tmpimportOptions;
		ScriptImportOptionsWrapperBase* scriptObjectWrapperimportOptions;
		scriptObjectWrapperimportOptions = (ScriptImportOptionsWrapperBase*)ScriptImportOptions::GetScriptObjectWrapper(importOptions);
		if(scriptObjectWrapperimportOptions != nullptr)
			tmpimportOptions = std::static_pointer_cast<ImportOptions>(scriptObjectWrapperimportOptions->GetBaseNativeObjectAsShared());
		tmp__output = Importer::Instance().ImportAsync(tmpinputFilePath, tmpimportOptions, *UUID);

		MonoObject* __output;
		auto fnConvertCallback = [](const Any& returnValue)
		{
			TResourceHandle<Resource> nativeObject = AnyCast<TResourceHandle<Resource>>(returnValue);
			MonoObject* scriptObject;
			ScriptRRefBase* scriptWrapperObject;
			scriptWrapperObject = ScriptResourceManager::Instance().GetScriptRRef(nativeObject);
			if(scriptWrapperObject != nullptr)
				scriptObject = scriptWrapperObject->GetScriptObject();
			else
				scriptObject = nullptr;
			return scriptObject;
		};

;		__output = ScriptAsyncOpBase::Create(tmp__output, fnConvertCallback, ScriptRRefBase::GetMetaData()->ScriptClass);

		return __output;
	}

	MonoObject* ScriptImporter::InternalImportAll(MonoString* inputFilePath, MonoObject* importOptions)
	{
		SPtr<MultiResource> tmp__output;
		Path tmpinputFilePath;
		tmpinputFilePath = MonoUtil::MonoToString(inputFilePath);
		SPtr<ImportOptions> tmpimportOptions;
		ScriptImportOptionsWrapperBase* scriptObjectWrapperimportOptions;
		scriptObjectWrapperimportOptions = (ScriptImportOptionsWrapperBase*)ScriptImportOptions::GetScriptObjectWrapper(importOptions);
		if(scriptObjectWrapperimportOptions != nullptr)
			tmpimportOptions = std::static_pointer_cast<ImportOptions>(scriptObjectWrapperimportOptions->GetBaseNativeObjectAsShared());
		tmp__output = Importer::Instance().ImportAll(tmpinputFilePath, tmpimportOptions);

		MonoObject* __output;
		__output = ScriptMultiResource::Create(tmp__output);

		return __output;
	}

	MonoObject* ScriptImporter::InternalImportAllAsync(MonoString* inputFilePath, MonoObject* importOptions)
	{
		TAsyncOp<SPtr<MultiResource>> tmp__output;
		Path tmpinputFilePath;
		tmpinputFilePath = MonoUtil::MonoToString(inputFilePath);
		SPtr<ImportOptions> tmpimportOptions;
		ScriptImportOptionsWrapperBase* scriptObjectWrapperimportOptions;
		scriptObjectWrapperimportOptions = (ScriptImportOptionsWrapperBase*)ScriptImportOptions::GetScriptObjectWrapper(importOptions);
		if(scriptObjectWrapperimportOptions != nullptr)
			tmpimportOptions = std::static_pointer_cast<ImportOptions>(scriptObjectWrapperimportOptions->GetBaseNativeObjectAsShared());
		tmp__output = Importer::Instance().ImportAllAsync(tmpinputFilePath, tmpimportOptions);

		MonoObject* __output;
		auto fnConvertCallback = [](const Any& returnValue)
		{
			SPtr<MultiResource> nativeObject = AnyCast<SPtr<MultiResource>>(returnValue);
			MonoObject* scriptObject;
			scriptObject = ScriptMultiResource::Create(nativeObject);
			return scriptObject;
		};

;		__output = ScriptAsyncOpBase::Create(tmp__output, fnConvertCallback, ScriptMultiResource::GetMetaData()->ScriptClass);

		return __output;
	}

	bool ScriptImporter::InternalSupportsFileType(MonoString* extension)
	{
		bool tmp__output;
		String tmpextension;
		tmpextension = MonoUtil::MonoToString(extension);
		tmp__output = Importer::Instance().SupportsFileType(tmpextension);

		bool __output;
		__output = tmp__output;

		return __output;
	}

#endif
}
