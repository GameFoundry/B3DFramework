//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptReflectableWrapper.h"
#include "../../../Engine/Core/Resources/B3DPackage.h"

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

		ScriptPackageMetaData(const TShared<PackageMetaData>& nativeObject);
		~ScriptPackageMetaData();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
	};
}
