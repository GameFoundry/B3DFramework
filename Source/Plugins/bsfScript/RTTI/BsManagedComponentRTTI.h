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

	class BS_SCR_BE_EXPORT ManagedComponentRTTI : public RTTIType<ManagedComponent, Component, ManagedComponentRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN(mNamespace, 0)
			BS_RTTI_MEMBER_PLAIN(mTypeName, 1)
			BS_RTTI_MEMBER_PLAIN(mMissingType, 3)
		BS_END_RTTI_MEMBERS

		SPtr<ManagedSerializableObject> getObjectData(ManagedComponent* obj)
		{
			return mSerializedObjectData;
		}

		void setObjectData(ManagedComponent* obj, SPtr<ManagedSerializableObject> val)
		{
			obj->mSerializedObjectData = val;
		}

	public:
		ManagedComponentRTTI()
		{
			addReflectablePtrField("mObjectData", 2, &ManagedComponentRTTI::getObjectData, &ManagedComponentRTTI::setObjectData);
		}

		void onSerializationStarted(IReflectable* obj, SerializationContext* context) override
		{
			ManagedComponent* mc = static_cast<ManagedComponent*>(obj);
			MonoObject* managedInstance = mc->getManagedInstance();

			if (managedInstance != nullptr)
				mSerializedObjectData = ManagedSerializableObject::createFromExisting(managedInstance);
			else
				mSerializedObjectData = mc->mSerializedObjectData;
		}

		const String& getRTTIName() override
		{
			static String name = "ManagedComponent";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_ManagedComponent;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return GameObjectRTTI::createGameObject<ManagedComponent>();
		}

	private:
		SPtr<ManagedSerializableObject> mSerializedObjectData;
	};

	/** @} */
	/** @endcond */
}