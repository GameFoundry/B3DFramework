//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsSpecificImporter.h"

namespace BansheeEngine
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
		ShaderIncludeImporter();
		virtual ~ShaderIncludeImporter();

		/** @copydoc SpecificImporter::isExtensionSupported */
		bool isExtensionSupported(const WString& ext) const override;

		/** @copydoc SpecificImporter::isMagicNumberSupported */
		bool isMagicNumberSupported(const UINT8* magicNumPtr, UINT32 numBytes) const override;

		/** @copydoc SpecificImporter::import */
		SPtr<Resource> import(const Path& filePath, SPtr<const ImportOptions> importOptions) override;
	};

	/** @} */
}