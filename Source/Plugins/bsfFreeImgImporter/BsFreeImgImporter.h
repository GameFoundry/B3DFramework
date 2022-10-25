//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsFreeImgPrerequisites.h"
#include "Importer/BsSpecificImporter.h"
#include "Importer/BsImporter.h"

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
		bool IsExtensionSupported(const String& ext) const;

		/** @copydoc SpecificImporter::isMagicNumberSupported */
		bool IsMagicNumberSupported(const u8* magicNumPtr, u32 numBytes) const;

		/** @copydoc SpecificImporter::import */
		SPtr<Resource> Import(const Path& filePath, SPtr<const ImportOptions> importOptions);

		/** @copydoc SpecificImporter::createImportOptions */
		SPtr<ImportOptions> CreateImportOptions() const;

	private:
		/**	Converts a magic number into an extension name. */
		String MagicNumToExtension(const u8* magic, u32 maxBytes) const;

		/**	Imports an image from the provided data stream. */
		SPtr<PixelData> ImportRawImage(const Path& fileData);

		/**
		 * Generates six cubemap faces from the provided source texture. *
		 *
		 * @param[in]	source		Source texture containing the pixels to generate the cubemap from.
		 * @param[in]	sourceType	Type of the source texture, determines how is the data interpreted.
		 * @param[out]	output		Will contain the six cubemap faces, if the method returns true. The faces will be in the
		 *							same order as presented in the CubemapFace enum.
		 * @return					True if the cubemap faces were successfully generated, false otherwise.
		 */
		bool GenerateCubemap(const SPtr<PixelData>& source, CubemapSourceType sourceType, std::array<SPtr<PixelData>, 6>& output);

		Vector<String> mExtensions;
		UnorderedMap<String, int> mExtensionToFID;
	};

	/** @} */
} // namespace bs
