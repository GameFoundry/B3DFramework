//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Utility/BsDynLibManager.h"
#include "Utility/BsDynLib.h"

namespace bs
{
	static bool operator<(const UPtr<DynLib>& lhs, const String& rhs)
	{
		return lhs->getName() < rhs;
	}

	static bool operator<(const String& lhs, const UPtr<DynLib>& rhs)
	{
		return lhs < rhs->getName();
	}

	static bool operator<(const UPtr<DynLib>& lhs, const UPtr<DynLib>& rhs)
	{
		return lhs->getName() < rhs->getName();
	}

	DynLib* DynLibManager::load(String filename)
	{
		// Add the extension (.dll, .so, ...) if necessary.

		// Note: The string comparison here could be slightly more efficent by using a templatized string_concat function
		// for the lower_bound call and/or a custom comparitor that does comparison by parts.
		const String::size_type length = filename.length();
		const String extension = String(".") + DynLib::EXTENSION;
		const String::size_type extLength = extension.length();
		if(length <= extLength || filename.substr(length - extLength) != extension)
			filename.append(extension);

		if(DynLib::PREFIX != nullptr)
			filename.insert(0, DynLib::PREFIX);

		const auto& iterFind = mLoadedLibraries.lower_bound(filename);
		if(iterFind != mLoadedLibraries.end() && (*iterFind)->getName() == filename)
		{
			return iterFind->get();
		}
		else
		{
			DynLib* newLib = bs_new<DynLib>(std::move(filename));
			mLoadedLibraries.emplace_hint(iterFind, newLib);

			return newLib;
		}
	}

	void DynLibManager::unload(DynLib* lib)
	{
		const auto& iterFind = mLoadedLibraries.find(lib->getName());
		if(iterFind != mLoadedLibraries.end())
			mLoadedLibraries.erase(iterFind);
		else
			bs_delete(lib);
	}

	DynLibManager& gDynLibManager()
	{
		return DynLibManager::instance();
	}
}
