//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Utility/BsModule.h"

namespace bs
{
	/** @addtogroup bsfScript
	 *  @{
	 */

	/**	Keeps track of all loaded managed resources. */
	class BS_SCR_BE_EXPORT ManagedResourceManager : public Module <ManagedResourceManager>
	{
	public:
		ManagedResourceManager() = default;
		~ManagedResourceManager();

		/** Unloads all managed resources. */
		void clear();

		/**	Register a newly created managed resource. */
		void registerManagedResource(const WeakResourceHandle<ManagedResource>& resource);

		/**	Unregister a managed resource that's about to be destroyed. */
		void unregisterManagedResource(const WeakResourceHandle<ManagedResource>& resource);

	private:
		UnorderedMap<UUID, WeakResourceHandle<ManagedResource>> mResources;
	};

	/** @} */
}