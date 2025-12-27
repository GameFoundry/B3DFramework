//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "RenderAPI/B3DRenderWindow.h"

namespace b3d
{
	class CocoaWindow;

	namespace render
	{
		class MacOSRenderWindow;
	}

	/** @addtogroup Vulkan
	 *  @{
	 */

	/** Render window implementation for MacOS using Cocoa. */
	class B3D_EXPORT MacOSRenderWindow : public RenderWindow
	{
	public:
		MacOSRenderWindow(const RenderWindowCreateInformation& createInformation, u32 windowId, const SPtr<RenderWindow>& parentWindow);

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
		void SetFullscreen(const VideoMode& mode);
		void SetWindowed(u32 width, u32 height) override;
		void SetVSync(bool enabled, u32 interval = 1) override;

		u64 GetPlatformWindowHandle() const override;

	protected:
		friend class render::MacOSRenderWindow;

		SPtr<render::RenderProxy> CreateRenderProxy() const override;
		void DoOnWindowMovedOrResized() override;

		/** Changes the display mode (resolution, refresh rate) of the specified output device. */
		void SetDisplayMode(const VideoOutputInfo& output, const VideoMode& mode);

	private:
		CocoaWindow* mWindow = nullptr;
		bool mIsChild = false;
	};

	namespace render
	{
		/** Render thread proxy for b3d::MacOSRenderWindow. */
		class MacOSRenderWindow : public RenderWindow
		{
		public:
			MacOSRenderWindow(const RenderWindowCreateInformation& createInformation, u32 windowId, u64 platformWindowHandle, const SPtr<RenderWindow>& parentWindow);

		protected:
			friend class b3d::MacOSRenderWindow;
		};
	} // namespace render

	/** @} */
} // namespace b3d
