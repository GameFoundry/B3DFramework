//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Scene/BsTransform.h"

namespace b3d
{namespace render
	{
		class SceneActor;
	}

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

	/** Common class for both main and render thread variants of SceneActor. */
	template<bool IsRenderProxy>
	class B3D_CORE_EXPORT TSceneActor
	{
		using SceneInstanceType = CoreVariantType<SceneInstance, IsRenderProxy>;
	public:
		TSceneActor() = default;
		TSceneActor(const SPtr<SceneInstanceType>& sceneInstance);
		virtual ~TSceneActor() = default;

		/** Determines the position, rotation and scale of the actor. */
		virtual void SetTransform(const Transform& transform);

		/** @copydoc SetTransform */
		const Transform& GetTransform() const { return mTransform; }

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

		/** Determines the scene that the actor is a part of. */
		virtual void SetScene(const SPtr<SceneInstanceType>& instance);

		/** @copydoc SetScene */
		const SPtr<SceneInstanceType>& GetSceneInstance() const { return mSceneInstance; }
	protected:
		/**
		 * Marks the main thread object as dirty and notifies the system its data should be synced with its render
		 * thread counterpart.
		 */
		virtual void MarkSceneActorRenderProxyDataDirty(ActorDirtyFlag flag = ActorDirtyFlag::Everything) {}

	protected:
		friend class SceneManager;

		Transform mTransform;
		ObjectMobility mMobility = ObjectMobility::Movable;
		bool mActive = true;
		u32 mHash = 0;
		SPtr<SceneInstanceType> mSceneInstance;
	};

#if B3D_CORE_EXPORTS
	template <bool IsRenderProxy>
	TSceneActor<IsRenderProxy>::TSceneActor(const SPtr<SceneInstanceType>& sceneInstance)
		: mSceneInstance(sceneInstance)
	{ }

	template<bool IsRenderProxy>
	void TSceneActor<IsRenderProxy>::SetTransform(const Transform& transform)
	{
		if(mMobility != ObjectMobility::Movable)
			return;

		mTransform = transform;
		MarkSceneActorRenderProxyDataDirty(ActorDirtyFlag::Transform);
	}

	template<bool IsRenderProxy>
	void TSceneActor<IsRenderProxy>::SetMobility(ObjectMobility mobility)
	{
		if(mMobility == mobility)
			return;

		mMobility = mobility;
		MarkSceneActorRenderProxyDataDirty(ActorDirtyFlag::Mobility);
	}

	template<bool IsRenderProxy>
	void TSceneActor<IsRenderProxy>::SetActive(bool active)
	{
		if(mActive == active)
			return;

		mActive = active;
		MarkSceneActorRenderProxyDataDirty(ActorDirtyFlag::Active);
	}

	template<bool IsRenderProxy>
	void TSceneActor<IsRenderProxy>::SetScene(const SPtr<SceneInstanceType>& instance)
	{
		if(mSceneInstance == instance)
			return;

		B3D_ENSURE(instance != nullptr);

		mSceneInstance = instance;
		MarkSceneActorRenderProxyDataDirty(ActorDirtyFlag::Everything);
	}
#endif

	/**
	 * A base class for objects that can be placed in the scene. It has a transform object that allows it to be positioned,
	 * scaled and rotated, as well a properties that control its mobility (movable vs. immovable) and active status.
	 *
	 * In a way scene actors are similar to SceneObject%s, the main difference being that their implementations perform
	 * some functionality directly, rather than relying on attached Components. Scene actors can be considered as a
	 * lower-level alternative to SceneObject/Component model. In fact many Components internally just wrap scene actors.
	 */
	class B3D_CORE_EXPORT SceneActor : public TSceneActor<false>
	{
	public:
		using TSceneActor::TSceneActor;

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
		virtual void UpdateStateFromSceneObject(const SceneObject& so, bool force = false);

		/**
		 * Creates a data packet that will be used for syncing the core object with it's render proxy.
		 * Caller must free the retrieved packet using the provided allocator, when done using it.
		 */
		RenderProxySyncPacket* CreateSceneActorRenderProxySyncPacket(FrameAllocator& allocator, u32 flags);

		/** @} */

		struct SyncEverythingPacket;
		struct SyncTransformPacket;
		struct SyncActiveStatePacket;

		/** @} */
	private:
		friend class render::SceneActor;
	};

	namespace render
	{
		/** @copydoc b3d::SceneActor */
		class B3D_CORE_EXPORT SceneActor : public TSceneActor<true>
		{
		public:
			using TSceneActor::TSceneActor;

		private:
			friend class b3d::SceneActor;
		};
	}

	/** @} */
} // namespace b3d
