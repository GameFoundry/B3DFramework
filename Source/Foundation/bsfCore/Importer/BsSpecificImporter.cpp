//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Importer/BsSpecificImporter.h"
#include "Importer/BsImportOptions.h"
#include "Resources/BsResources.h"

namespace bs
{
	Vector<SubResourceRaw> SpecificImporter::importAll(const Path& filePath, SPtr<const ImportOptions> importOptions)
	{
		SPtr<Resource> resource = import(filePath, importOptions);
		if (resource == nullptr)
			return Vector<SubResourceRaw>();

		return { { u8"primary", resource } };;
	}

	SPtr<ImportOptions> SpecificImporter::createImportOptions() const
	{
		return bs_shared_ptr_new<ImportOptions>();
	}

	SPtr<const ImportOptions> SpecificImporter::getDefaultImportOptions() const
	{
		if(mDefaultImportOptions == nullptr)
			mDefaultImportOptions = createImportOptions();

		return mDefaultImportOptions;
	}
}