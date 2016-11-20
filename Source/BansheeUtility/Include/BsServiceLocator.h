//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisitesUtil.h"
#include "BsException.h"

namespace bs
{
	/** @addtogroup General
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
		static T* instance() { return mService; }

		/** @name Internal 
		 *  @{
		 */

		/** Starts providing a new service when "instance()" is called. Replaces the previous service. */
		static void _provide(T* service)
		{
			mService = service;
		}

		/**
		 * Stops providing a service when "instance()" is called. Ignored if the current service doesn't match the 
		 * provided service.
		 */
		static void _remove(T* service)
		{
			if (mService != service)
				return;

			mService = nullptr;
		}

		/** @} */

	private:
		static T* mService;
	};

	template <class T>
	T* ServiceLocator<T>::mService = nullptr;

	/** @} */
}