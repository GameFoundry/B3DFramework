//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisites.h"
#include "BsSpecificImporter.h"

namespace bs
{
	/** @addtogroup Resources-Engine-Internal
	 *  @{
	 */

	/**	Imports plain text files (.txt, .xml, .json). */
	class BS_EXPORT PlainTextImporter : public SpecificImporter
	{
	public:
		PlainTextImporter();
		virtual ~PlainTextImporter();

		/** @copydoc SpecificImporter::isExtensionSupported */
		virtual bool isExtensionSupported(const WString& ext) const override;

		/** @copydoc SpecificImporter::isMagicNumberSupported */
		virtual bool isMagicNumberSupported(const UINT8* magicNumPtr, UINT32 numBytes) const override;

		/** @copydoc SpecificImporter::import */
		virtual SPtr<Resource> import(const Path& filePath, SPtr<const ImportOptions> importOptions) override;

		static const WString DEFAULT_EXTENSION;
	};

	/** @} */
}