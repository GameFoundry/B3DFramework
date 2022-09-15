//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Components/BsCFixedJoint.h"
#include "Private/RTTI/BsGameObjectRTTI.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class BS_CORE_EXPORT CFixedJointRTTI : public RTTIType<CFixedJoint, CJoint, CFixedJointRTTI>
	{
	public:
		CFixedJointRTTI()
		{ }

		const String& GetRttiName() override
		{
			static String name = "CFixedJoint";
			return name;
		}

		UINT32 GetRttiId() override
		{
			return TID_CFixedJoint;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return SceneObject::CreateEmptyComponent<CFixedJoint>();
		}
	};

	/** @} */
	/** @endcond */
}
