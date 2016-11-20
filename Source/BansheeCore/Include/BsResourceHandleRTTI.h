//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsRTTIType.h"
#include "BsResourceHandle.h"
#include "BsResources.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class BS_CORE_EXPORT ResourceHandleRTTI : public RTTIType<TResourceHandleBase<false>, IReflectable, ResourceHandleRTTI>
	{
	private:
		String& getUUID(TResourceHandleBase<false>* obj)
		{ 
			static String Blank = "";

			return obj->mData != nullptr ? obj->mData->mUUID : Blank; 
		}

		void setUUID(TResourceHandleBase<false>* obj, String& uuid) { obj->mData->mUUID = uuid; }
	public:
		ResourceHandleRTTI()
		{
			addPlainField("mUUID", 0, &ResourceHandleRTTI::getUUID, &ResourceHandleRTTI::setUUID);
		}

		void onDeserializationEnded(IReflectable* obj, const UnorderedMap<String, UINT64>& params) override
		{
			TResourceHandleBase<false>* resourceHandle = static_cast<TResourceHandleBase<false>*>(obj);

			if(resourceHandle->mData && resourceHandle->mData->mUUID != "")
			{
				HResource loadedResource = gResources()._getResourceHandle(resourceHandle->mData->mUUID);

				resourceHandle->releaseRef();
				resourceHandle->mData = loadedResource.mData;
				resourceHandle->addRef();
			}
		}

		const String& getRTTIName() override
		{
			static String name = "ResourceHandleBase";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_ResourceHandle;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			SPtr<TResourceHandleBase<false>> obj = bs_shared_ptr<TResourceHandleBase<false>>
				(new (bs_alloc<TResourceHandleBase<false>>()) TResourceHandleBase<false>());
			obj->mData = bs_shared_ptr_new<ResourceHandleData>();
			obj->mData->mRefCount++;

			return obj;
		}
	};

	class BS_CORE_EXPORT WeakResourceHandleRTTI : public RTTIType<TResourceHandleBase<true>, IReflectable, WeakResourceHandleRTTI>
	{
	private:
		String& getUUID(TResourceHandleBase<true>* obj)
		{
			static String Blank = "";

			return obj->mData != nullptr ? obj->mData->mUUID : Blank;
		}

		void setUUID(TResourceHandleBase<true>* obj, String& uuid) { obj->mData->mUUID = uuid; }
	public:
		WeakResourceHandleRTTI()
		{
			addPlainField("mUUID", 0, &WeakResourceHandleRTTI::getUUID, &WeakResourceHandleRTTI::setUUID);
		}

		void onDeserializationEnded(IReflectable* obj, const UnorderedMap<String, UINT64>& params) override
		{
			TResourceHandleBase<true>* resourceHandle = static_cast<TResourceHandleBase<true>*>(obj);

			if (resourceHandle->mData && resourceHandle->mData->mUUID != "")
			{
				HResource loadedResource = gResources()._getResourceHandle(resourceHandle->mData->mUUID);
				resourceHandle->mData = loadedResource.mData;
			}
		}

		const String& getRTTIName() override
		{
			static String name = "WeakResourceHandleBase";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_WeakResourceHandle;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			SPtr<TResourceHandleBase<true>> obj = bs_shared_ptr<TResourceHandleBase<true>>
				(new (bs_alloc<TResourceHandleBase<true>>()) TResourceHandleBase<true>());
			obj->mData = bs_shared_ptr_new<ResourceHandleData>();

			return obj;
		}
	};

	/** @} */
	/** @endcond */
}