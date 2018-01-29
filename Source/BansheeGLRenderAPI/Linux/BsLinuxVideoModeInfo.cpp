//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Linux/BsLinuxVideoModeInfo.h"
#include "Private/Linux/BsLinuxPlatform.h"
#include <X11/extensions/Xrandr.h>

#define XRANDR_ROTATION_LEFT    (1 << 1)
#define XRANDR_ROTATION_RIGHT   (1 << 3)

namespace bs { namespace ct
{
	LinuxVideoModeInfo::LinuxVideoModeInfo()
	{
		LinuxPlatform::lockX();

		::Display* display = LinuxPlatform::getXDisplay();

		INT32 minor, major;
		XRRQueryVersion(display, &minor, &major);

		INT32 defaultScreen = XDefaultScreen(display);
		RROutput primaryOutput = XRRGetOutputPrimary(display, RootWindow(display, defaultScreen));

		INT32 screenCount = XScreenCount(display);
		for(INT32 i = 0; i < screenCount; i++)
		{
			XRRScreenResources* screenRes = XRRGetScreenResources(display, RootWindow(display, i));

			for(INT32 j = 0; j < screenRes->noutput; j++)
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

				VideoOutputInfo* output = bs_new<LinuxVideoOutputInfo>(display, i, outputInfo, crtcInfo, screenRes,
						screenRes->outputs[j], (UINT32)mOutputs.size());

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

		LinuxPlatform::unlockX();
	}

	LinuxVideoOutputInfo::LinuxVideoOutputInfo(::Display* x11Display, INT32 screen, XRROutputInfo* outputInfo,
		XRRCrtcInfo* crtcInfo, XRRScreenResources* screenRes, RROutput outputID, UINT32 outputIdx)
			: mOutputID(outputID), mScreen(screen)
	{
		RRMode currentMode = crtcInfo->mode;

		// Parse output name
		Atom EDID = XInternAtom(x11Display, "EDID", False);

		INT32 numOutputProps;
		Atom* outputProps = XRRListOutputProperties(x11Display, mOutputID, &numOutputProps);

		for(INT32 k = 0; k < numOutputProps; k++)
		{
			if(outputProps[k] != EDID)
				continue;

			Atom actualType;
			unsigned long numItems, bytesAfter;
			INT32 actualFormat;
			UINT8* data;

			Status status = XRRGetOutputProperty(x11Display, mOutputID, outputProps[k], 0, 100, False,
					False, AnyPropertyType, &actualType, &actualFormat, &numItems, &bytesAfter, &data);
			if(status == Success)
			{
				// Decode EDID to get the name
				for(UINT32 l = 0; l < 4; l++)
				{
					INT32 idx = 0x36 + l * 18;
					if(data[idx] == 0 && data[idx + 1] == 0 && data[idx + 3] == 0xFC)
					{
						UINT8* nameSrc = &data[idx + 5];

						char name[13];
						for(UINT32 m = 0; m < 13; m++)
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

						mName = String(name);
					}
				}

				continue;
			}

			XFree(data);
			break;
		}

		XFree(outputProps);

		// Use the output name if display name cannot be found
		if(mName.empty())
			mName = outputInfo->name;

		// Enumerate all valid resolutions
		for(INT32 k = 0; k < screenRes->nmode; k++)
		{
			const XRRModeInfo& modeInfo = screenRes->modes[k];

			UINT32 width, height;

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

			LinuxVideoMode* videoMode = new (bs_alloc<LinuxVideoMode>())
					LinuxVideoMode(width, height, refreshRate, outputIdx, modeInfo.id);
			mVideoModes.push_back(videoMode);
		}

		// Save current desktop mode
		for(INT32 k = 0; k < screenRes->nmode; k++)
		{
			if(screenRes->modes[k].id == currentMode)
			{
				mDesktopVideoMode = new (bs_alloc<LinuxVideoMode>())
						LinuxVideoMode(mVideoModes[k]->getWidth(), mVideoModes[k]->getHeight(),
						mVideoModes[k]->getRefreshRate(), mVideoModes[k]->getOutputIdx(), currentMode);
				break;
			}
		}
	}

	LinuxVideoMode::LinuxVideoMode(UINT32 width, UINT32 height, float refreshRate, UINT32 outputIdx)
		:VideoMode(width, height, refreshRate, outputIdx), mModeID((RRMode)-1)
	{ }

	LinuxVideoMode::LinuxVideoMode(UINT32 width, UINT32 height, float refreshRate, UINT32 outputIdx, RRMode modeID)
		:VideoMode(width, height, refreshRate, outputIdx), mModeID(modeID)
	{
		mIsCustom = false;
	}
}}
