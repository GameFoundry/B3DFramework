//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"

namespace b3d
{
	/** @addtogroup RenderAPI
	 *  @{
	 */

	/**
	 * Video mode contains information about how a render window presents its information to an output device like a
	 * monitor.
	 */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(RenderAPI), ExportAsStruct(true), API(Framework)) VideoMode
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
		VideoMode(u32 width, u32 height, float refreshRate = 60.0f, u32 outputIdx = 0)
			: Width(width), Height(height), RefreshRate(refreshRate), OutputIdx(outputIdx)
		{}

		virtual ~VideoMode() = default;

		bool operator==(const VideoMode& other) const;

		/**	Width of the front/back buffer in pixels. */
		u32 Width = 1280;

		/**	Height of the front/back buffer in pixels. */
		u32 Height = 720;

		/**	Refresh rate in hertz. */
		float RefreshRate = 60.0f;

		/**	Index of the parent video output. */
		u32 OutputIdx = 0;

		/**
		 * Determines was video mode user created or provided by the API/OS. API/OS created video modes can contain
		 * additional information that allows the video mode to be used more accurately and you should use them when possible.
		 */
		bool IsCustom = true;
	};

	/** Contains information about a video output device, including a list of all available video modes. */
	class B3D_EXPORT VideoOutputInfo
	{
	public:
		VideoOutputInfo() = default;
		virtual ~VideoOutputInfo();

		VideoOutputInfo(const VideoOutputInfo&) = delete; // Make non-copyable
		VideoOutputInfo& operator=(const VideoOutputInfo&) = delete; // Make non-copyable

		/**	Name of the output device. */
		const String& GetName() const { return mName; }

		/**	Number of available video modes for this output. */
		u32 GetVideoModeCount() const { return (u32)mVideoModes.size(); }

		/**	Returns video mode at the specified index. */
		const VideoMode& GetVideoMode(u32 idx) const { return *mVideoModes.at(idx); }

		/**	Returns the video mode currently used by the desktop. */
		const VideoMode& GetDesktopVideoMode() const { return *mDesktopVideoMode; }

	protected:
		String mName;
		Vector<VideoMode*> mVideoModes;
		VideoMode* mDesktopVideoMode = nullptr;
	};

	/** Contains information about available output devices (for example monitor) and their video modes. */
	class B3D_EXPORT VideoModeInfo
	{
	public:
		VideoModeInfo() = default;
		virtual ~VideoModeInfo();

		VideoModeInfo(const VideoModeInfo&) = delete; // Make non-copyable
		VideoModeInfo& operator=(const VideoModeInfo&) = delete; // Make non-copyable

		/**	Returns the number of available output devices. */
		u32 GetOutputCount() const { return (u32)mOutputs.size(); }

		/**
		 * Returns video mode information about a specific output device. 0th index always represents the primary device
		 * while order of others is undefined.
		 */
		const VideoOutputInfo& GetOutputInfo(u32 idx) const { return *mOutputs[idx]; }

	protected:
		Vector<VideoOutputInfo*> mOutputs;
	};

	/** @} */
} // namespace b3d
