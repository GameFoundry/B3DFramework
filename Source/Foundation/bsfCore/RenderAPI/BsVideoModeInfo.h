//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"

namespace bs
{
	/** @addtogroup RenderAPI
	 *  @{
	 */

	/**
	 * Video mode contains information about how a render window presents its information to an output device like a 
	 * monitor.
	 */
	class BS_CORE_EXPORT BS_SCRIPT_EXPORT(m:RenderAPI,pl:true,api:bsf) VideoMode
	{
	public:
		VideoMode() = default;

		/**
		 * Creates a new video mode.
		 *
		 * @param[in]	width		Width of the frame buffer in pixels.
		 * @param[in]	height		Height of the frame buffer in pixels.
		 * @param[in]	refreshRate	How often should the output device refresh the output image in hertz.
		 * @param[in]	outputIdx	Output index of the output device. Normally this means output monitor. 0th index always
		 *							represents the primary device while order of others is undefined.
		 */
		VideoMode(UINT32 width, UINT32 height, float refreshRate = 60.0f, UINT32 outputIdx = 0)
			:width(width), height(height), refreshRate(refreshRate), outputIdx(outputIdx)
		{ }
		virtual ~VideoMode() = default;

		bool operator== (const VideoMode& other) const;

		/**	Width of the front/back buffer in pixels. */
		UINT32 width = 1280;

		/**	Height of the front/back buffer in pixels. */
		UINT32 height = 720;

		/**	Refresh rate in hertz. */
		float refreshRate = 60.0f;

		/**	Index of the parent video output. */
		UINT32 outputIdx = 0;

		/**
		 * Determines was video mode user created or provided by the API/OS. API/OS created video modes can contain 
		 * additional information that allows the video mode to be used more accurately and you should use them when possible.
		 */
		bool isCustom = true;
	};

	/** Contains information about a video output device, including a list of all available video modes. */
	class BS_CORE_EXPORT VideoOutputInfo
	{
	public:
		VideoOutputInfo() = default;
		virtual ~VideoOutputInfo();

		VideoOutputInfo(const VideoOutputInfo&) = delete; // Make non-copyable
		VideoOutputInfo& operator=(const VideoOutputInfo&) = delete; // Make non-copyable

		/**	Name of the output device. */
		const String& getName() const { return mName; }

		/**	Number of available video modes for this output. */
		UINT32 getNumVideoModes() const { return (UINT32)mVideoModes.size(); }

		/**	Returns video mode at the specified index. */
		const VideoMode& getVideoMode(UINT32 idx) const { return *mVideoModes.at(idx); }

		/**	Returns the video mode currently used by the desktop. */
		const VideoMode& getDesktopVideoMode() const { return *mDesktopVideoMode; }

	protected:
		String mName;
		Vector<VideoMode*> mVideoModes;
		VideoMode* mDesktopVideoMode = nullptr;
	};

	/** Contains information about available output devices (for example monitor) and their video modes. */
	class BS_CORE_EXPORT VideoModeInfo
	{
	public:
		VideoModeInfo() = default;
		virtual ~VideoModeInfo();

		VideoModeInfo(const VideoModeInfo&) = delete; // Make non-copyable
		VideoModeInfo& operator=(const VideoModeInfo&) = delete; // Make non-copyable

		/**	Returns the number of available output devices. */
		UINT32 getNumOutputs() const { return (UINT32)mOutputs.size(); }

		/**
		 * Returns video mode information about a specific output device. 0th index always represents the primary device 
		 * while order of others is undefined.
		 */
		const VideoOutputInfo& getOutputInfo(UINT32 idx) const { return *mOutputs[idx]; }

	protected:
		Vector<VideoOutputInfo*> mOutputs;
	};

	/** @} */
}