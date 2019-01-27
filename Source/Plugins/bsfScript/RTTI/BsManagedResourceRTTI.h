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
		SPtr<ManagedSerializableObject> getObjectData(ManagedResource* obj)
		{
			return any_cast<SPtr<ManagedSerializableObject>>(mSerializableObject);
		}

		void setObjectData(ManagedResource* obj, SPtr<ManagedSerializableObject> val)
		{
			mSerializableObject = val;
		}

	public:
		ManagedResourceRTTI()
		{
			addReflectablePtrField("mObjectData", 0, &ManagedResourceRTTI::getObjectData, &ManagedResourceRTTI::setObjectData);
		}

		void onSerializationStarted(IReflectable* obj, SerializationContext* context) override
		{
			ManagedResource* mr = static_cast<ManagedResource*>(obj);
			mSerializableObject = ManagedSerializableObject::createFromExisting(mr->getManagedInstance());
		}

		void onDeserializationEnded(IReflectable* obj, SerializationContext* context) override
		{
			ManagedResource* mr = static_cast<ManagedResource*>(obj);

			SPtr<Resource> mrPtr = std::static_pointer_cast<Resource>(mr->getThisPtr());
			HManagedResource handle = static_resource_cast<ManagedResource>(gResources()._createResourceHandle(mrPtr));
			mr->setHandle(mSerializableObject->deserialize(), handle);
		}

		const String& getRTTIName() override
		{
			static String name = "ManagedResource";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_ManagedResource;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return ManagedResource::createEmpty();
		}

	private:
		SPtr<ManagedSerializableObject> mSerializableObject;
	};

	/** @} */
	/** @endcond */
}