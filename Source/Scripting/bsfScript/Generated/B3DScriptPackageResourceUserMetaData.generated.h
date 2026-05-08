//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptReflectableWrapper.h"
#include "../../../Engine/Core/Resources/B3DPackage.h"

namespace b3d { class PackageResourceUserMetaData; }
namespace b3d
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

		ScriptPackageResourceUserMetaData(const TShared<PackageResourceUserMetaData>& nativeObject);
		~ScriptPackageResourceUserMetaData();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
	};
}
