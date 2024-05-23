//************************************ bs::framework - Copyright 2024 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Physics/BsD6Joint.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class B3D_CORE_EXPORT D6JointDriveRTTI : public RTTIType<D6JointDrive, IReflectable, D6JointDriveRTTI>
	{
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(Stiffness, 0)
			B3D_RTTI_MEMBER(Damping, 1)
			B3D_RTTI_MEMBER(ForceLimit, 2)
			B3D_RTTI_MEMBER(Acceleration, 3)
		B3D_RTTI_END_MEMBERS
	public:
		const String& GetRttiName() override
		{
			static String name = "D6JointDrive";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_D6JointDrive;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<D6JointDrive>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
