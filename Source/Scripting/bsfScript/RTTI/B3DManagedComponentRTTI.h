//********************************* B3D Framework - Copyright 2026 Marko Pintera *****************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "RTTI/B3DGameObjectRTTI.h"
#include "B3DManagedComponent.h"
#include "B3DMonoManager.h"
#include "Serialization/B3DManagedSerializableObject.h"
#include "Scene/B3DGameObjectManager.h"
#include "Wrappers/B3DScriptComponent.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-SEngine
	 *  @{
	 */

	class B3D_SCRIPT_INTEROP_EXPORT ManagedComponentRTTI : public TRTTIType<ManagedComponent, Component, ManagedComponentRTTI>
	{
		SPtr<ManagedSerializableObject> mSerializedObjectData;

		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(mNamespace, 0)
			B3D_RTTI_MEMBER(mTypeName, 1)
			B3D_RTTI_GENERATED_MEMBER(mSerializedObjectData, 2)
			B3D_RTTI_MEMBER(mMissingType, 3)
		B3D_RTTI_END_MEMBERS

	public:
		void OnOperationStarted(ManagedComponent& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
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

		void OnOperationEnded(ManagedComponent& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(operationType.IsSet(RTTIOperationType::WriteBit))
			{
				object.mSerializedObjectData = mSerializedObjectData;
			}
		}

		const String& GetRttiName() override
		{
			static String name = "ManagedComponent";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_ManagedComponent;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return SceneObject::CreateEmptyComponent<ManagedComponent>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
