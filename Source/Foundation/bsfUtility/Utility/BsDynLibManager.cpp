//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Utility/BsDynLibManager.h"
#include "Utility/BsDynLib.h"

namespace bs
{
	static bool operator<(const UPtr<DynLib>& lhs, const String& rhs)
	{
		return lhs->GetName() < rhs;
	}

	static bool operator<(const String& lhs, const UPtr<DynLib>& rhs)
	{
		return lhs < rhs->GetName();
	}

	static bool operator<(const UPtr<DynLib>& lhs, const UPtr<DynLib>& rhs)
	{
		return lhs->GetName() < rhs->getName();
	}

	DynLib* DynLibManager::load(String filename)
	{
		// Add the extension (.dll, .so, ...) if necessary.

		// Note: The string comparison here could be slightly more efficent by using a templatized string_concat function
		// for the lower_bound call and/or a custom comparitor that does comparison by parts.
		const String::size_type length = filename.Length();
		const String extension = String(".") + DynLib::EXTENSION;
		const String::size_type extLength = extension.Length();
		if(length <= extLength || filename.Substr(length - extLength) != extension)
			filename.Append(extension);

		if(DynLib::PREFIX != nullptr)
			filename.Insert(0, DynLib::PREFIX);

		const auto& iterFind = mLoadedLibraries.lower_bound(filename);
		if(iterFind != mLoadedLibraries.End() && (*iterFind)->GetName() == filename)
		{
			return iterFind->Get();
		}
		else
		{
			DynLib* newLib = bs_new<DynLib>(std::move(filename));
			mLoadedLibraries.emplace_hint(iterFind, newLib);

			return newLib;
		}
	}

	void DynLibManager::Unload(DynLib* lib)
	{
		const auto& iterFind = mLoadedLibraries.Find(lib->GetName());
		if(iterFind != mLoadedLibraries.End())
			mLoadedLibraries.Erase(iterFind);
		else
			bs_delete(lib);
	}

	DynLibManager& GDynLibManager()
	{
		return DynLibManager::Instance();
	}
}
