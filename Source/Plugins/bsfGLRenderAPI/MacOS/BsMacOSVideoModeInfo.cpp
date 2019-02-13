//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "MacOS/BsMacOSVideoModeInfo.h"
#include <IOKit/graphics/IOGraphicsLib.h>

namespace bs::ct
{
	MacOSVideoModeInfo::MacOSVideoModeInfo()
	{
		VideoModeInfo info;

		CGDisplayCount numDisplays;
		CGGetOnlineDisplayList(0, nullptr, &numDisplays);

		auto displays = (CGDirectDisplayID*)bs_stack_alloc(sizeof(CGDirectDisplayID) * numDisplays);
		CGGetOnlineDisplayList(numDisplays, displays, &numDisplays);

		for(UINT32 i = 0; i < numDisplays; i++)
		{
			if(CGDisplayMirrorsDisplay(displays[i]) != kCGNullDirectDisplay)
				continue;

			CGDisplayModeRef modeRef = CGDisplayCopyDisplayMode(displays[i]);
			if(!modeRef)
				continue;

			VideoOutputInfo* output = bs_new<MacOSVideoOutputInfo>(displays[i], i);

			// Make sure the primary output is the first in the output list
			if(CGDisplayIsMain(displays[i]))
				mOutputs.insert(mOutputs.begin(), output);
			else
				mOutputs.push_back(output);
		}
		bs_stack_free(displays);
	}

	MacOSVideoOutputInfo::MacOSVideoOutputInfo(CGDirectDisplayID displayID, UINT32 outputIdx)
		:mDisplayID(displayID)
	{
		CGDisplayModeRef desktopModeRef = CGDisplayCopyDisplayMode(displayID);

		CVDisplayLinkRef linkRef = nullptr;
		CVDisplayLinkCreateWithCGDisplay(displayID, &linkRef);

		io_service_t service = CGDisplayIOServicePort(displayID);
		CFDictionaryRef deviceInfo = IODisplayCreateInfoDictionary(service, kIODisplayOnlyPreferredName);
		auto locNames = (CFDictionaryRef)CFDictionaryGetValue(deviceInfo, CFSTR(kDisplayProductName));

		CFIndex numNames = CFDictionaryGetCount(locNames);
		if(numNames > 0)
		{
			auto keys = (CFStringRef*)bs_stack_alloc(numNames * sizeof(CFTypeRef));
			CFDictionaryGetKeysAndValues(locNames, (const void**)keys, nullptr);

			auto value = (CFStringRef)CFDictionaryGetValue(locNames, keys[0]);
			if(value)
			{
				const char* chars = CFStringGetCStringPtr(value, kCFStringEncodingUTF8);
				if(chars)
					mName = chars;
				else
				{
					CFIndex stringLength = CFStringGetLength(value) + 1;
					auto buffer = bs_stack_alloc<char>((UINT32)stringLength);

					CFStringGetCString(value, buffer, stringLength, kCFStringEncodingUTF8);

					mName = buffer;
					bs_stack_free(buffer);
				}
			}
			else
				mName = "Unknown";

			bs_stack_free(keys);
		}

		CFRelease(deviceInfo);

		mDesktopVideoMode = new (bs_alloc<MacOSVideoMode>()) MacOSVideoMode(desktopModeRef, linkRef, outputIdx);

		CFArrayRef modes = CGDisplayCopyAllDisplayModes(displayID, nullptr);
		if(modes)
		{
			CFIndex count = CFArrayGetCount(modes);
			for(int i = 0; i < count; i++)
			{
				auto modeRef = (CGDisplayModeRef) CFArrayGetValueAtIndex(modes, i);
				CGDisplayModeRetain(modeRef);

				VideoMode* videoMode = new (bs_alloc<MacOSVideoMode>()) MacOSVideoMode(modeRef, linkRef, outputIdx);

				mVideoModes.push_back(videoMode);
			}

			CFRelease(modes);
		}
	}

	MacOSVideoMode::MacOSVideoMode(UINT32 width, UINT32 height, float refreshRate, UINT32 outputIdx)
			:VideoMode(width, height, refreshRate, outputIdx), mModeRef(nullptr)
	{ }

	MacOSVideoMode::MacOSVideoMode(CGDisplayModeRef mode, CVDisplayLinkRef linkRef, UINT32 outputIdx)
			:VideoMode(0, 0, 0.0f, outputIdx), mModeRef(mode)
	{
		width = (UINT32)CGDisplayModeGetPixelWidth(mModeRef);
		height = (UINT32)CGDisplayModeGetPixelHeight(mModeRef);

		refreshRate = (float)CGDisplayModeGetRefreshRate(mModeRef);
		if(refreshRate == 0.0f && linkRef != nullptr)
		{
			CVTime time = CVDisplayLinkGetNominalOutputVideoRefreshPeriod(linkRef);
			if((time.flags & kCVTimeIsIndefinite) == 0 && time.timeValue != 0)
				refreshRate = time.timeScale / (float)time.timeValue;
		}
		isCustom = false;
	}

	MacOSVideoMode::~MacOSVideoMode()
	{
		if(!isCustom && mModeRef)
			CGDisplayModeRelease(mModeRef);
	}
}

