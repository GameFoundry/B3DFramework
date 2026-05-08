//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Utility/B3DModule.h"
#include "GpuBackend/B3DRenderWindow.h"
#include "Utility/B3DEvent.h"

namespace b3d
{
	/** @addtogroup GpuBackend-Internal
	 *  @{
	 */

	/** Handles creation and internal updates relating to render windows. */
	class B3D_EXPORT RenderWindowManager : public Module<RenderWindowManager>
	{
	public:
		RenderWindowManager() = default;
		~RenderWindowManager() = default;

		/** Creates a new render window using the specified options. Optionally makes the created window a child of another window. */
		TShared<RenderWindow> CreateRenderWindow(const RenderWindowCreateInformation& createInformation, const TShared<RenderWindow>& parentWindow);

		/** Creates a render window surface that is appropriate for the currently active platform & GPU backend combination. Thread safe. */
		virtual TShared<render::IRenderWindowSurface> CreateRenderWindowSurface(const render::RenderWindowSurfaceCreateInformation& createInformation) = 0;

		/** Called once per frame. Dispatches events. */
		void Update();

		/** Called by the main thread when window is destroyed. */
		void NotifyWindowDestroyed(RenderWindow& window);

		/**	Called by the main thread when window receives focus. */
		void NotifyFocusReceived(RenderWindow& window);

		/**	Called by the main thread when window loses focus. */
		void NotifyFocusLost(RenderWindow& window);

		/**	Called by the main thread when mouse leaves a window. */
		void NotifyMouseLeft(RenderWindow& window);

		/** Requests a window to be shown or hidden. Expected to be called from non-main thread, otherwise you can call this directly on the render window. */
		void RequestShowWindow(u32 windowId, bool show);

		/**	Returns a list of all open render windows. */
		Vector<RenderWindow*> GetRenderWindows() const;

		/** Returns the window that is currently the top-most modal window. Returns null if no modal windows are active. */
		RenderWindow* GetTopMostModal() const;

		/** Event that is triggered when a window gains focus. */
		Event<void(RenderWindow&)> OnFocusGained;

		/**	Event that is triggered when a window loses focus. */
		Event<void(RenderWindow&)> OnFocusLost;

		/**	Event that is triggered when mouse leaves a window. */
		Event<void(RenderWindow&)> OnMouseLeftWindow;

	protected:
		friend class RenderWindow;

	protected:
		Map<u32, RenderWindow*> mWindows;
		Vector<RenderWindow*> mModalWindowStack;

		RenderWindow* mWindowInFocus = nullptr;

		u32 mNextWindowId = 0;
	};

	/** @} */
} // namespace b3d
