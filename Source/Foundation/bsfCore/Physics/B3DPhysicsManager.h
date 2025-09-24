//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Utility/BsModule.h"

namespace b3d
{
	/** @addtogroup Physics-Internal
	 *  @{
	 */

	/** Creates and destroys a specific physics implementation. */
	class B3D_CORE_EXPORT PhysicsFactory
	{
	public:
		virtual ~PhysicsFactory() = default;

		/** Initializes the physics system. */
		virtual void StartUp(bool cooking) = 0;

		/** Shuts down the physics system. */
		virtual void ShutDown() = 0;
	};

	/** Takes care of loading, initializing and shutting down of a particular physics implementation. */
	class B3D_CORE_EXPORT PhysicsManager : public Module<PhysicsManager>
	{
	public:
		/**
		 * Initializes the physics manager and a particular physics implementation.
		 *
		 * @param[in]	pluginName	Name of the plugin containing a physics implementation.
		 * @param[in]	cooking		Should the physics cooking library be initialized (normally only needed during
		 *							development).
		 */
		PhysicsManager(const String& pluginName, bool cooking);
		~PhysicsManager();

	private:
		DynamicLibrary* mPlugin;
		PhysicsFactory* mFactory;
	};

	/** @} */
} // namespace b3d
