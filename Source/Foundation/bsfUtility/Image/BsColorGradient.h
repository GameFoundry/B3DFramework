//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"
#include "Image/BsColor.h"

namespace bs 
{
	/** @addtogroup Image
	 *  @{
	 */

	struct ColorGradientKey
	{
		Color color;
		float time;
	};

	// TODO - Doc
	class BS_UTILITY_EXPORT ColorGradient
	{
		static constexpr UINT32 MAX_KEYS = 8;
	public:
		RGBA evaluate(float t);

		// TODO - Must be sorted, time must be normalized
		void setKeys(const Vector<ColorGradientKey>& keys, float duration = 1.0f);
		void setConstant(const Color& color);

	private:
		RGBA mColors[MAX_KEYS];
		uint16_t mTimes[MAX_KEYS];
		uint32_t mNumKeys = 0;
		float mDuration = 0.0f;
	};

	/* @} */
}
