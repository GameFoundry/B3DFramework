//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DFontPrerequisites.h"
#include "Importer/B3DSpecificImporter.h"
#include "Importer/B3DImporter.h"

namespace b3d
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

		bool IsExtensionSupported(const String& ext) const override;
		bool IsMagicNumberSupported(const u8* magicNumPtr, u32 numBytes) const override;
		TShared<Resource> Import(const Path& filePath, TShared<const ImportOptions> importOptions) override;
		TShared<ImportOptions> CreateImportOptions() const override;

	private:
		Vector<String> mExtensions;

		const static int kMaximumTextureSize = 2048;
	};

	/** @} */
} // namespace b3d
