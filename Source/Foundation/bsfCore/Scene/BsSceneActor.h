//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Scene/BsTransform.h"

namespace bs
{
	/** @addtogroup Scene-Internal
	 *  @{
	 */

	/**	Signals which portion of a scene actor is dirty. */
	enum class ActorDirtyFlag
	{
		Transform	= 1 << 0,
		Mobility	= 1 << 1,
		Active		= 1 << 2,
		Everything	= 1 << 3,
		Dependency	= 1 << 31
	};

	/**
	 * A base class for objects that can be placed in the scene. It has a transform object that allows it to be positioned,
	 * scaled and rotated, as well a properties that control its mobility (movable vs. immovable) and active status. 
	 * 
	 * In a way scene actors are similar to SceneObject%s, the main difference being that their implementations perform
	 * some functionality directly, rather than relying on attached Components. Scene actors can be considered as a
	 * lower-level alternative to SceneObject/Component model. In fact many Components internally just wrap scene actors.
	 */
	class BS_CORE_EXPORT SceneActor
	{
	public:
		SceneActor() = default;
		virtual ~SceneActor() = default;

		/** Determines the position, rotation and scale of the actor. */
		virtual void setTransform(const Transform& transform);

		/** @copydoc setTransform */
		const Transform& getTransform() const { return mTransform; }

		/** Shorthand for getTransform(). */
		const Transform& tfrm() const { return mTransform; }

		/** 
		 * Determines if the actor is currently active. Deactivated actors act as if they have been destroyed, without
		 * actually being destroyed.
		 */
		virtual void setActive(bool active);

		/** @copydoc setActive */
		bool getActive() const { return mActive; }

		/**
		 * Determines the mobility of the actor. This is used primarily as a performance hint to engine systems. Objects
		 * with more restricted mobility will result in higher performance. Any transform changes to immobile actors will
		 * be ignored. By default actor's mobility is unrestricted.
		 */
		virtual void setMobility(ObjectMobility mobility);

		/** @copydoc setMobility */
		ObjectMobility getMobility() const { return mMobility; }

		/** 
		 * @name Internal
		 * @{
		 */
		
		/** 
		 * Updates the internal actor state by transfering the relevant state from the scene object. The system tracks
		 * the last state and only performs the update if the scene object was modified since the last call. You can force
		 * an update by setting the @p force parameter to true. 
		 * 
		 * This method is used by the scene manager to update actors that have been bound to a scene object. Never call this
		 * method for multiple different scene objects, as actor can only ever be bound to one during its lifetime.
		 */
		virtual void _updateState(const SceneObject& so, bool force = false);

		/** @} */
	protected:
		/** 
		 * Marks the simulation thread object as dirty and notifies the system its data should be synced with its core 
		 * thread counterpart. 
		 */
		virtual void _markCoreDirty(ActorDirtyFlag flag = ActorDirtyFlag::Everything) { }

		/** 
		 * Writes the contents of this object into the provided data buffer. Buffer must have enough size of hold
		 * getSyncActorDataSize() bytes. Returns the address after the last written byte.
		 */
		char* syncActorTo(char* data);

		/** 
		 * Reads the contents of this object from the provided data buffer. The buffer is expected to be populated by the
		 * sim thread version of this object by calling syncActorTo(). Returns the address after the last read byte. 
		 */
		char* syncActorFrom(char* data);

		/** Returns the size of the buffer required to store all data in this object, in bytes. */
		UINT32 getActorSyncDataSize() const;
	protected:
		friend class SceneManager;

		Transform mTransform;
		ObjectMobility mMobility = ObjectMobility::Movable;
		bool mActive = true;
		UINT32 mHash = 0;
	};

	/** @} */
}
