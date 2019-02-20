//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptImportOptions.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
#if !BS_IS_BANSHEE3D
	ScriptImportOptionsBase::ScriptImportOptionsBase(MonoObject* managedInstance)
		:ScriptReflectableBase(managedInstance)
	 { }

	SPtr<ImportOptions> ScriptImportOptionsBase::getInternal() const
	{
		return std::static_pointer_cast<ImportOptions>(mInternal);
	}
	ScriptImportOptions::ScriptImportOptions(MonoObject* managedInstance, const SPtr<ImportOptions>& value)
		:TScriptReflectable(managedInstance, value)
	{
		mInternal = value;
	}

	void ScriptImportOptions::initRuntimeData()
	{

	}

	MonoObject* ScriptImportOptions::create(const SPtr<ImportOptions>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.scriptClass->createInstance("bool", ctorParams);
		new (bs_alloc<ScriptImportOptions>()) ScriptImportOptions(managedInstance, value);
		return managedInstance;
	}
#endif
}
