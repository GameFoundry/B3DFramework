//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Scene/BsTransform.h"

namespace bs
{
	struct RenderProxySyncPacket;

	/** @addtogroup Scene-Internal
	 *  @{
	 */

	/**	Signals which portion of a scene actor is dirty. */
	enum class ActorDirtyFlag
	{
		Transform = 1 << 0,
		Mobility = 1 << 1,
		Active = 1 << 2,
		Everything = 1 << 3,
		Dependency = kDirtyDependencyMask
	};

	typedef Flags<ActorDirtyFlag> ActorDirtyFlags;
	B3D_FLAGS_OPERATORS(ActorDirtyFlag)

	// SceneActor is not a core object itself, but as it's used as a base for many, this is needed for common core sync functionality used by those.
	template <>
	struct RenderThreadType<SceneActor>
	{
		typedef SceneActor Type;
	};

	/**
	 * A base class for objects that can be placed in the scene. It has a transform object that allows it to be positioned,
	 * scaled and rotated, as well a properties that control its mobility (movable vs. immovable) and active status.
	 *
	 * In a way scene actors are similar to SceneObject%s, the main difference being that their implementations perform
	 * some functionality directly, rather than relying on attached Components. Scene actors can be considered as a
	 * lower-level alternative to SceneObject/Component model. In fact many Components internally just wrap scene actors.
	 */
	class B3D_CORE_EXPORT SceneActor
	{
	public:
		SceneActor() = default;
		virtual ~SceneActor() = default;

		/** Determines the position, rotation and scale of the actor. */
		virtual void SetTransform(const Transform& transform);

		/** @copydoc SetTransform */
		const Transform& GetTransform() const { return mTransform; }

		/** Shorthand for getTransform(). */
		const Transform& Tfrm() const { return mTransform; }

		/**
		 * Determines if the actor is currently active. Deactivated actors act as if they have been destroyed, without
		 * actually being destroyed.
		 */
		virtual void SetActive(bool active);

		/** @copydoc SetActive */
		bool GetActive() const { return mActive; }

		/**
		 * Determines the mobility of the actor. This is used primarily as a performance hint to engine systems. Objects
		 * with more restricted mobility will result in higher performance. Any transform changes to immobile actors will
		 * be ignored. By default actor's mobility is unrestricted.
		 */
		virtual void SetMobility(ObjectMobility mobility);

		/** @copydoc SetMobility */
		ObjectMobility GetMobility() const { return mMobility; }

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
		virtual void UpdateStateInternal(const SceneObject& so, bool force = false);

		/**
		 * Creates a data packet that will be used for syncing the core object with it's render proxy.
		 * Caller must free the retrieved packet using the provided allocator, when done using it.
		 */
		RenderProxySyncPacket* CreateSceneActorRenderProxySyncPacket(FrameAllocator& allocator, u32 flags);

		struct SyncEverythingPacket;
		struct SyncTransformPacket;
		struct SyncActiveStatePacket;

		/** @} */
	protected:
		/**
		 * Marks the main thread object as dirty and notifies the system its data should be synced with its render
		 * thread counterpart.
		 */
		virtual void MarkRenderProxyDataDirtyInternal(ActorDirtyFlag flag = ActorDirtyFlag::Everything) {}

	protected:
		friend class SceneManager;

		Transform mTransform;
		ObjectMobility mMobility = ObjectMobility::Movable;
		bool mActive = true;
		u32 mHash = 0;
	};
	/** @} */
} // namespace bs
