//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Importer/B3DSpecificImporter.h"

namespace b3d
{
	/** @addtogroup Importer
	 *  @{
	 */

	/**
	 * Importer using for importing GPU program (shader) include files. Include files are just text files ending with
	 * ".bslinc" extension.
	 */
	class B3D_EXPORT ShaderIncludeImporter : public SpecificImporter
	{
	public:
		/** @copydoc SpecificImporter::IsExtensionSupported */
		bool IsExtensionSupported(const String& extension) const;

		/** @copydoc SpecificImporter::IsMagicNumberSupported */
		bool IsMagicNumberSupported(const u8* magicNumber, u32 magicNumberSize) const;

		/** @copydoc SpecificImporter::Import */
		SPtr<Resource> Import(const Path& filePath, SPtr<const ImportOptions> importOptions);
	};

	/** @} */
} // namespace b3d
