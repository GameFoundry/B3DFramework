//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptManagedPropertyInfo.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"

namespace b3d
{
	ScriptManagedPropertyInfo::ScriptManagedPropertyInfo(const TShared<ManagedPropertyInfo>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptManagedPropertyInfo::~ScriptManagedPropertyInfo()
	{
		UnregisterEvents();
	}

	void ScriptManagedPropertyInfo::SetupScriptBindings()
	{

	}

	MonoObject* ScriptManagedPropertyInfo::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
}
