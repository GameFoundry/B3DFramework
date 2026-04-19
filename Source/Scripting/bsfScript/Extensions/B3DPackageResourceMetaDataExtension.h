//********************************* B3D Framework - Copyright 2026 Marko Pintera *****************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "Resources/B3DPackage.h"

namespace b3d
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */
	/** @cond SCRIPT_EXTENSIONS */

	/** Extension class for PackageResourceMetaData, for adding additional functionality for the script version of the class. */
	class B3D_SCRIPT_EXPORT(ExtensionClassForType(PackageResourceMetaData)) PackageResourceMetaDataExtension
	{
	public:
		/** Returns managed type of the resource described by the meta-data. */
		B3D_SCRIPT_EXPORT(ExtensionMethodForType(PackageResourceMetaData), Property(Getter), ExportName(ResourceType))
		static MonoReflectionType* GetResourceType(const SPtr<PackageResourceMetaData>& self);
	};

	/** @endcond */
	/** @} */
} // namespace b3d
