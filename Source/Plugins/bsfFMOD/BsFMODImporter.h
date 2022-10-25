//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsFMODPrerequisites.h"
#include "Importer/BsSpecificImporter.h"

namespace bs
{
	/** @addtogroup FMOD
	 *  @{
	 */

	/** Importer using for importing WAV/FLAC/OGGVORBIS audio files. */
	class FMODImporter : public SpecificImporter
	{
	public:
		FMODImporter();
		virtual ~FMODImporter() = default;

		/** @copydoc SpecificImporter::isExtensionSupported */
		bool IsExtensionSupported(const String& ext) const override;

		/** @copydoc SpecificImporter::isMagicNumberSupported */
		bool IsMagicNumberSupported(const u8* magicNumPtr, u32 numBytes) const override;

		/** @copydoc SpecificImporter::import */
		SPtr<Resource> Import(const Path& filePath, SPtr<const ImportOptions> importOptions);

		/** @copydoc SpecificImporter::createImportOptions */
		SPtr<ImportOptions> CreateImportOptions() const;
	};

	/** @} */
} // namespace bs
