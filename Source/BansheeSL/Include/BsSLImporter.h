//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsSLPrerequisites.h"
#include "BsSpecificImporter.h"

namespace bs
{
	/** @addtogroup BansheeSL
	 *  @{
	 */

	/** 
	 * Importer using for importing a shader written using the BSL syntax. Shader files are plain text files ending with
	 * ".bsl" extension.
	 */
	class SLImporter : public SpecificImporter
	{
	public:
		SLImporter();
		virtual ~SLImporter();

		/** @copydoc SpecificImporter::isExtensionSupported */
		bool isExtensionSupported(const WString& ext) const override;

		/** @copydoc SpecificImporter::isMagicNumberSupported */
		bool isMagicNumberSupported(const UINT8* magicNumPtr, UINT32 numBytes) const override;

		/** @copydoc SpecificImporter::import */
		SPtr<Resource> import(const Path& filePath, SPtr<const ImportOptions> importOptions) override;

		/** @copydoc SpecificImporter::createImportOptions */
		SPtr<ImportOptions> createImportOptions() const override;
	};

	/** @} */
}