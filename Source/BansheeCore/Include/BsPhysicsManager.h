//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsModule.h"

namespace bs
{
	/** @addtogroup Physics-Internal
	 *  @{
	 */

	/** Creates and destroys a specific physics implementation. */
	class BS_CORE_EXPORT PhysicsFactory
	{
	public:
		virtual ~PhysicsFactory() { }

		/** Initializes the physics system. */
		virtual void startUp(bool cooking) = 0;

		/** Shuts down the physics system. */
		virtual void shutDown() = 0;
	};

	/** Takes care of loading, initializing and shutting down of a particular physics implementation. */
	class BS_CORE_EXPORT PhysicsManager : public Module<PhysicsManager>
	{
	public:
		/** 
		 * Initializes the physics manager and a particular physics implementation. 
		 *
		 * @param[in]	pluginName	Name of the plugin containing a physics implementation.
		 * @param[in]	cooking		Should the cooking library be initialized with physics (normally only needed for 
		 *							editor).
		 */
		PhysicsManager(const String& pluginName, bool cooking);
		~PhysicsManager();

	private:
		DynLib* mPlugin;
		PhysicsFactory* mFactory;
	};

	/** @} */
}