//********************************* B3D Framework - Copyright 2026 Marko Pintera *****************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "RTTI/B3DResourceRTTI.h"
#include "B3DManagedResource.h"
#include "B3DMonoManager.h"
#include "Serialization/B3DManagedSerializableObject.h"
#include "Resources/B3DResources.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-SEngine
	 *  @{
	 */

	class B3D_SCRIPT_INTEROP_EXPORT ManagedResourceRTTI : public TRTTIType<ManagedResource, Resource, ManagedResourceRTTI>
	{
		SPtr<ManagedSerializableObject> mSerializedObjectData;

		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_GENERATED_MEMBER(mSerializedObjectData, 0)
			B3D_RTTI_MEMBER(mMissingType, 1)
		B3D_RTTI_END_MEMBERS

	public:
		void OnOperationStarted(ManagedResource& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(operationType.IsSet(RTTIOperationType::ReadBit))
			{
				MonoObject* managedInstance = object.GetManagedInstance();

				if(managedInstance != nullptr)
					mSerializedObjectData = ManagedSerializableObject::CreateFromExisting(managedInstance);
				else
					mSerializedObjectData = object.mSerializedObjectData;
			}
		}

		void OnOperationEnded(ManagedResource& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(operationType.IsSet(RTTIOperationType::WriteBit) && !operationType.IsSet(RTTIOperationType::PreExistingObjectBit))
			{
				const SPtr<ManagedResource>& managedResource = std::static_pointer_cast<ManagedResource>(object.GetShared());
				managedResource->mSerializedObjectData = mSerializedObjectData; 
				managedResource->Initialize();
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
			return ManagedResource::CreateUninitializedAsShared();
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
