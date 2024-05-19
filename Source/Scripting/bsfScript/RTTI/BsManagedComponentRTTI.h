//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Private/RTTI/BsGameObjectRTTI.h"
#include "BsManagedComponent.h"
#include "BsMonoManager.h"
#include "Serialization/BsManagedSerializableObject.h"
#include "Scene/BsGameObjectManager.h"
#include "Wrappers/BsScriptComponent.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-SEngine
	 *  @{
	 */

	class B3D_SCRIPT_INTEROP_EXPORT ManagedComponentRTTI : public RTTIType<ManagedComponent, Component, ManagedComponentRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_PLAIN(mNamespace, 0)
			B3D_RTTI_MEMBER_PLAIN(mTypeName, 1)
			B3D_RTTI_MEMBER_PLAIN(mMissingType, 3)
		B3D_RTTI_END_MEMBERS

		SPtr<ManagedSerializableObject> GetObjectData(ManagedComponent* obj)
		{
			return mSerializedObjectData;
		}

		void SetObjectData(ManagedComponent* obj, SPtr<ManagedSerializableObject> val)
		{
			obj->mSerializedObjectData = val;
		}

	public:
		ManagedComponentRTTI()
		{
			AddReflectablePtrField("mObjectData", 2, &ManagedComponentRTTI::GetObjectData, &ManagedComponentRTTI::SetObjectData);
		}

		void OnSerializationStarted(IReflectable* obj, RTTIOperationContext* context) override
		{
			ManagedComponent* mc = static_cast<ManagedComponent*>(obj);
			MonoObject* managedInstance = mc->GetManagedInstance();

			if(managedInstance != nullptr)
				mSerializedObjectData = ManagedSerializableObject::CreateFromExisting(managedInstance);
			else
				mSerializedObjectData = mc->mSerializedObjectData;
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

	private:
		SPtr<ManagedSerializableObject> mSerializedObjectData;
	};

	/** @} */
	/** @endcond */
} // namespace bs
