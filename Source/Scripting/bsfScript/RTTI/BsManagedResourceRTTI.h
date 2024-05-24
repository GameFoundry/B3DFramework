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

	class B3D_SCRIPT_INTEROP_EXPORT ManagedResourceRTTI : public RTTIType<ManagedResource, Resource, ManagedResourceRTTI>
	{
		SPtr<ManagedSerializableObject> mSerializableObject;

		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_GENERATED_MEMBER(mSerializableObject, 0)
		B3D_RTTI_END_MEMBERS

	public:
		void OnOperationStarted(ManagedResource& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(operationType.IsSet(RTTIOperationType::ReadBit))
			{
				mSerializableObject = ManagedSerializableObject::CreateFromExisting(object.GetManagedInstance());
			}
		}

		void OnOperationEnded(ManagedResource& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(operationType.IsSet(RTTIOperationType::WriteBit))
			{
				SPtr<Resource> shared = std::static_pointer_cast<Resource>(object.GetShared());
				HManagedResource handle = B3DStaticResourceCast<ManagedResource>(GetResources().CreateResourceHandle(shared));
				object.SetHandle(mSerializableObject->Deserialize(), handle);
			}
		}

		const String& GetRttiName()
		{
			static String name = "ManagedResource";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_ManagedResource;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return ManagedResource::CreateEmpty();
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
