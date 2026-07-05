//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Private/Linux/B3DLinuxVideoModeInfo.h"
#include "Private/Linux/B3DLinuxPlatform.h"
#include <X11/extensions/Xrandr.h>

#define XRANDR_ROTATION_LEFT (1 << 1)
#define XRANDR_ROTATION_RIGHT (1 << 3)

using namespace b3d;
using namespace b3d::render;

LinuxVideoModeInfo::LinuxVideoModeInfo()
{
	LinuxPlatform::LockX();

	::Display* display = LinuxPlatform::GetXDisplay();

	// XRRGetOutputPrimary (used below and by the windowing code) requires RandR 1.3.
	i32 major = 0;
	i32 minor = 0;
	if(!XRRQueryVersion(display, &major, &minor) || major < 1 || (major == 1 && minor < 3))
	{
		B3D_LOG(Error, LogPlatform, "XRandR 1.3 or newer is required for video mode enumeration (found {0}.{1}).", major, minor);
		LinuxPlatform::UnlockX();
		return;
	}

	i32 defaultScreen = XDefaultScreen(display);
	RROutput primaryOutput = XRRGetOutputPrimary(display, RootWindow(display, defaultScreen));

	i32 screenCount = XScreenCount(display);
	for(i32 i = 0; i < screenCount; i++)
	{
		XRRScreenResources* screenRes = XRRGetScreenResources(display, RootWindow(display, i));

		for(i32 j = 0; j < screenRes->noutput; j++)
		{
			XRROutputInfo* outputInfo = XRRGetOutputInfo(display, screenRes, screenRes->outputs[j]);

			if(outputInfo == nullptr || outputInfo->crtc == 0 || outputInfo->connection == RR_Disconnected)
			{
				XRRFreeOutputInfo(outputInfo);
				continue;
			}

			XRRCrtcInfo* crtcInfo = XRRGetCrtcInfo(display, screenRes, outputInfo->crtc);
			if(crtcInfo == nullptr)
			{
				XRRFreeCrtcInfo(crtcInfo);
				XRRFreeOutputInfo(outputInfo);
				continue;
			}

			VideoOutputInfo* output = B3DNew<LinuxVideoOutputInfo>(display, i, outputInfo, crtcInfo, screenRes, screenRes->outputs[j], (u32)mOutputs.size());

			// Make sure the primary output is the first in the output list
			if(i == defaultScreen && screenRes->outputs[j] == primaryOutput)
				mOutputs.insert(mOutputs.begin(), output);
			else
				mOutputs.push_back(output);

			XRRFreeCrtcInfo(crtcInfo);
			XRRFreeOutputInfo(outputInfo);
		}

		XRRFreeScreenResources(screenRes);
	}

	LinuxPlatform::UnlockX();
}

LinuxVideoOutputInfo::LinuxVideoOutputInfo(::Display* x11Display, i32 screen, XRROutputInfo* outputInfo, XRRCrtcInfo* crtcInfo, XRRScreenResources* screenRes, RROutput outputID, u32 outputIdx)
	: mOutputID(outputID), mScreen(screen)
{
	RRMode currentMode = crtcInfo->mode;

	// Parse output name
	Atom EDID = XInternAtom(x11Display, "EDID", False);

	i32 numOutputProps;
	Atom* outputProps = XRRListOutputProperties(x11Display, mOutputID, &numOutputProps);

	for(i32 k = 0; k < numOutputProps; k++)
	{
		if(outputProps[k] != EDID)
			continue;

		Atom actualType;
		unsigned long numItems, bytesAfter;
		i32 actualFormat;
		u8* data;

		Status status = XRRGetOutputProperty(x11Display, mOutputID, outputProps[k], 0, 128, False, False, AnyPropertyType, &actualType, &actualFormat, &numItems, &bytesAfter, &data);
		if(status == Success)
		{
			// Decode EDID to get the name
			for(u32 l = 0; l < 4; l++)
			{
				i32 idx = 0x36 + l * 18;
				if(data[idx] == 0 && data[idx + 1] == 0 && data[idx + 3] == 0xFC)
				{
					u8* nameSrc = &data[idx + 5];

					char name[14];
					for(u32 m = 0; m < 13; m++)
					{
						if(nameSrc[m] == 0x0a)
						{
							name[m] = '\0';
							break;
						}
						else if(nameSrc[m] == 0x00)
							name[m] = ' ';
						else
							name[m] = nameSrc[m];
					}

					name[13] = '\0';
					mName = String(name);
				}
			}

			XFree(data);
		}

		break; // EDID property handled (or failed); no other properties are relevant.
	}

	XFree(outputProps);

	// Use the output name if display name cannot be found
	if(mName.empty())
		mName = outputInfo->name;

	// Enumerate all valid resolutions
	for(i32 k = 0; k < screenRes->nmode; k++)
	{
		const XRRModeInfo& modeInfo = screenRes->modes[k];

		u32 width, height;

		if(crtcInfo->rotation & (XRANDR_ROTATION_LEFT | XRANDR_ROTATION_RIGHT))
		{
			width = modeInfo.height;
			height = modeInfo.width;
		}
		else
		{
			width = modeInfo.width;
			height = modeInfo.height;
		}

		float refreshRate;
		if(modeInfo.hTotal != 0 && modeInfo.vTotal != 0)
			refreshRate = (float)(modeInfo.dotClock / (double)(modeInfo.hTotal * modeInfo.vTotal));
		else
			refreshRate = 0.0f;

		LinuxVideoMode* videoMode = new(B3DAllocate<LinuxVideoMode>())
			LinuxVideoMode(width, height, refreshRate, outputIdx, modeInfo.id);
		mVideoModes.push_back(videoMode);
	}

	// Save current desktop mode
	for(i32 k = 0; k < screenRes->nmode; k++)
	{
		if(screenRes->modes[k].id == currentMode)
		{
			mDesktopVideoMode = new(B3DAllocate<LinuxVideoMode>())
				LinuxVideoMode(mVideoModes[k]->Width, mVideoModes[k]->Height, mVideoModes[k]->RefreshRate, mVideoModes[k]->OutputIdx, currentMode);
			break;
		}
	}
}

LinuxVideoMode::LinuxVideoMode(u32 width, u32 height, float refreshRate, u32 outputIdx)
	: VideoMode(width, height, refreshRate, outputIdx), mModeID((RRMode)-1)
{}

LinuxVideoMode::LinuxVideoMode(u32 width, u32 height, float refreshRate, u32 outputIdx, RRMode modeID)
	: VideoMode(width, height, refreshRate, outputIdx), mModeID(modeID)
{
	IsCustom = false;
}
