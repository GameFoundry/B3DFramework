//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsSLPrerequisites.h"
#include "Importer/BsSpecificImporter.h"

namespace bs
{
	/** @addtogroup BansheeSL
	 *  @{
	 */

	/** 
	 * Importer using for importing a shader written using the BSL syntax. Shader files are plain text files ending with
	 * ".bsl" extension.
	 */
	class SLImporter : public SpecificImporter
	{
	public:
		SLImporter();
		virtual ~SLImporter();

		/** @copydoc SpecificImporter::isExtensionSupported */
		bool isExtensionSupported(const WString& ext) const override;

		/** @copydoc SpecificImporter::isMagicNumberSupported */
		bool isMagicNumberSupported(const UINT8* magicNumPtr, UINT32 numBytes) const override;

		/** @copydoc SpecificImporter::getAsyncMode */
		ImporterAsyncMode getAsyncMode() const override { return ImporterAsyncMode::Single; }

		/** @copydoc SpecificImporter::import */
		SPtr<Resource> import(const Path& filePath, SPtr<const ImportOptions> importOptions) override;

		/** @copydoc SpecificImporter::createImportOptions */
		SPtr<ImportOptions> createImportOptions() const override;
	};

	/** @} */
}