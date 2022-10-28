//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "Importer/BsSpecificImporter.h"

namespace bs
{
	/** @addtogroup Resources-Engine-Internal
	 *  @{
	 */

	/**	Imports C# script source code files (.cs). */
	class BS_EXPORT ScriptCodeImporter : public SpecificImporter
	{
	public:
		bool IsExtensionSupported(const String& ext) const override;
		bool IsMagicNumberSupported(const u8* magicNumPtr, u32 numBytes) const override;
		SPtr<Resource> Import(const Path& filePath, SPtr<const ImportOptions> importOptions) override;
		SPtr<ImportOptions> CreateImportOptions() const override;
	};

	/** @} */
} // namespace bs
