//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Utility/B3DModule.h"
#include "Scene/B3DGameObject.h"

namespace b3d
{
	/** @addtogroup Scene-Internal
	 *  @{
	 */

	/**
	 * Keeps track of all active game object collections.
	 *
	 * @note	Main thread only.
	 */
	class B3D_EXPORT GameObjectManager : public Module<GameObjectManager>
	{
	public:
		GameObjectManager() = default;
		~GameObjectManager();

		/**	Destroys any GameObjects that were queued for destruction. */
		void DestroyQueuedObjects();

	private:
		friend class GameObjectCollection;

		/** Notifies the manager that a new game object collection was created. */
		void RegisterGameObjectCollection(const SPtr<GameObjectCollection>& collection);

		/** Notifies the manager that a game object collection was about to be destroyed. */
		void UnregisterGameObjectCollection(const GameObjectCollection& collection);

		UnorderedMap<UUID, WeakSPtr<GameObjectCollection>> mGameObjectCollections;
	};

	/** @} */
} // namespace b3d
