//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Importer/B3DSpecificImporter.h"
#include "Importer/B3DImportOptions.h"
#include "Resources/B3DResources.h"

using namespace b3d;

Vector<SubResourceRaw> SpecificImporter::ImportAll(const Path& filePath, SPtr<const ImportOptions> importOptions)
{
	SPtr<Resource> resource = Import(filePath, importOptions);
	if(resource == nullptr)
		return Vector<SubResourceRaw>();

	return { { SubResourceRaw::kPrimaryResourceName, resource } };
	;
}

SPtr<ImportOptions> SpecificImporter::CreateImportOptions() const
{
	return B3DMakeShared<ImportOptions>();
}

SPtr<const ImportOptions> SpecificImporter::GetDefaultImportOptions() const
{
	if(mDefaultImportOptions == nullptr)
		mDefaultImportOptions = CreateImportOptions();

	return mDefaultImportOptions;
}
