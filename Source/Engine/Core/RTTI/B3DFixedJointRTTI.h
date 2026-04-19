//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "Components/B3DFixedJoint.h"
#include "RTTI/B3DGameObjectRTTI.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class B3D_EXPORT FixedJointRTTI : public TRTTIType<FixedJoint, Joint, FixedJointRTTI>
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
