//************************************ B3D Framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Utility/BsPersistentCache.h"
#include "Reflection/BsRTTIType.h"
#include "Reflection/BsRTTIPlain.h"
#include "Private/RTTI/BsResourceRTTI.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class B3D_CORE_EXPORT PersistentCacheObjectRTTI : public TRTTIType<PersistentCacheObject, Resource, PersistentCacheObjectRTTI>
	{
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_CONTAINER(mObjects, 0)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "PersistentCacheObject";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_PersistentCacheObject;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return PersistentCacheObject::Create(nullptr);
		}
	};

	class B3D_CORE_EXPORT PersistentCacheMetaDataRTTI : public TRTTIType<PersistentCacheMetaData, IReflectable, PersistentCacheMetaDataRTTI>
	{
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(Priority, 0)
			B3D_RTTI_MEMBER(LastUsedTimestamp, 1)
			B3D_RTTI_MEMBER(CacheVersion, 2)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "PersistentCacheMetaData";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_PersistentCacheMetaData;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<PersistentCacheMetaData>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
