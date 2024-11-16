//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "RenderAPI/BsRenderWindow.h"
#include <X11/extensions/Xrandr.h>

namespace bs
{
	class LinuxWindow;
	class LinuxRenderWindow;

	namespace ct
	{
		class LinuxRenderWindow;
	}

	/** @addtogroup Vulkan
	 *  @{
	 */

	/** Render window implementation for Linux. */
	class B3D_CORE_EXPORT LinuxRenderWindow : public RenderWindow
	{
	public:
		LinuxRenderWindow(const RenderWindowCreateInformation& createInformation, u32 windowId, const SPtr<RenderWindow>& parentWindow);
		~LinuxRenderWindow() override;

		void Initialize() override;

		Vector2I ScreenToWindowPosition(const Vector2I& screenPos) const override;
		Vector2I WindowToScreenPosition(const Vector2I& windowPos) const override;
		void Move(i32 left, i32 top) override;
		void Resize(u32 width, u32 height) override;
		void Hide() override;
		void Show() override;
		void Minimize() override;
		void Maximize() override;
		void Restore() override;
		void SetFullscreen(u32 width, u32 height, float refreshRate = 60.0f, u32 monitorIdx = 0) override;
		void SetFullscreen(const VideoMode& mode);
		void SetWindowed(u32 width, u32 height) override;
		void SetVSync(bool enabled, u32 interval = 1) override;

		u64 GetPlatformWindowHandle() const override;

	protected:
		friend class ct::LinuxRenderWindow;

		/** Changes the video mode to the specified RandR mode on the specified output device. */
		void SetVideoMode(i32 screen, RROutput output, RRMode mode);

		SPtr<ct::RenderProxy> CreateRenderProxy() const override;
		void DoOnWindowMovedOrResized() override;

	private:
		LinuxWindow* mWindow = nullptr;
		bool mIsChild = false;
		i32 mDisplayFrequency = 0;
	};

	namespace ct
	{
		/** Render thread proxy for bs::LinuxRenderWindow. */
		class B3D_CORE_EXPORT LinuxRenderWindow : public RenderWindow
		{
		public:
			LinuxRenderWindow(const RenderWindowCreateInformation& createInformation, u32 windowId, u64 x11WindowHandle, const SPtr<RenderWindow>& parentWindow);

			void SwapBuffers(u32 syncMask = 0xFFFFFFFF) override;
		};
	} // namespace ct

	/** @} */
} // namespace bs
