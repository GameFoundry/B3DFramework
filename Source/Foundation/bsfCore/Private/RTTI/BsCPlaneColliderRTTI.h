//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Components/BsCPlaneCollider.h"
#include "Private/RTTI/BsGameObjectRTTI.h"
#include "RTTI/BsMathRTTI.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class B3D_CORE_EXPORT CPlaneColliderRTTI : public RTTIType<CPlaneCollider, CCollider, CPlaneColliderRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(mNormal, 0)
			B3D_RTTI_MEMBER(mDistance, 1)
		B3D_RTTI_END_MEMBERS
	public:
		const String& GetRttiName() override
		{
			static String name = "CPlaneCollider";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_CPlaneCollider;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return SceneObject::CreateEmptyComponent<CPlaneCollider>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
