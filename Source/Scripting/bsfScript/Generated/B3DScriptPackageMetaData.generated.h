//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptReflectableWrapper.h"
#include "../../../Foundation/bsfCore/Resources/BsPackage.h"

namespace b3d { class PackageMetaData; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptPackageMetaDataWrapperBase : public ScriptReflectableWrapper
	{
	public:
		using ScriptReflectableWrapper::ScriptReflectableWrapper;

	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptPackageMetaData : public TScriptReflectableWrapper<PackageMetaData, ScriptPackageMetaData, ScriptPackageMetaDataWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "PackageMetaData")

		ScriptPackageMetaData(const SPtr<PackageMetaData>& nativeObject);
		~ScriptPackageMetaData();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
	};
}
