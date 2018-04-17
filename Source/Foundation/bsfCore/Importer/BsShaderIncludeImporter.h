//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Importer/BsSpecificImporter.h"

namespace bs
{
	/** @addtogroup Importer
	 *  @{
	 */

	/**
	 * Importer using for importing GPU program (shader) include files. Include files are just text files ending with 
	 * ".bslinc" extension.
	 */
	class BS_CORE_EXPORT ShaderIncludeImporter : public SpecificImporter
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