//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisites.h"
#include "BsRTTIType.h"
#include "BsPostProcessSettings.h"

namespace BansheeEngine
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class BS_EXPORT PostProcessSettingsRTTI : public RTTIType <PostProcessSettings, IReflectable, PostProcessSettingsRTTI>
	{
	private:
		BS_PLAIN_MEMBER(enableAutoExposure);
		BS_PLAIN_MEMBER(autoExposure);
		BS_PLAIN_MEMBER(enableTonemapping);
		BS_PLAIN_MEMBER(tonemapping);
		BS_PLAIN_MEMBER(whiteBalance);
		BS_PLAIN_MEMBER(colorGrading);
		BS_PLAIN_MEMBER(exposureScale);
		BS_PLAIN_MEMBER(gamma);

	public:
		PostProcessSettingsRTTI()
		{
			BS_ADD_PLAIN_FIELD(enableAutoExposure, 0);
			BS_ADD_PLAIN_FIELD(autoExposure, 1);
			BS_ADD_PLAIN_FIELD(enableTonemapping, 2);
			BS_ADD_PLAIN_FIELD(tonemapping, 3);
			BS_ADD_PLAIN_FIELD(whiteBalance, 4);
			BS_ADD_PLAIN_FIELD(colorGrading, 5);
			BS_ADD_PLAIN_FIELD(exposureScale, 6);
			BS_ADD_PLAIN_FIELD(gamma, 7);
		}

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
			return PostProcessSettings::create();
		}
	};

	BS_ALLOW_MEMCPY_SERIALIZATION(AutoExposureSettings);
	BS_ALLOW_MEMCPY_SERIALIZATION(WhiteBalanceSettings);
	BS_ALLOW_MEMCPY_SERIALIZATION(ColorGradingSettings);
	BS_ALLOW_MEMCPY_SERIALIZATION(TonemappingSettings);

	/** @} */
	/** @endcond */
}