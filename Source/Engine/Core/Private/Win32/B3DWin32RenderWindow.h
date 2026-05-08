//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "GpuBackend/B3DRenderWindow.h"

namespace b3d
{
	namespace render
	{
		class Win32RenderWindow;
	}

	/** @addtogroup Vulkan
	 *  @{
	 */

	/** Render window implementation for Windows using Win32 API. */
	class B3D_EXPORT Win32RenderWindow : public RenderWindow
	{
		using Super = RenderWindow;
	public:
		Win32RenderWindow(const RenderWindowCreateInformation& createInformation, u32 windowId, const TShared<RenderWindow>& parentWindow);

		void Initialize() override;
		void Destroy() override;

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
		void SetWindowed(u32 width, u32 height) override;
		void SetVSync(bool enabled, u32 interval = 1) override;

		u64 GetPlatformWindowHandle() const override;

	protected:
		friend class render::Win32RenderWindow;

		TShared<render::RenderProxy> CreateRenderProxy() const override;

		void DoOnWindowMovedOrResized() override;
		void DoOnDPIScaleChanged() override;

	private:
		Win32Window* mWindow = nullptr;
		bool mIsChild = false;
		i32 mDisplayFrequency = 0;
	};

	namespace render
	{
		/** Render thread proxy for b3d::Win32RenderWindow. */
		class B3D_EXPORT Win32RenderWindow : public RenderWindow
		{
			using Super = RenderWindow;
		public:
			Win32RenderWindow(const RenderWindowCreateInformation& createInformation, u32 windowId, u64 hWnd, const TShared<RenderWindow>& parentWindow);

		protected:
			friend class b3d::Win32RenderWindow;
		};
	} // namespace render

	/** @} */
} // namespace b3d
