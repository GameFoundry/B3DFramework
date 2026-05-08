//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DOAPrerequisites.h"
#include "Importer/B3DSpecificImporter.h"

namespace b3d
{
	/** @addtogroup OpenAudio
	 *  @{
	 */

	/** Importer using for importing WAV/FLAC/OGGVORBIS audio files. */
	class OAImporter : public SpecificImporter
	{
	public:
		OAImporter();
		virtual ~OAImporter() = default;

		bool IsExtensionSupported(const String& ext) const override;
		bool IsMagicNumberSupported(const u8* magicNumPtr, u32 numBytes) const override;
		TShared<Resource> Import(const Path& filePath, TShared<const ImportOptions> importOptions) override;
		TShared<ImportOptions> CreateImportOptions() const override;
	};

	/** @} */
} // namespace b3d
