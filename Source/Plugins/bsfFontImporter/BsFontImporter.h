//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsFontPrerequisites.h"
#include "Importer/BsSpecificImporter.h"
#include "Importer/BsImporter.h"

namespace bs
{
	/** @addtogroup Font
	 *  @{
	 */

	/** Importer implementation that handles font import by using the FreeType library. */
	class FontImporter : public SpecificImporter
	{
	public:
		FontImporter();
		virtual ~FontImporter() = default;

		/** @copydoc SpecificImporter::isExtensionSupported */
		bool isExtensionSupported(const String& ext) const override;

		/** @copydoc SpecificImporter::isMagicNumberSupported */
		bool isMagicNumberSupported(const UINT8* magicNumPtr, UINT32 numBytes) const override;

		/** @copydoc SpecificImporter::import */
		SPtr<Resource> import(const Path& filePath, SPtr<const ImportOptions> importOptions) override;

		/** @copydoc SpecificImporter::createImportOptions */
		SPtr<ImportOptions> createImportOptions() const override;
	private:
		Vector<String> mExtensions;

		const static int MAXIMUM_TEXTURE_SIZE = 2048;
	};

	/** @} */
}