//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "RTTI/B3DStringRTTI.h"
#include "RTTI/B3DUUIDRTTI.h"
#include "RTTI/B3DFlagsRTTI.h"
#include "Scene/B3DGameObject.h"
#include "Scene/B3DSceneObject.h"
#include "Scene/B3DGameObjectManager.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	/**	Provides temporary storage for data used during GameObject deserialization. */
	struct GODeserializationData
	{
		TShared<GameObject> Ptr;
	};

	class B3D_EXPORT GameObjectRTTI : public TRTTIType<GameObject, IReflectable, GameObjectRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(mName, 1)
			B3D_RTTI_MEMBER_INFO(mId, 3, RTTIFieldInfo(RTTIFieldFlag::SkipInDeltaCompare))
			B3D_RTTI_MEMBER_INFO(mPrefabObjectId, 4, RTTIFieldInfo(RTTIFieldFlag::SkipInDeltaCompare))
			B3D_RTTI_MEMBER(mPersistentGameObjectFlags, 5)
		B3D_RTTI_END_MEMBERS

	public:
		void OnOperationStarted(GameObject& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(operationType.IsSet(RTTIOperationType::WriteBit))
			{
				// It's possible we're just accessing the game object fields, in which case the process below is not needed
				// (it's only required for new game objects).
				if(object.mRTTIData.Empty())
					return;

				TShared<GameObject> gameObjectPtr = AnyCast<TShared<GameObject>>(object.mRTTIData);

				// Every GameObject must store GODeserializationData in its RTTI data field during deserialization
				object.mRTTIData = GODeserializationData();
				GODeserializationData& deserializationData = AnyCastRef<GODeserializationData>(object.mRTTIData);

				// Store shared pointer since the system only provides us with raw ones
				deserializationData.Ptr = gameObjectPtr;
			}
		}

		const String& GetRttiName() override
		{
			static String name = "GameObject";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_GameObject;
		}

		TShared<IReflectable> NewRttiObject() override
		{
			B3D_ASSERT(false && "Cannot instantiate an abstract class.");
			return nullptr;
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
