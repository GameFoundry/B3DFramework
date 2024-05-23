//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Components/BsCLight.h"
#include "Private/RTTI/BsGameObjectRTTI.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class B3D_CORE_EXPORT CLightRTTI : public RTTIType<CLight, Component, CLightRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(mInternal, 0)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "CLight";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_CLight;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return SceneObject::CreateEmptyComponent<CLight>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
