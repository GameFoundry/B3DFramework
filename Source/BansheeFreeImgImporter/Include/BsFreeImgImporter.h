//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsFreeImgPrerequisites.h"
#include "BsSpecificImporter.h"
#include "BsImporter.h"

namespace bs
{
	/** @addtogroup FreeImg
	 *  @{
	 */

	/** Importer implementation that handles various import for various image formats using the FreeImg library. */
	class FreeImgImporter : public SpecificImporter
	{
		struct RawImageData;

	public:
		FreeImgImporter();
		virtual ~FreeImgImporter();

		/** @copydoc SpecificImporter::isExtensionSupported */
		virtual bool isExtensionSupported(const WString& ext) const override;

		/** @copydoc SpecificImporter::isMagicNumberSupported */
		virtual bool isMagicNumberSupported(const UINT8* magicNumPtr, UINT32 numBytes) const override;

		/** @copydoc SpecificImporter::import */
		virtual SPtr<Resource> import(const Path& filePath, SPtr<const ImportOptions> importOptions) override;

		/** @copydoc SpecificImporter::createImportOptions */
		virtual SPtr<ImportOptions> createImportOptions() const override;
	private:
		/**	Converts a magic number into an extension name. */
		WString magicNumToExtension(const UINT8* magic, UINT32 maxBytes) const;

		/**	Imports an image from the provided data stream. */
		SPtr<PixelData> importRawImage(SPtr<DataStream> fileData);

		Vector<WString> mExtensions;
		UnorderedMap<WString, int> mExtensionToFID;
	};

	/** @} */
}