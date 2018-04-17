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

	/**	Imports plain text files (.txt, .xml, .json). */
	class BS_EXPORT PlainTextImporter : public SpecificImporter
	{
	public:
		/** @copydoc SpecificImporter::isExtensionSupported */
		bool isExtensionSupported(const String& ext) const override;

		/** @copydoc SpecificImporter::isMagicNumberSupported */
		bool isMagicNumberSupported(const UINT8* magicNumPtr, UINT32 numBytes) const override;

		/** @copydoc SpecificImporter::import */
		SPtr<Resource> import(const Path& filePath, SPtr<const ImportOptions> importOptions) override;
	};

	/** @} */
}