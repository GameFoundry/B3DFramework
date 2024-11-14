//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "RenderAPI/BsRenderWindow.h"

namespace bs
{
	namespace ct
	{
		class Win32RenderWindow;
	}

	/** @addtogroup Vulkan
	 *  @{
	 */

	/** Render window implementation for Windows using Win32 API. */
	class B3D_CORE_EXPORT Win32RenderWindow : public RenderWindow
	{
		using Super = RenderWindow;
	public:
		Win32RenderWindow(const RenderWindowCreateInformation& createInformation, u32 windowId, const SPtr<RenderWindow>& parentWindow);
		~Win32RenderWindow() override;

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
		void SetWindowed(u32 width, u32 height) override;
		void SetVSync(bool enabled, u32 interval = 1) override;

		u64 GetPlatformWindowHandle() const override;

	protected:
		friend class VulkanRenderWindowManager;
		friend class ct::Win32RenderWindow;

		SPtr<ct::RenderProxy> CreateRenderProxy() const override;

		void DoOnWindowMovedOrResized() override;

	private:
		Win32Window* mWindow = nullptr;
		bool mIsChild = false;
		i32 mDisplayFrequency = 0;
	};

	namespace ct
	{
		/** Render thread proxy for bs::Win32RenderWindow. */
		class B3D_CORE_EXPORT Win32RenderWindow : public RenderWindow
		{
			using Super = RenderWindow;
		public:
			Win32RenderWindow(const RenderWindowCreateInformation& createInformation, u32 windowId, u64 hWnd, const SPtr<RenderWindow>& parentWindow);

			void SwapBuffers(u32 syncMask = 0xFFFFFFFF) override;

		protected:
			friend class bs::Win32RenderWindow;
		};
	} // namespace ct

	/** @} */
} // namespace bs
