//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsRTTIType.h"
#include "BsPostProcessSettings.h"

namespace BansheeEngine
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class BS_CORE_EXPORT PostProcessSettingsRTTI : public RTTIType<PostProcessSettings, IReflectable, PostProcessSettingsRTTI>
	{
	private:

	public:
		PostProcessSettingsRTTI() { }

		const String& getRTTIName() override
		{
			static String name = "PostProcessSettings";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_PostProcessSettings;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			assert(false); // Abstract class
			return nullptr;
		}
	};

	/** @} */
	/** @endcond */
}