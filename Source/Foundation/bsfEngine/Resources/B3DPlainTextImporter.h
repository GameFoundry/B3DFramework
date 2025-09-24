//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "Importer/BsSpecificImporter.h"

namespace b3d
{
	/** @addtogroup Resources-Engine-Internal
	 *  @{
	 */

	/**	Imports plain text files (.txt, .xml, .json). */
	class B3D_EXPORT PlainTextImporter : public SpecificImporter
	{
	public:
		bool IsExtensionSupported(const String& ext) const override;
		bool IsMagicNumberSupported(const u8* magicNumPtr, u32 numBytes) const override;
		SPtr<Resource> Import(const Path& filePath, SPtr<const ImportOptions> importOptions) override;
	};

	/** @} */
} // namespace b3d
