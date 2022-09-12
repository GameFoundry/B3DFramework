//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Private/RTTI/BsResourceRTTI.h"
#include "BsManagedResource.h"
#include "BsMonoManager.h"
#include "Serialization/BsManagedSerializableObject.h"
#include "Resources/BsResources.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-SEngine
	 *  @{
	 */

	class BS_SCR_BE_EXPORT ManagedResourceRTTI : public RTTIType<ManagedResource, Resource, ManagedResourceRTTI>
	{
	private:
		SPtr<ManagedSerializableObject> GetObjectData(ManagedResource* obj)
		{
			return any_cast<SPtr<ManagedSerializableObject>>(mSerializableObject);
		}

		void SetObjectData(ManagedResource* obj, SPtr<ManagedSerializableObject> val)
		{
			mSerializableObject = val;
		}

	public:
		ManagedResourceRTTI()
		{
			addReflectablePtrField("mObjectData", 0, &ManagedResourceRTTI::getObjectData, &ManagedResourceRTTI::setObjectData);
		}

		void OnSerializationStarted(IReflectable* obj, SerializationContext* context) override
		{
			ManagedResource* mr = static_cast<ManagedResource*>(obj);
			mSerializableObject = ManagedSerializableObject::createFromExisting(mr->GetManagedInstance());
		}

		void OnDeserializationEnded(IReflectable* obj, SerializationContext* context) override
		{
			ManagedResource* mr = static_cast<ManagedResource*>(obj);

			SPtr<Resource> mrPtr = std::static_pointer_cast<Resource>(mr->GetThisPtr());
			HManagedResource handle = static_resource_cast<ManagedResource>(gResources()._createResourceHandle(mrPtr));
			mr->SetHandle(mSerializableObject->deserialize(), handle);
		}

		const String& GetRTTIName() override
		{
			static String name = "ManagedResource";
			return name;
		}

		UINT32 GetRTTIId() override
		{
			return TID_ManagedResource;
		}

		SPtr<IReflectable> NewRTTIObject() override
		{
			return ManagedResource::CreateEmpty();
		}

	private:
		SPtr<ManagedSerializableObject> mSerializableObject;
	};

	/** @} */
	/** @endcond */
}
