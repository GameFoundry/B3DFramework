//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Utility/B3DPersistentCache.h"
#include "Reflection/B3DRTTIType.h"
#include "Reflection/B3DRTTIPlain.h"
#include "RTTI/B3DResourceRTTI.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class B3D_EXPORT PersistentCacheObjectRTTI : public TRTTIType<PersistentCacheObject, Resource, PersistentCacheObjectRTTI>
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

		TShared<IReflectable> NewRttiObject() override
		{
			return PersistentCacheObject::Create(nullptr);
		}
	};

	class B3D_EXPORT PersistentCacheMetaDataRTTI : public TRTTIType<PersistentCacheMetaData, IReflectable, PersistentCacheMetaDataRTTI>
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

		TShared<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<PersistentCacheMetaData>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
