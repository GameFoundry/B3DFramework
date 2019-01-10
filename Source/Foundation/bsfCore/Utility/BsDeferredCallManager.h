//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Utility/BsModule.h"

namespace bs
{
	/** @addtogroup Utility-Core-Internal
	 *  @{
	 */

	/**
	 * Allows you to queue calls that can get executed later.
	 * 			
	 * @note	Sim thread only.
	 */
	class BS_CORE_EXPORT DeferredCallManager : public Module<DeferredCallManager>
	{
	public:
		DeferredCallManager() = default;

		/**
		 * Register a deferred call that will be executed once at the start of next frame.
		 *
		 * @param[in]	func	The function to execute.
		 */
		void queueDeferredCall(std::function<void()> func);

		/** Executes all the scheduled calls. To be called once per frame. */
		void _update();

	private:
		friend class DeferredCall;

		Vector<std::function<void()>> mCallbacks;
	};

	/** @} */
}