//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "RTTI/BsStringRTTI.h"
#include "RTTI/BsUUIDRTTI.h"
#include "RTTI/BsFlagsRTTI.h"
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
	};

	class B3D_CORE_EXPORT GameObjectRTTI : public RTTIType<GameObject, IReflectable, GameObjectRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_PLAIN(mName, 1)
			// B3D_RTTI_MEMBER_PLAIN(mLinkId, 2)
			B3D_RTTI_MEMBER_PLAIN_INFO(mId, 3, RTTIFieldInfo(RTTIFieldFlag::SkipInDeltaCompare))
			B3D_RTTI_MEMBER_PLAIN_INFO(mPrefabObjectId, 4, RTTIFieldInfo(RTTIFieldFlag::SkipInDeltaCompare))
			B3D_RTTI_MEMBER_PLAIN(mPersistentGameObjectFlags, 5)
		B3D_RTTI_END_MEMBERS

	public:
		GameObjectRTTI()
		{
			//AddPlainField("mInstanceID", 0, &GameObjectRTTI::GetInstanceId, &GameObjectRTTI::SetInstanceId);
		}

		void OnOperationStarted(GameObject& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(operationType.IsSet(RTTIOperationType::WriteBit))
			{
				// It's possible we're just accessing the game object fields, in which case the process below is not needed
				// (it's only required for new game objects).
				if(object.mRTTIData.Empty())
					return;

				SPtr<GameObject> gameObjectPtr = AnyCast<SPtr<GameObject>>(object.mRTTIData);

				// Every GameObject must store GODeserializationData in its RTTI data field during deserialization
				object.mRTTIData = GODeserializationData();
				GODeserializationData& deserializationData = AnyCastRef<GODeserializationData>(object.mRTTIData);

				// Store shared pointer since the system only provides us with raw ones
				deserializationData.Ptr = gameObjectPtr;
			}
		}

		const String& GetRttiName()
		{
			static String name = "GameObject";
			return name;
		}

		u32 GetRttiId() const override
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
