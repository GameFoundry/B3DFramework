//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "GpuBackend/B3DRenderWindow.h"
#include <X11/extensions/Xrandr.h>

namespace b3d
{
	class LinuxWindow;

	namespace render
	{
		class LinuxRenderWindow;
	}

	/** @addtogroup Vulkan
	 *  @{
	 */

	/** Render window implementation for Linux using Xlib. */
	class B3D_EXPORT LinuxRenderWindow : public RenderWindow
	{
		using Super = RenderWindow;

	public:
		LinuxRenderWindow(const RenderWindowCreateInformation& createInformation, u32 windowId, const TShared<RenderWindow>& parentWindow);

		void Initialize() override;
		void Destroy() override;

		Vector2I ScreenToWindowPosition(const Vector2I& screenPosition) const override;
		Vector2I WindowToScreenPosition(const Vector2I& windowPosition) const override;
		void Move(i32 left, i32 top) override;
		void Resize(u32 width, u32 height) override;
		void Hide() override;
		void Show() override;
		void Minimize() override;
		void Maximize() override;
		void Restore() override;
		void SetFullscreen(u32 width, u32 height, float refreshRate = 60.0f, u32 monitorIdx = 0) override;
		void SetFullscreen(const VideoMode& mode) override;
		void SetWindowed(u32 width, u32 height) override;
		void SetVSync(bool enabled, u32 interval = 1) override;

		u64 GetPlatformWindowHandle() const override;

	protected:
		friend class render::LinuxRenderWindow;

		/** Changes the video mode to the specified RandR mode on the specified output device. */
		void SetVideoMode(i32 screen, RROutput output, RRMode mode);

		TShared<render::RenderProxy> CreateRenderProxy() const override;
		void DoOnWindowMovedOrResized() override;

	private:
		LinuxWindow* mWindow = nullptr;
		bool mIsChild = false;
		i32 mDisplayFrequency = 0;
	};

	namespace render
	{
		/** Render thread proxy for b3d::LinuxRenderWindow. */
		class B3D_EXPORT LinuxRenderWindow : public RenderWindow
		{
			using Super = RenderWindow;

		public:
			LinuxRenderWindow(const RenderWindowCreateInformation& createInformation, u32 windowId, u64 x11WindowHandle, const TShared<RenderWindow>& parentWindow);
		};
	} // namespace render

	/** @} */
} // namespace b3d
