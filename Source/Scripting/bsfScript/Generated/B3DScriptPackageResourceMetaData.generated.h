//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptReflectableWrapper.h"
#include "../../../Foundation/bsfCore/Resources/BsPackage.h"
#include "Utility/BsUUID.h"
#include "../../../Foundation/bsfUtility/Utility/BsCompression.h"
#include "../../../Foundation/bsfCore/Resources/BsPackage.h"

namespace b3d { class PackageResourceMetaDataExtension; }
namespace b3d { class PackageResourceMetaData; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptPackageResourceMetaData : public TScriptReflectableWrapper<PackageResourceMetaData, ScriptPackageResourceMetaData>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "PackageResourceMetaData")

		ScriptPackageResourceMetaData(const SPtr<PackageResourceMetaData>& nativeObject);
		~ScriptPackageResourceMetaData();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static MonoString* InternalGetResourceName(ScriptPackageResourceMetaData* self);
		static MonoString* InternalGetPath(ScriptPackageResourceMetaData* self);
		static void InternalSetPath(ScriptPackageResourceMetaData* self, MonoString* value);
		static void InternalGetId(ScriptPackageResourceMetaData* self, UUID* __output);
		static void InternalSetId(ScriptPackageResourceMetaData* self, UUID* value);
		static uint32_t InternalGetTypeId(ScriptPackageResourceMetaData* self);
		static void InternalSetTypeId(ScriptPackageResourceMetaData* self, uint32_t value);
		static MonoArray* InternalGetDependencies(ScriptPackageResourceMetaData* self);
		static void InternalSetDependencies(ScriptPackageResourceMetaData* self, MonoArray* value);
		static CompressionType InternalGetCompressionType(ScriptPackageResourceMetaData* self);
		static void InternalSetCompressionType(ScriptPackageResourceMetaData* self, CompressionType value);
		static PackageResourceFlag InternalGetFlags(ScriptPackageResourceMetaData* self);
		static void InternalSetFlags(ScriptPackageResourceMetaData* self, PackageResourceFlag value);
		static MonoObject* InternalGetAdditionalMetaData(ScriptPackageResourceMetaData* self);
		static void InternalSetAdditionalMetaData(ScriptPackageResourceMetaData* self, MonoObject* value);
		static MonoObject* InternalGetResourceMetaData(ScriptPackageResourceMetaData* self);
		static void InternalSetResourceMetaData(ScriptPackageResourceMetaData* self, MonoObject* value);
		static MonoReflectionType* InternalGetResourceType(ScriptPackageResourceMetaData* self);
	};
}
