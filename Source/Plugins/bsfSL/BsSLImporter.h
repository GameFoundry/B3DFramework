//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsSLPrerequisites.h"
#include "Importer/BsSpecificImporter.h"

namespace bs
{
	/** @addtogroup bsfSL
	 *  @{
	 */

	/**
	 * Importer using for importing a shader written using the BSL syntax. Shader files are plain text files ending with
	 * ".bsl" extension.
	 */
	class SLImporter : public SpecificImporter
	{
	public:
		SLImporter() = default;
		virtual ~SLImporter() = default;

		/** @copydoc SpecificImporter::isExtensionSupported */
		bool IsExtensionSupported(const String& ext) const;

		/** @copydoc SpecificImporter::isMagicNumberSupported */
		bool IsMagicNumberSupported(const u8* magicNumPtr, u32 numBytes) const;

		/** @copydoc SpecificImporter::getAsyncMode */
		ImporterAsyncMode GetAsyncMode() const { return ImporterAsyncMode::Single; }

		/** @copydoc SpecificImporter::import */
		SPtr<Resource> Import(const Path& filePath, SPtr<const ImportOptions> importOptions);

		/** @copydoc SpecificImporter::createImportOptions */
		SPtr<ImportOptions> CreateImportOptions() const;
	};

	/** @} */
} // namespace bs
