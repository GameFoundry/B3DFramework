//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptImporter.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Engine/Core/Importer/B3DImporter.h"
#include "B3DScriptResourceManager.h"
#include "Wrappers/B3DScriptRRefBase.h"
#include "Wrappers/B3DScriptAsyncOp.h"
#include "Reflection/B3DRTTIType.h"
#include "Resources/B3DResource.h"
#include "B3DScriptResourceWrapper.h"
#include "B3DScriptMultiResource.generated.h"
#include "B3DScriptResourceWrapper.h"
#include "B3DScriptImportOptions.generated.h"

namespace b3d
{
#if !B3D_IS_ENGINE
	ScriptImporter::ScriptImporter()
		:TScriptTypeDefinition()
	{
	}

	void ScriptImporter::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Import", (void*)&ScriptImporter::InternalImport);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_ImportAsync", (void*)&ScriptImporter::InternalImportAsync);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_ImportAll", (void*)&ScriptImporter::InternalImportAll);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_ImportAllAsync", (void*)&ScriptImporter::InternalImportAllAsync);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SupportsFileType", (void*)&ScriptImporter::InternalSupportsFileType);

	}

	MonoObject* ScriptImporter::InternalImport(MonoString* inputFilePath, MonoObject* importOptions, UUID* UUID)
	{
		TResourceHandle<Resource> tmp__output;
		Path tmpinputFilePath;
		tmpinputFilePath = MonoUtil::MonoToString(inputFilePath);
		TShared<ImportOptions> tmpimportOptions;
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
		TShared<ImportOptions> tmpimportOptions;
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
		TShared<MultiResource> tmp__output;
		Path tmpinputFilePath;
		tmpinputFilePath = MonoUtil::MonoToString(inputFilePath);
		TShared<ImportOptions> tmpimportOptions;
		ScriptImportOptionsWrapperBase* scriptObjectWrapperimportOptions;
		scriptObjectWrapperimportOptions = (ScriptImportOptionsWrapperBase*)ScriptImportOptions::GetScriptObjectWrapper(importOptions);
		if(scriptObjectWrapperimportOptions != nullptr)
			tmpimportOptions = std::static_pointer_cast<ImportOptions>(scriptObjectWrapperimportOptions->GetBaseNativeObjectAsShared());
		tmp__output = Importer::Instance().ImportAll(tmpinputFilePath, tmpimportOptions);

		MonoObject* __output;
		__output = ScriptMultiResource::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	MonoObject* ScriptImporter::InternalImportAllAsync(MonoString* inputFilePath, MonoObject* importOptions)
	{
		TAsyncOp<TShared<MultiResource>> tmp__output;
		Path tmpinputFilePath;
		tmpinputFilePath = MonoUtil::MonoToString(inputFilePath);
		TShared<ImportOptions> tmpimportOptions;
		ScriptImportOptionsWrapperBase* scriptObjectWrapperimportOptions;
		scriptObjectWrapperimportOptions = (ScriptImportOptionsWrapperBase*)ScriptImportOptions::GetScriptObjectWrapper(importOptions);
		if(scriptObjectWrapperimportOptions != nullptr)
			tmpimportOptions = std::static_pointer_cast<ImportOptions>(scriptObjectWrapperimportOptions->GetBaseNativeObjectAsShared());
		tmp__output = Importer::Instance().ImportAllAsync(tmpinputFilePath, tmpimportOptions);

		MonoObject* __output;
		auto fnConvertCallback = [](const Any& returnValue)
		{
			TShared<MultiResource> nativeObject = AnyCast<TShared<MultiResource>>(returnValue);
			MonoObject* scriptObject;
			scriptObject = ScriptMultiResource::GetOrCreateScriptObject(nativeObject);
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
