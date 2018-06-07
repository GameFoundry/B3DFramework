//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Image/BsColorGradient.h"
#include "Debug/BsDebug.h"
#include "Utility/BsBitwise.h"

namespace bs
{
	RGBA ColorGradient::evaluate(float t) const
	{
		if(mNumKeys == 0)
			return 0;

		if(mNumKeys == 1)
			return mColors[0];

		if(mDuration > 0.0f)
			t = t / mDuration;

		uint32_t time = Bitwise::unormToUint<16>(t);
		time = Math::clamp(time, (uint32_t)mTimes[0], (uint32_t)mTimes[mNumKeys - 1]);

		// Note: Add a version of evaluate that supports caching?
		for(UINT32 i = 1; i < mNumKeys; i++)
		{
			const uint32_t curKeyTime = mTimes[i];
			if(time > curKeyTime)
				continue;

			const uint32_t prevKeyTime = mTimes[i - 1];
			const uint32_t fracColor = Bitwise::invLerpWord(prevKeyTime, curKeyTime, time) >> 8;
			return Color::lerp(fracColor, mColors[i - 1], mColors[i]);
		}

		return mColors[mNumKeys - 1];
	}

	void ColorGradient::setKeys(const Vector<ColorGradientKey>& keys)
	{
#if BS_DEBUG_MODE
		// Ensure keys are sorted
		if(!keys.empty())
		{
			float time = keys[0].time;
			for (UINT32 i = 1; i < (UINT32)keys.size(); i++)
			{
				assert(keys[i].time >= time);
				time = keys[i].time;
			}
		}
#endif

		if(keys.size() > MAX_KEYS)
		{
			LOGWRN("Number of keys in ColorGradient exceeds the support number (" + 
				toString(MAX_KEYS) + "). Keys will be ignored.");
		}

		if(!keys.empty())
			mDuration = keys.back().time;
		else
			mDuration = 0.0f;


		mNumKeys = 0;
		for(auto& key : keys)
		{
			if(mNumKeys >= MAX_KEYS)
				break;

			mColors[mNumKeys] = key.color.getAsRGBA();
			mTimes[mNumKeys] = Bitwise::unormToUint<16>(key.time / mDuration);

			mNumKeys++;
		}
	}

	void ColorGradient::setConstant(const Color& color)
	{
		mColors[0] = color.getAsRGBA();
		mTimes[0] = 0;
		mNumKeys = 1;
		mDuration = 0.0f;
	}
}
