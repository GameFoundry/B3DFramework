//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"
#include "Error/BsException.h"

namespace bs
{
	/** @addtogroup General
	 *  @{
	 */

	/**
	 * Represents one engine module. Essentially it is a specialized type of singleton. Module must be manually started up 
	 * and shut down before and after use.
	 */
	template <class T>
	class Module
	{
		public:
		/**
		 * Returns a reference to the module instance. Module has to have been started up first otherwise an exception will
		 * be thrown.
		 */
		static T& instance()
		{
			if (!isStartedUp())
			{
				BS_EXCEPT(InternalErrorException, 
					"Trying to access a module but it hasn't been started up yet.");
			}

			if (isDestroyed())
			{
				BS_EXCEPT(InternalErrorException, 
					"Trying to access a destroyed module.");
			}

			return *_instance();
		}

		/**
		 * Returns a pointer to the module instance. Module has to have been started up first otherwise an exception will 
		 * be thrown.
		 */
		static T* instancePtr()
		{
			if (!isStartedUp())
			{
				BS_EXCEPT(InternalErrorException, 
					"Trying to access a module but it hasn't been started up yet.");
			}

			if (isDestroyed())
			{
				BS_EXCEPT(InternalErrorException, 
					"Trying to access a destroyed module.");
			}

			return _instance();
		}
		
		/** Constructs and starts the module using the specified parameters. */
		template<class ...Args>
		static void startUp(Args &&...args)
		{
			if (isStartedUp())
				BS_EXCEPT(InternalErrorException, "Trying to start an already started module.");

			_instance() = bs_new<T>(std::forward<Args>(args)...);
			isStartedUp() = true;

			((Module*)_instance())->onStartUp();
		}

		/**
		 * Constructs and starts a specialized type of the module. Provided type must derive from type the Module is 
		 * initialized with.
		 */
		template<class SubType, class ...Args>
		static void startUp(Args &&...args)
		{
			static_assert(std::is_base_of<T, SubType>::value, "Provided type is not derived from type the Module is initialized with.");

			if (isStartedUp())
				BS_EXCEPT(InternalErrorException, "Trying to start an already started module.");

			_instance() = bs_new<SubType>(std::forward<Args>(args)...);
			isStartedUp() = true;

			((Module*)_instance())->onStartUp();
		}

		/** Shuts down this module and frees any resources it is using. */
		static void shutDown()
		{
			if (isDestroyed())
			{
				BS_EXCEPT(InternalErrorException, 
					"Trying to shut down an already shut down module.");
			}
			
			if (!isStartedUp()) 
			{
				BS_EXCEPT(InternalErrorException, 
					"Trying to shut down a module which was never started.");
			}

			((Module*)_instance())->onShutDown();

			bs_delete(_instance());
			isDestroyed() = true;
		}

		/** Query if the module has been started. */
		static bool isStarted()
		{
			return isStartedUp() && !isDestroyed();
		}

	protected:
		Module() 
		{ }

		virtual ~Module()
		{ }

		Module(const Module&) { }
		Module& operator=(const Module&) { return *this; }

		/**
		 * Override if you want your module to be notified once it has been constructed and started.
		 * 			
		 * @note	Useful when your module is polymorphic and you cannot perform some implementation specific 
		 *			initialization in constructor itself.
		 */
		virtual void onStartUp() {}

		/**
		 * Override if you want your module to be notified just before it is deleted.
		 * 			
		 * @note	Useful when your module is polymorphic and you might want to perform some kind of clean up perhaps 
		 *			overriding that of a base class.
		 */
		virtual void onShutDown() {}

		/** Returns a singleton instance of this module. */
		static T*& _instance()
		{
			static T* inst = nullptr;
			return inst;
		}

		/**
		 * Checks has the Module been shut down.
		 *			
		 * @note	If module was never even started, this will return false.
		 */
		static bool& isDestroyed()
		{
			static bool inst = false;
			return inst;
		}

		/** Checks has the Module been started up. */
		static bool& isStartedUp()
		{
			static bool inst = false;
			return inst;
		}
	};

	/** @} */
}
