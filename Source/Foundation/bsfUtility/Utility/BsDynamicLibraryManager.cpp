//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Utility/BsDynamicLibraryManager.h"
#include "Utility/BsDynamicLibrary.h"

namespace bs
{
static bool operator<(const UPtr<DynamicLibrary>& lhs, const String& rhs)
{
	return lhs->GetName() < rhs;
}

static bool operator<(const String& lhs, const UPtr<DynamicLibrary>& rhs)
{
	return lhs < rhs->GetName();
}

static bool operator<(const UPtr<DynamicLibrary>& lhs, const UPtr<DynamicLibrary>& rhs)
{
	return lhs->GetName() < rhs->GetName();
}
} // namespace bs

using namespace bs;

DynamicLibrary* DynamicLibraryManager::Load(String filename)
{
	// Add the extension (.dll, .so, ...) if necessary.

	// Note: The string comparison here could be slightly more efficent by using a templatized string_concat function
	// for the lower_bound call and/or a custom comparitor that does comparison by parts.
	const String::size_type length = filename.length();
	const String extension = String(".") + DynamicLibrary::kExtension;
	const String::size_type extLength = extension.length();
	if(length <= extLength || filename.substr(length - extLength) != extension)
		filename.append(extension);

	if(DynamicLibrary::kPrefix != nullptr)
		filename.insert(0, DynamicLibrary::kPrefix);

	const auto& iterFind = mLoadedLibraries.lower_bound(filename);
	if(iterFind != mLoadedLibraries.end() && (*iterFind)->GetName() == filename)
	{
		return iterFind->get();
	}
	else
	{
		DynamicLibrary* newLib = bs_new<DynamicLibrary>(std::move(filename));
		mLoadedLibraries.emplace_hint(iterFind, newLib);

		return newLib;
	}
}

void DynamicLibraryManager::Unload(DynamicLibrary* lib)
{
	const auto& iterFind = mLoadedLibraries.find(lib->GetName());
	if(iterFind != mLoadedLibraries.end())
		mLoadedLibraries.erase(iterFind);
	else
		bs_delete(lib);
}

namespace bs
{
DynamicLibraryManager& GetDynamicLibraryManager()
{
	return DynamicLibraryManager::Instance();
}
} // namespace bs
