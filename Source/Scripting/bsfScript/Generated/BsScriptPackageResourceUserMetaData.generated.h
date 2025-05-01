//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptReflectableWrapper.h"
#include "../../../Foundation/bsfCore/Resources/BsPackage.h"

namespace bs { class PackageResourceUserMetaData; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptPackageResourceUserMetaDataWrapperBase : public ScriptReflectableWrapper
	{
	public:
		using ScriptReflectableWrapper::ScriptReflectableWrapper;

	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptPackageResourceUserMetaData : public TScriptReflectableWrapper<PackageResourceUserMetaData, ScriptPackageResourceUserMetaData, ScriptPackageResourceUserMetaDataWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "PackageResourceUserMetaData")

		ScriptPackageResourceUserMetaData(const SPtr<PackageResourceUserMetaData>& nativeObject);
		~ScriptPackageResourceUserMetaData();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
	};
}
