//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Components/BsCSkybox.h"
#include "Private/RTTI/BsGameObjectRTTI.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class BS_CORE_EXPORT CSkyboxRTTI : public RTTIType <CSkybox, Component, CSkyboxRTTI>
	{
	private:
		SPtr<Skybox> getInternal(CSkybox* obj) { return obj->mInternal; }
		void setInternal(CSkybox* obj, SPtr<Skybox> val) { obj->mInternal = val; }

	public:
		CSkyboxRTTI()
		{
			addReflectablePtrField("mInternal", 0, &CSkyboxRTTI::getInternal, &CSkyboxRTTI::setInternal);
		}

		const String& getRTTIName() override
		{
			static String name = "CSkybox";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_CSkybox;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return GameObjectRTTI::createGameObject<CSkybox>();
		}
	};

	/** @} */
	/** @endcond */
}