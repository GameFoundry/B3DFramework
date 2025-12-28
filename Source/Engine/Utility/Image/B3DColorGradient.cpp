//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Image/B3DColorGradient.h"
#include "RTTI/B3DColorGradientRTTI.h"
#include "Debug/B3DDebug.h"
#include "Utility/B3DBitwise.h"

using namespace b3d;

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

	const auto time = TGradientHelper<COLOR>::GetInternalTime(Math::Clamp01(t));

	if(time < mTimes[0])
		return mColors[0];

	// Note: Add a version of evaluate that supports caching?
	for(uint32_t keyIndex = 1; keyIndex < mNumKeys; keyIndex++)
	{
		const auto curKeyTime = mTimes[keyIndex];
		if(time > curKeyTime)
			continue;

		const auto prevKeyTime = mTimes[keyIndex - 1];
		const auto fracColor = TGradientHelper<COLOR>::InvLerp(prevKeyTime, curKeyTime, time);
		return Color::Lerp(fracColor, mColors[keyIndex - 1], mColors[keyIndex]);
	}

	return mColors[mNumKeys - 1];
}

template <class COLOR, class TIME>
void TColorGradient<COLOR, TIME>::SetKeys(const Vector<ColorGradientKey>& keys, float duration)
{
#if B3D_DEBUG
	// Ensure keys are sorted
	if(!keys.empty())
	{
		float time = keys[0].Time;
		for(u32 keyIndex = 1; keyIndex < (u32)keys.size(); keyIndex++)
		{
			B3D_ASSERT(keys[keyIndex].Time >= time);
			time = keys[keyIndex].Time;
		}
	}
#endif

	if(keys.size() > kMaxKeys)
	{
		B3D_LOG(Warning, LogGeneric, "Number of keys in ColorGradient exceeds the support number ({0}). "
								 "Keys will be ignored.",
			   kMaxKeys);
	}

	mDuration = duration;
	mNumKeys = 0;

	for(auto& key : keys)
	{
		if(mNumKeys >= kMaxKeys)
			break;

		mColors[mNumKeys] = TGradientHelper<COLOR>::ToInternalColor(key.Color);
		mTimes[mNumKeys] = TGradientHelper<COLOR>::GetInternalTime(Math::Clamp01(key.Time));

		mNumKeys++;
	}
}

template <class COLOR, class TIME>
Vector<ColorGradientKey> TColorGradient<COLOR, TIME>::GetKeys() const
{
	Vector<ColorGradientKey> output(mNumKeys);
	for(u32 keyIndex = 0; keyIndex < mNumKeys; keyIndex++)
	{
		output[keyIndex].Color = TGradientHelper<COLOR>::FromInternalColor(mColors[keyIndex]);
		output[keyIndex].Time = TGradientHelper<COLOR>::FromInternalTime(mTimes[keyIndex]);
	}

	return output;
}

template <class COLOR, class TIME>
ColorGradientKey TColorGradient<COLOR, TIME>::GetKey(u32 index) const
{
	if(index >= mNumKeys)
		return ColorGradientKey(Color::kBlack, 0.0f);

	return ColorGradientKey(
		TGradientHelper<COLOR>::FromInternalColor(mColors[index]),
		TGradientHelper<COLOR>::FromInternalTime(mTimes[index]));
}

template <class COLOR, class TIME>
void TColorGradient<COLOR, TIME>::SetConstant(const Color& color)
{
	mColors[0] = TGradientHelper<COLOR>::ToInternalColor(color);
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
		float time = TGradientHelper<COLOR>::FromInternalTime(mTimes[0]);
		return std::make_pair(time, time);
	}

	return std::make_pair(
		TGradientHelper<COLOR>::FromInternalTime(mTimes[0]),
		TGradientHelper<COLOR>::FromInternalTime(mTimes[mNumKeys - 1]));
}

template <class COLOR, class TIME>
bool TColorGradient<COLOR, TIME>::operator==(const TColorGradient<COLOR, TIME>& rhs) const
{
	if(mNumKeys != rhs.mNumKeys || mDuration != rhs.mDuration)
		return false;

	for(uint32_t keyIndex = 0; keyIndex < mNumKeys; keyIndex++)
	{
		if(mColors[keyIndex] != rhs.mColors[keyIndex] || mTimes[keyIndex] != rhs.mTimes[keyIndex])
			return false;
	}

	return true;
}

template class TColorGradient<RGBA, uint16_t>;
template class TColorGradient<Color, float>;
