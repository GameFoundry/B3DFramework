//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Components/BsCSphereCollider.h"
#include "Private/RTTI/BsGameObjectRTTI.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class B3D_CORE_EXPORT CSphereColliderRTTI : public RTTIType<CSphereCollider, CCollider, CSphereColliderRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(mRadius, 0)
		B3D_RTTI_END_MEMBERS
	public:
		const String& GetRttiName() override
		{
			static String name = "CSphereCollider";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_CSphereCollider;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return SceneObject::CreateEmptyComponent<CSphereCollider>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
