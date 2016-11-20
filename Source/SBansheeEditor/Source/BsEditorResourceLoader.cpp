//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsEditorResourceLoader.h"
#include "BsProjectLibrary.h"
#include "BsResources.h"
#include "BsProjectResourceMeta.h"
#include "BsDebug.h"

namespace bs
{
	HResource EditorResourceLoader::load(const Path& path, bool keepLoaded) const
	{
		ProjectLibrary::LibraryEntry* entry = gProjectLibrary().findEntry(path);

		if (entry == nullptr || entry->type == ProjectLibrary::LibraryEntryType::Directory)
			return HResource();

		ProjectLibrary::FileEntry* resEntry = static_cast<ProjectLibrary::FileEntry*>(entry);
		if (resEntry->meta == nullptr)
		{
			LOGWRN("Missing .meta file for resource at path: \"" + path.toString() + "\".");
			return HResource();
		}

		// Note: Calling both findEntry and findResourceMeta is a bit redundant since they do a lot of the same work, and 
		// this could be optimized so only one of them is called.
		SPtr<ProjectResourceMeta> meta = gProjectLibrary().findResourceMeta(path);
		if (meta == nullptr)
			LOGWRN("Unable to load resource at path: \"" + path.toString() + "\". File not found. ");

		String resUUID = meta->getUUID();

		if (resEntry->meta->getIncludeInBuild())
		{
			LOGWRN("Dynamically loading a resource at path: \"" + path.toString() + "\" but the resource \
					isn't flagged to be included in the build. It may not be available outside of the editor.");
		}

		ResourceLoadFlags loadFlags = ResourceLoadFlag::LoadDependencies | ResourceLoadFlag::KeepSourceData;
		if (keepLoaded)
			loadFlags |= ResourceLoadFlag::KeepInternalRef;

		return gResources().loadFromUUID(resUUID, false, loadFlags);
	}
}