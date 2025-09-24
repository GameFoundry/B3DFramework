//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Components/BsFixedJoint.h"
#include "Private/RTTI/BsGameObjectRTTI.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class B3D_CORE_EXPORT FixedJointRTTI : public TRTTIType<FixedJoint, Joint, FixedJointRTTI>
	{
	public:
		const String& GetRttiName() override
		{
			static String name = "FixedJoint";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_FixedJoint;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return SceneObject::CreateEmptyComponent<FixedJoint>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
