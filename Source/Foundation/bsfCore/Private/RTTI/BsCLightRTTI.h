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

	class BS_CORE_EXPORT CLightRTTI : public RTTIType <CLight, Component, CLightRTTI>
	{
	private:
		SPtr<Light> GetInternal(CLight* obj) { return obj->mInternal; }
		void SetInternal(CLight* obj, SPtr<Light> val) { obj->mInternal = val; }

	public:
		CLightRTTI()
		{
			AddReflectablePtrField("mInternal", 0, &CLightRTTI::GetInternal, &CLightRTTI::SetInternal);
		}

		const String& GetRttiName() override
		{
			static String name = "CLight";
			return name;
		}

		UINT32 GetRttiId() override
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
}
