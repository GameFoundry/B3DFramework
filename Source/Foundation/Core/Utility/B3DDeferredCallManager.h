//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Utility/B3DModule.h"

namespace b3d
{
	/** @addtogroup Utility-Engine-Internal
	 *  @{
	 */

	/**
	 * Allows you to queue calls that can get executed later.
	 *
	 * @note	Main thread only.
	 */
	class B3D_EXPORT DeferredCallManager : public Module<DeferredCallManager>
	{
	public:
		DeferredCallManager() = default;

		/**
		 * Register a deferred call that will be executed once at the start of next frame.
		 *
		 * @param[in]	func	The function to execute.
		 */
		void QueueDeferredCall(std::function<void()> func);

		/** Executes all the scheduled calls. To be called once per frame. */
		void UpdateInternal();

	private:
		friend class DeferredCall;

		Vector<std::function<void()>> mCallbacks;
	};

	/** @} */
} // namespace b3d
