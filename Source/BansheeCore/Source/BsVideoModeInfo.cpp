//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsVideoModeInfo.h"

namespace bs
{
	VideoMode::VideoMode(UINT32 width, UINT32 height, float refreshRate, UINT32 outputIdx)
		:mWidth(width), mHeight(height), mRefreshRate(refreshRate), mOutputIdx(outputIdx), mIsCustom(true)
	{

	}

	VideoMode::~VideoMode()
	{ }

	bool VideoMode::operator== (const VideoMode& other) const
	{
		return mWidth == other.mWidth && mHeight == other.mHeight && 
			mOutputIdx == other.mOutputIdx && mRefreshRate == other.mRefreshRate;
	}

	VideoOutputInfo::~VideoOutputInfo()
	{ 
		for (auto& videoMode : mVideoModes)
			bs_delete(videoMode);

		if (mDesktopVideoMode != nullptr)
			bs_delete(mDesktopVideoMode);
	}

	VideoModeInfo::~VideoModeInfo()
	{ 
		for (auto& output : mOutputs)
			bs_delete(output);
	}
}