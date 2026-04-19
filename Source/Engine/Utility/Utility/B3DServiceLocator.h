//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"

#include <atomic>

namespace b3d
{
	/** @addtogroup System
	 *  @{
	 */

	/**
	 * A locator system that allows you to quickly find a service of a specific type.
	 *
	 * @note
	 * This is similar to a singleton pattern but unlike singleton the active instance is not required to be available and
	 * can be replaced with another system during runtime, or completely removed.
	 */
	template <class T>
	class ServiceLocator
	{
	public:
		/**
		 * Returns an instance of the service we are looking for, if one is available.
		 *
		 * @note	Can return null.
		 */
		static T* Instance() { return mService; }

		/** @name Internal
		 *  @{
		 */

		/** Starts providing a new service when "instance()" is called. Replaces the previous service. */
		static void ProvideInternal(T* service)
		{
			mService = service;
		}

		/**
		 * Stops providing a service when "instance()" is called. Ignored if the current service doesn't match the
		 * provided service.
		 */
		static void RemoveInternal(T* service)
		{
			mService.compare_exchange_strong(service, nullptr);
		}

		/** @} */

	private:
		static std::atomic<T*> mService;
	};

	template <class T>
	std::atomic<T*> ServiceLocator<T>::mService{ nullptr };

	/** @} */
} // namespace b3d
