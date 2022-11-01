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

	class BS_CORE_EXPORT PrefabRTTI : public RTTIType<Prefab, Resource, PrefabRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN(mHash, 1)
			// BS_RTTI_MEMBER_PLAIN(mNextLinkId, 2)
			BS_RTTI_MEMBER_PLAIN(mUUID, 3)
			BS_RTTI_MEMBER_PLAIN(mIsScene, 4)
		BS_END_RTTI_MEMBERS

		SPtr<SceneObject> GetSceneObject(Prefab* obj) { return obj->mRoot.GetInternalPtr(); }

		void SetSceneObject(Prefab* obj, SPtr<SceneObject> value) { obj->mRoot = value->GetHandle(); }

	public:
		PrefabRTTI()
		{
			AddReflectablePtrField("mRoot", 0, &PrefabRTTI::GetSceneObject, &PrefabRTTI::SetSceneObject);
		}

		void OnDeserializationStarted(IReflectable* ptr, SerializationContext* context)
		{
			BS_ASSERT(context != nullptr && B3DRTTIIsOfType<CoreSerializationContext>(context));
			auto coreContext = static_cast<CoreSerializationContext*>(context);

			// Make sure external IDs are broken because we do some ID matching when dealing with prefabs and keeping
			// the invalid external references could cause it to match invalid objects in case they end up having the
			// same ID.
			BS_ASSERT(!coreContext->GoState);
			coreContext->GoState = B3DMakeShared<GameObjectDeserializationState>(GODM_BreakExternal | GODM_UseNewIds);
		}

		const String& GetRttiName()
		{
			static String name = "Prefab";
			return name;
		}

		u32 GetRttiId()
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
