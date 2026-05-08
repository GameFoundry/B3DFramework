//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "GpuBackend/B3DRenderWindow.h"

namespace b3d
{
	/** @addtogroup GpuBackend-Internal
	 *  @{
	 */

	/**
	 * A headless render window that doesn't create an OS window.
	 * Used for headless rendering in automated testing or offscreen rendering scenarios.
	 * All window-handle-dependent operations become no-ops.
	 */
	class B3D_EXPORT HeadlessRenderWindow : public RenderWindow
	{
		using Super = RenderWindow;

	public:
		HeadlessRenderWindow(const RenderWindowCreateInformation& createInformation, u32 windowId, const TShared<RenderWindow>& parentWindow);

		void Initialize() override;
		void Destroy() override;

		Vector2I ScreenToWindowPosition(const Vector2I& screenPosition) const override { return screenPosition; }
		Vector2I WindowToScreenPosition(const Vector2I& windowPosition) const override { return windowPosition; }

		void Resize(u32 width, u32 height) override;
		void Move(i32 left, i32 top) override;
		void Hide() override;
		void Show() override;
		void Minimize() override;
		void Maximize() override;
		void Restore() override;
		void SetFullscreen(u32 width, u32 height, float refreshRate = 60.0f, u32 monitorIndex = 0) override;
		void SetWindowed(u32 width, u32 height) override;
		void SetVSync(bool enabled, u32 interval = 1) override;

		/** Returns 0 since there is no platform window. */
		u64 GetPlatformWindowHandle() const override { return 0; }

	protected:
		TShared<render::RenderProxy> CreateRenderProxy() const override;
	};

	/** @} */

	namespace render
	{
		/** @addtogroup GpuBackend-Internal
		 *  @{
		 */

		/** Render thread counterpart of b3d::HeadlessRenderWindow. */
		class B3D_EXPORT HeadlessRenderWindow : public RenderWindow
		{
			using Super = RenderWindow;

		public:
			HeadlessRenderWindow(const RenderWindowCreateInformation& createInformation, u32 windowId, const TShared<RenderWindow>& parentWindow);
		};

		/** @} */
	} // namespace render
} // namespace b3d
