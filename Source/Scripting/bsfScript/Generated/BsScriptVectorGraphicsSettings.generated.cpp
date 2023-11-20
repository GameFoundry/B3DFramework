//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptVectorGraphicsSettings.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/VectorGraphics/BsVectorGraphics.h"

namespace bs
{
	ScriptVectorGraphicsSettings::ScriptVectorGraphicsSettings(MonoObject* managedInstance, const SPtr<VectorGraphicsSettings>& value)
		:ScriptObject(managedInstance), mInternal(value)
	{
	}

	void ScriptVectorGraphicsSettings::InitRuntimeData()
	{

	}

	MonoObject* ScriptVectorGraphicsSettings::Create(const SPtr<VectorGraphicsSettings>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
		new (B3DAllocate<ScriptVectorGraphicsSettings>()) ScriptVectorGraphicsSettings(managedInstance, value);
		return managedInstance;
	}
}
