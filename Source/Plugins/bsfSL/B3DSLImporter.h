//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DSLPrerequisites.h"
#include "Importer/B3DSpecificImporter.h"

namespace b3d
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

		bool IsExtensionSupported(const String& ext) const override;
		bool IsMagicNumberSupported(const u8* magicNumPtr, u32 numBytes) const override;
		ImporterAsyncMode GetAsyncMode() const override { return ImporterAsyncMode::Single; }
		TShared<Resource> Import(const Path& filePath, TShared<const ImportOptions> importOptions) override;
		TShared<ImportOptions> CreateImportOptions() const override;

		static inline constexpr const char* kShaderExtensionWithoutLeadingDot = "bsl";
	};

	/** @} */
} // namespace b3d
