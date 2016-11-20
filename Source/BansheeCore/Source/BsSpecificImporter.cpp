//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsSpecificImporter.h"
#include "BsImportOptions.h"
#include "BsResources.h"

namespace bs
{
	Vector<SubResourceRaw> SpecificImporter::importAll(const Path& filePath, SPtr<const ImportOptions> importOptions)
	{
		SPtr<Resource> resource = import(filePath, importOptions);
		if (resource == nullptr)
			return Vector<SubResourceRaw>();

		return { { L"primary", resource } };;
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