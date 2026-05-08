//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptReflectableWrapper.h"
#include "../../../Engine/Core/Resources/B3DResourceMetaData.h"

namespace b3d { class ResourceMetaData; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptResourceMetaDataWrapperBase : public ScriptReflectableWrapper
	{
	public:
		using ScriptReflectableWrapper::ScriptReflectableWrapper;

	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptResourceMetaData : public TScriptReflectableWrapper<ResourceMetaData, ScriptResourceMetaData, ScriptResourceMetaDataWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ResourceMetaData")

		ScriptResourceMetaData(const TShared<ResourceMetaData>& nativeObject);
		~ScriptResourceMetaData();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
	};
}
