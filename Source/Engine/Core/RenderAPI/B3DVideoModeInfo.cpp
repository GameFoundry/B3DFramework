//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RenderAPI/B3DVideoModeInfo.h"

using namespace b3d;

bool VideoMode::operator==(const VideoMode& other) const
{
	return Width == other.Width && Height == other.Height &&
		OutputIdx == other.OutputIdx && RefreshRate == other.RefreshRate;
}

VideoOutputInfo::~VideoOutputInfo()
{
	for(auto& videoMode : mVideoModes)
		B3DDelete(videoMode);

	if(mDesktopVideoMode != nullptr)
		B3DDelete(mDesktopVideoMode);
}

VideoModeInfo::~VideoModeInfo()
{
	for(auto& output : mOutputs)
		B3DDelete(output);
}
