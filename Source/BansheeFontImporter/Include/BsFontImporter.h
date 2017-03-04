//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsFontPrerequisites.h"
#include "BsSpecificImporter.h"
#include "BsImporter.h"

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
		virtual ~FontImporter();

		/** @copydoc SpecificImporter::isExtensionSupported */
		bool isExtensionSupported(const WString& ext) const override;

		/** @copydoc SpecificImporter::isMagicNumberSupported */
		bool isMagicNumberSupported(const UINT8* magicNumPtr, UINT32 numBytes) const override;

		/** @copydoc SpecificImporter::import */
		SPtr<Resource> import(const Path& filePath, SPtr<const ImportOptions> importOptions) override;

		/** @copydoc SpecificImporter::createImportOptions */
		SPtr<ImportOptions> createImportOptions() const override;
	private:
		Vector<WString> mExtensions;

		const static int MAXIMUM_TEXTURE_SIZE = 2048;
	};

	/** @} */
}