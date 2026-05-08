//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptVectorGraphicsSettings.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Engine/Core/VectorGraphics/B3DVectorGraphics.h"

namespace b3d
{
	ScriptVectorGraphicsSettings::ScriptVectorGraphicsSettings(const TShared<VectorGraphicsSettings>& nativeObject)
		:TScriptNonReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptVectorGraphicsSettings::~ScriptVectorGraphicsSettings()
	{
		UnregisterEvents();
	}

	void ScriptVectorGraphicsSettings::SetupScriptBindings()
	{

	}

	MonoObject* ScriptVectorGraphicsSettings::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
}
