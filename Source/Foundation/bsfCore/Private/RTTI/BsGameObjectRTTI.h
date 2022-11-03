//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "RTTI/BsStringRTTI.h"
#include "RTTI/BsUUIDRTTI.h"
#include "Scene/BsGameObject.h"
#include "Scene/BsSceneObject.h"
#include "Scene/BsGameObjectManager.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	/**	Provides temporary storage for data used during GameObject deserialization. */
	struct GODeserializationData
	{
		SPtr<GameObject> Ptr;
		u64 OriginalId = 0;
	};

	class B3D_CORE_EXPORT GameObjectRTTI : public RTTIType<GameObject, IReflectable, GameObjectRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_PLAIN(mName, 1)
			B3D_RTTI_MEMBER_PLAIN(mLinkId, 2)
			B3D_RTTI_MEMBER_PLAIN(mUUID, 3)
		B3D_RTTI_END_MEMBERS

		u64& GetInstanceId(GameObject* obj) { return obj->mInstanceData->MInstanceId; }

		void SetInstanceId(GameObject* obj, u64& instanceId)
		{
			// We record the ID for later use. Any child RTTI of GameObject must call GameObjectManager::registerObject
			// with this ID, so we know how to map deserialized GO handles to live objects, otherwise the handle
			// references will get broken.
			GameObject* go = static_cast<GameObject*>(obj);
			GODeserializationData& deserializationData = AnyCastRef<GODeserializationData>(go->mRTTIData);

			deserializationData.OriginalId = instanceId;
		}

	public:
		GameObjectRTTI()
		{
			AddPlainField("mInstanceID", 0, &GameObjectRTTI::GetInstanceId, &GameObjectRTTI::SetInstanceId);
		}

		void OnDeserializationStarted(IReflectable* obj, SerializationContext* context)
		{
			GameObject* gameObject = static_cast<GameObject*>(obj);

			// It's possible we're just accessing the game object fields, in which case the process below is not needed
			// (it's only required for new game objects).
			if(gameObject->mRTTIData.Empty())
				return;

			SPtr<GameObject> gameObjectPtr = AnyCast<SPtr<GameObject>>(gameObject->mRTTIData);

			// Every GameObject must store GODeserializationData in its RTTI data field during deserialization
			gameObject->mRTTIData = GODeserializationData();
			GODeserializationData& deserializationData = AnyCastRef<GODeserializationData>(gameObject->mRTTIData);

			// Store shared pointer since the system only provides us with raw ones
			deserializationData.Ptr = gameObjectPtr;
		}

		const String& GetRttiName()
		{
			static String name = "GameObject";
			return name;
		}

		u32 GetRttiId()
		{
			return TID_GameObject;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			B3D_EXCEPT(InternalErrorException, "Cannot instantiate an abstract class.");
			return nullptr;
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
