//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
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
		String& getNamespace(ManagedComponent* obj)
		{
			return obj->mNamespace;
		}

		void setNamespace(ManagedComponent* obj, String& val)
		{
			obj->mNamespace = val;
		}

		String& getTypename(ManagedComponent* obj)
		{
			return obj->mTypeName;
		}

		void setTypename(ManagedComponent* obj, String& val)
		{
			obj->mTypeName = val;
		}

		SPtr<ManagedSerializableObject> getObjectData(ManagedComponent* obj)
		{
			return any_cast<SPtr<ManagedSerializableObject>>(obj->mRTTIData);
		}

		void setObjectData(ManagedComponent* obj, SPtr<ManagedSerializableObject> val)
		{
			obj->mSerializedObjectData = val;
		}

		bool& getMissingType(ManagedComponent* obj)
		{
			return obj->mMissingType;
		}

		void setMissingType(ManagedComponent* obj, bool& val)
		{
			obj->mMissingType = val;
		}

	public:
		ManagedComponentRTTI()
		{
			addPlainField("mNamespace", 0, &ManagedComponentRTTI::getNamespace, &ManagedComponentRTTI::setNamespace);
			addPlainField("mTypename", 1, &ManagedComponentRTTI::getTypename, &ManagedComponentRTTI::setTypename);
			addReflectablePtrField("mObjectData", 2, &ManagedComponentRTTI::getObjectData, &ManagedComponentRTTI::setObjectData);
			addPlainField("mMissingType", 3, &ManagedComponentRTTI::getMissingType, &ManagedComponentRTTI::setMissingType);
		}

		void onSerializationStarted(IReflectable* obj, const UnorderedMap<String, UINT64>& params) override
		{
			ManagedComponent* mc = static_cast<ManagedComponent*>(obj);
			MonoObject* managedInstance = mc->getManagedInstance();

			if (managedInstance != nullptr)
				mc->mRTTIData = ManagedSerializableObject::createFromExisting(managedInstance);
			else
				mc->mRTTIData = mc->mSerializedObjectData;
		}

		void onSerializationEnded(IReflectable* obj, const UnorderedMap<String, UINT64>& params) override
		{
			ManagedComponent* mc = static_cast<ManagedComponent*>(obj);
			mc->mRTTIData = nullptr;
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
	};

	/** @} */
	/** @endcond */
}