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
		bool IsExtensionSupported(const String& ext) const;

		/** @copydoc SpecificImporter::isMagicNumberSupported */
		bool IsMagicNumberSupported(const u8* magicNumPtr, u32 numBytes) const;

		/** @copydoc SpecificImporter::import */
		SPtr<Resource> Import(const Path& filePath, SPtr<const ImportOptions> importOptions);

		/** @copydoc SpecificImporter::createImportOptions */
		SPtr<ImportOptions> CreateImportOptions() const;

	private:
		Vector<String> mExtensions;

		const static int kMaximumTextureSize = 2048;
	};

	/** @} */
} // namespace bs
