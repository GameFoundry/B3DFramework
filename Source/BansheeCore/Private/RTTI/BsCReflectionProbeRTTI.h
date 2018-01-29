//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Components/BsCReflectionProbe.h"
#include "Private/RTTI/BsGameObjectRTTI.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class BS_CORE_EXPORT CReflectionProbeRTTI : public RTTIType <CReflectionProbe, Component, CReflectionProbeRTTI>
	{
	private:
		SPtr<ReflectionProbe> getInternal(CReflectionProbe* obj) { return obj->mInternal; }
		void setInternal(CReflectionProbe* obj, SPtr<ReflectionProbe> val) { obj->mInternal = val; }

	public:
		CReflectionProbeRTTI()
		{
			addReflectablePtrField("mInternal", 0, &CReflectionProbeRTTI::getInternal, &CReflectionProbeRTTI::setInternal);
		}

		const String& getRTTIName() override
		{
			static String name = "CReflectionProbe";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_CReflectionProbe;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return GameObjectRTTI::createGameObject<CReflectionProbe>();
		}
	};

	/** @} */
	/** @endcond */
}