//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Image/BsColorGradient.h"
#include "RTTI/BsColorGradientRTTI.h"
#include "Debug/BsDebug.h"
#include "Utility/BsBitwise.h"

using namespace bs;

template <class COLOR, class TIME>
constexpr u32 TColorGradient<COLOR, TIME>::kMaxKeys;

template <class COLOR, class TIME>
TColorGradient<COLOR, TIME>::TColorGradient(const Color& color)
{
	SetConstant(color);
}

template <class COLOR, class TIME>
TColorGradient<COLOR, TIME>::TColorGradient(const Vector<ColorGradientKey>& keys)
{
	SetKeys(keys);
}

template <class COLOR, class TIME>
COLOR TColorGradient<COLOR, TIME>::Evaluate(float t) const
{
	if(mNumKeys == 0)
		return COLOR();

	if(mNumKeys == 1)
		return mColors[0];

	if(mDuration > 0.0f)
		t = t / mDuration;

	const auto time = impl::TGradientHelper<COLOR>::GetInternalTime(Math::Clamp01(t));

	if(time < mTimes[0])
		return mColors[0];

	// Note: Add a version of evaluate that supports caching?
	for(uint32_t i = 1; i < mNumKeys; i++)
	{
		const auto curKeyTime = mTimes[i];
		if(time > curKeyTime)
			continue;

		const auto prevKeyTime = mTimes[i - 1];
		const auto fracColor = impl::TGradientHelper<COLOR>::InvLerp(prevKeyTime, curKeyTime, time);
		return Color::Lerp(fracColor, mColors[i - 1], mColors[i]);
	}

	return mColors[mNumKeys - 1];
}

template <class COLOR, class TIME>
void TColorGradient<COLOR, TIME>::SetKeys(const Vector<ColorGradientKey>& keys, float duration)
{
#if BS_DEBUG_MODE
	// Ensure keys are sorted
	if(!keys.empty())
	{
		float time = keys[0].Time;
		for(u32 i = 1; i < (u32)keys.size(); i++)
		{
			assert(keys[i].Time >= time);
			time = keys[i].Time;
		}
	}
#endif

	if(keys.size() > kMaxKeys)
	{
		BS_LOG(Warning, Generic, "Number of keys in ColorGradient exceeds the support number ({0}). "
								 "Keys will be ignored.",
			   kMaxKeys);
	}

	mDuration = duration;
	mNumKeys = 0;

	for(auto& key : keys)
	{
		if(mNumKeys >= kMaxKeys)
			break;

		mColors[mNumKeys] = impl::TGradientHelper<COLOR>::ToInternalColor(key.Color);
		mTimes[mNumKeys] = impl::TGradientHelper<COLOR>::GetInternalTime(Math::Clamp01(key.Time));

		mNumKeys++;
	}
}

template <class COLOR, class TIME>
Vector<ColorGradientKey> TColorGradient<COLOR, TIME>::GetKeys() const
{
	Vector<ColorGradientKey> output(mNumKeys);
	for(u32 i = 0; i < mNumKeys; i++)
	{
		output[i].Color = impl::TGradientHelper<COLOR>::FromInternalColor(mColors[i]);
		output[i].Time = impl::TGradientHelper<COLOR>::FromInternalTime(mTimes[i]);
	}

	return output;
}

template <class COLOR, class TIME>
ColorGradientKey TColorGradient<COLOR, TIME>::GetKey(u32 idx) const
{
	if(idx >= mNumKeys)
		return ColorGradientKey(Color::kBlack, 0.0f);

	return ColorGradientKey(
		impl::TGradientHelper<COLOR>::FromInternalColor(mColors[idx]),
		impl::TGradientHelper<COLOR>::FromInternalTime(mTimes[idx]));
}

template <class COLOR, class TIME>
void TColorGradient<COLOR, TIME>::SetConstant(const Color& color)
{
	mColors[0] = impl::TGradientHelper<COLOR>::ToInternalColor(color);
	mTimes[0] = 0;
	mNumKeys = 1;
	mDuration = 0.0f;
}

template <class COLOR, class TIME>
std::pair<float, float> TColorGradient<COLOR, TIME>::GetTimeRange() const
{
	if(mNumKeys == 0)
		return std::make_pair(0.0f, 0.0f);

	if(mNumKeys == 1)
	{
		float time = impl::TGradientHelper<COLOR>::FromInternalTime(mTimes[0]);
		return std::make_pair(time, time);
	}

	return std::make_pair(
		impl::TGradientHelper<COLOR>::FromInternalTime(mTimes[0]),
		impl::TGradientHelper<COLOR>::FromInternalTime(mTimes[mNumKeys - 1]));
}

template <class COLOR, class TIME>
bool TColorGradient<COLOR, TIME>::operator==(const TColorGradient<COLOR, TIME>& rhs) const
{
	if(mNumKeys != rhs.mNumKeys || mDuration != rhs.mDuration)
		return false;

	for(uint32_t i = 0; i < mNumKeys; i++)
	{
		if(mColors[i] != rhs.mColors[i] || mTimes[i] != rhs.mTimes[i])
			return false;
	}

	return true;
}

template class TColorGradient<RGBA, uint16_t>;
template class TColorGradient<Color, float>;
