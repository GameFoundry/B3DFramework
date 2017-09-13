//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Resources/BsResourceManifest.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class BS_CORE_EXPORT ResourceManifestRTTI : public RTTIType<ResourceManifest, IReflectable, ResourceManifestRTTI>
	{
	private:
		String& getName(ResourceManifest* obj) { return obj->mName; }
		void setName(ResourceManifest* obj, String& val) { obj->mName = val; }

		UnorderedMap<UUID, Path>& getUUIDMap(ResourceManifest* obj) { return obj->mUUIDToFilePath; }

		void setUUIDMap(ResourceManifest* obj, UnorderedMap<UUID, Path>& val)
		{ 
			obj->mUUIDToFilePath = val; 

			obj->mFilePathToUUID.clear();

			for(auto& entry : obj->mUUIDToFilePath)
			{
				obj->mFilePathToUUID[entry.second] = entry.first;
			}
		} 
	public:
		ResourceManifestRTTI()
		{
			addPlainField("mName", 0, &ResourceManifestRTTI::getName, &ResourceManifestRTTI::setName);
			addPlainField("mUUIDToFilePath", 1, &ResourceManifestRTTI::getUUIDMap, &ResourceManifestRTTI::setUUIDMap);
		}

		const String& getRTTIName() override
		{
			static String name = "ResourceManifest";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_ResourceManifest;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return ResourceManifest::createEmpty();
		}
	};

	/** @} */
	/** @endcond */
}