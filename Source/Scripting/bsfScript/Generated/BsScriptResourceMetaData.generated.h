//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptReflectableWrapper.h"
#include "../../../Foundation/bsfCore/Resources/BsResourceMetaData.h"

namespace bs { class ResourceMetaData; }
namespace bs
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

		ScriptResourceMetaData(const SPtr<ResourceMetaData>& nativeObject);
		~ScriptResourceMetaData();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
	};
}
