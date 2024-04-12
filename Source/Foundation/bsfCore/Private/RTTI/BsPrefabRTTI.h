//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "RTTI/BsUUIDRTTI.h"
#include "Scene/BsPrefab.h"
#include "Scene/BsSceneObject.h"
#include "Utility/BsUtility.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class B3D_CORE_EXPORT PrefabRTTI : public RTTIType<Prefab, Resource, PrefabRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_PLAIN(mPrefabVersion, 1)
			// B3D_RTTI_MEMBER_PLAIN(mNextLinkId, 2)
			B3D_RTTI_MEMBER_PLAIN(mUUID, 3)
			B3D_RTTI_MEMBER_PLAIN(mIsScene, 4)
		B3D_RTTI_END_MEMBERS

		SPtr<SceneObject> GetSceneObject(Prefab* obj) { return obj->mRoot.GetShared(); }

		void SetSceneObject(Prefab* obj, SPtr<SceneObject> value) { obj->mRoot = value->GetHandle(); }

	public:
		PrefabRTTI()
		{
			AddReflectablePtrField("mRoot", 0, &PrefabRTTI::GetSceneObject, &PrefabRTTI::SetSceneObject);
		}

		void OnDeserializationStarted(IReflectable* object, SerializationContext* context) override
		{
			Prefab* const prefab = static_cast<Prefab*>(object);

			CoreSerializationContext* const serializationContext = B3DRTTICast<CoreSerializationContext>(context);
			if(B3D_ENSURE(serializationContext != nullptr))
				serializationContext->GameObjectCollection = prefab->mGameObjectCollection;
		}

		const String& GetRttiName()
		{
			static String name = "Prefab";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_Prefab;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return Prefab::CreateEmpty();
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
