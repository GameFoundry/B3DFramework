//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsModule.h"

namespace bs
{
	/** @addtogroup SBansheeEngine
	 *  @{
	 */

	/**	Keeps track of all loaded managed resources. */
	class BS_SCR_BE_EXPORT ManagedResourceManager : public Module <ManagedResourceManager>
	{
	public:
		ManagedResourceManager();
		~ManagedResourceManager();

		/** Unloads all managed resources. */
		void clear();

		/**	Register a newly created managed resource. */
		void registerManagedResource(const WeakResourceHandle<ManagedResource>& resource);

		/**	Unregister a managed resource that's about to be destroyed. */
		void unregisterManagedResource(const WeakResourceHandle<ManagedResource>& resource);

	private:
		UnorderedMap<String, WeakResourceHandle<ManagedResource>> mResources;
	};

	/** @} */
}