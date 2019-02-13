//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RenderAPI/BsVideoModeInfo.h"

namespace bs
{
	bool VideoMode::operator== (const VideoMode& other) const
	{
		return width == other.width && height == other.height && 
			outputIdx == other.outputIdx && refreshRate == other.refreshRate;
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