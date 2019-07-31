//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Image/BsColorGradient.h"
#include "Private/RTTI/BsColorGradientRTTI.h"
#include "Debug/BsDebug.h"
#include "Utility/BsBitwise.h"

namespace bs
{
	template<class COLOR, class TIME>
	constexpr UINT32 TColorGradient<COLOR, TIME>::MAX_KEYS;

	template<class COLOR, class TIME>
	TColorGradient<COLOR, TIME>::TColorGradient(const Color& color)
	{
		setConstant(color);
	}

	template<class COLOR, class TIME>
	TColorGradient<COLOR, TIME>::TColorGradient(const Vector<ColorGradientKey>& keys)
	{
		setKeys(keys);
	}

	template<class COLOR, class TIME>
	COLOR TColorGradient<COLOR, TIME>::evaluate(float t) const
	{
		if(mNumKeys == 0)
			return COLOR();

		if(mNumKeys == 1)
			return mColors[0];

		if(mDuration > 0.0f)
			t = t / mDuration;

		const auto time = impl::TGradientHelper<COLOR>::getInternalTime(Math::clamp01(t));

		if(time < mTimes[0])
			return mColors[0];

		// Note: Add a version of evaluate that supports caching?
		for(uint32_t i = 1; i < mNumKeys; i++)
		{
			const auto curKeyTime = mTimes[i];
			if(time > curKeyTime)
				continue;

			const auto prevKeyTime = mTimes[i - 1];
			const auto fracColor = impl::TGradientHelper<COLOR>::invLerp(prevKeyTime, curKeyTime, time);
			return Color::lerp(fracColor, mColors[i - 1], mColors[i]);
		}

		return mColors[mNumKeys - 1];
	}

	template<class COLOR, class TIME>
	void TColorGradient<COLOR, TIME>::setKeys(const Vector<ColorGradientKey>& keys, float duration)
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
			BS_LOG(Warning, Generic, "Number of keys in ColorGradient exceeds the support number ({0}). "
				"Keys will be ignored.", MAX_KEYS);
		}

		mDuration = duration;
		mNumKeys = 0;

		for(auto& key : keys)
		{
			if(mNumKeys >= MAX_KEYS)
				break;

			mColors[mNumKeys] = impl::TGradientHelper<COLOR>::toInternalColor(key.color);
			mTimes[mNumKeys] = impl::TGradientHelper<COLOR>::getInternalTime(Math::clamp01(key.time));

			mNumKeys++;
		}
	}

	template<class COLOR, class TIME>
	Vector<ColorGradientKey> TColorGradient<COLOR, TIME>::getKeys() const
	{
		Vector<ColorGradientKey> output(mNumKeys);
		for(UINT32 i = 0; i < mNumKeys; i++)
		{
			output[i].color = impl::TGradientHelper<COLOR>::fromInternalColor(mColors[i]);
			output[i].time = impl::TGradientHelper<COLOR>::fromInternalTime(mTimes[i]);
		}

		return output;
	}

	template<class COLOR, class TIME>
	ColorGradientKey TColorGradient<COLOR, TIME>::getKey(UINT32 idx) const
	{
		if(idx >= mNumKeys)
			return ColorGradientKey(Color::Black, 0.0f);

		return ColorGradientKey(
			impl::TGradientHelper<COLOR>::fromInternalColor(mColors[idx]),
			impl::TGradientHelper<COLOR>::fromInternalTime(mTimes[idx]));
	}

	template<class COLOR, class TIME>
	void TColorGradient<COLOR, TIME>::setConstant(const Color& color)
	{
		mColors[0] = impl::TGradientHelper<COLOR>::toInternalColor(color);
		mTimes[0] = 0;
		mNumKeys = 1;
		mDuration = 0.0f;
	}

	template<class COLOR, class TIME>
	std::pair<float, float> TColorGradient<COLOR, TIME>::getTimeRange() const
	{
		if(mNumKeys == 0)
			return std::make_pair(0.0f, 0.0f);

		if(mNumKeys == 1)
		{
			float time = impl::TGradientHelper<COLOR>::fromInternalTime(mTimes[0]);
			return std::make_pair(time, time);
		}

		return std::make_pair(
			impl::TGradientHelper<COLOR>::fromInternalTime(mTimes[0]),
			impl::TGradientHelper<COLOR>::fromInternalTime(mTimes[mNumKeys - 1])
		);
	}

	template<class COLOR, class TIME>
	bool TColorGradient<COLOR, TIME>::operator== (const TColorGradient<COLOR, TIME>& rhs) const
	{
		if (mNumKeys != rhs.mNumKeys || mDuration != rhs.mDuration)
			return false;

		for (uint32_t i = 0; i < mNumKeys; i++)
		{
			if (mColors[i] != rhs.mColors[i] || mTimes[i] != rhs.mTimes[i])
				return false;
		}

		return true;
	}

	template class TColorGradient<RGBA, uint16_t>;
	template class TColorGradient<Color, float>;
}
