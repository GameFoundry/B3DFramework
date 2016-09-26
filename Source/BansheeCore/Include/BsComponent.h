//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsGameObject.h"
#include "BsBounds.h"

namespace BansheeEngine
{
	/** @addtogroup Scene
	 *  @{
	 */

	/** Components represent primary logic elements in the scene. They are attached to scene objects. */
	class BS_CORE_EXPORT Component : public GameObject
	{
	public:
		/**	Returns the SceneObject this Component is assigned to. */
		HSceneObject sceneObject() const { return mParent; }

		/** @copydoc sceneObject */
		HSceneObject SO() const { return sceneObject(); }

		/**	Returns a handle to this object. */
		HComponent getHandle() const { return mThisHandle; }

		/**
		 * Called once per frame on all components.
		 * 			
		 * @note	Internal method.
		 */
		virtual void update() { }

		/**
		 * Calculates bounds of the visible contents represented by this component (for example a mesh for Renderable).
		 * 
		 * @param[in]	bounds	Bounds of the contents in world space coordinates.	
		 * @return				True if the component has bounds with non-zero volume, otherwise false.
		 */
		virtual bool calculateBounds(Bounds& bounds);

		/**
		 * Checks if this and the provided component represent the same type.
		 * 			
		 * @note	
		 * RTTI type cannot be checked directly since components can be further specialized internally for scripting 
		 * purposes.
		 */
		virtual bool typeEquals(const Component& other);

		/**
		 * Removes the component from parent SceneObject and deletes it. All the references to this component will be 
		 * marked as destroyed and you will get an exception if you try to use them.
		 *
		 * @param[in]	immediate	If true the destruction will be performed immediately, otherwise it will be delayed 
		 *							until the end of the current frame (preferred option).
		 */
		void destroy(bool immediate = false);

		/** @name Internal 
		 *  @{
		 */

		/**
		 * Construct any resources the component needs before use. Called when the parent scene object is instantiated. 
		 * A non-instantiated component shouldn't be used for any other purpose than serialization.
		 */
		virtual void _instantiate() {}

		/** @} */
	protected:
		friend class SceneObject;
		friend class SceneObjectRTTI;

		Component(const HSceneObject& parent);
		virtual ~Component();

		/**	Called when the component is ready to be initialized. */
		virtual void onInitialized() {}

		/**	Called just before the component is destroyed. */
		virtual void onDestroyed() {}

		/**	Called just before the component is deactivated or destroyed. */
		virtual void onDisabled() {}

		/**	Called when the component is activated or created. */
		virtual void onEnabled() {}

		/** Called when the component's parent scene object has changed. */
		virtual void onTransformChanged(TransformChangedFlags flags) { }

		/** Sets new flags that determine when is onTransformChanged called. */
		void setNotifyFlags(TransformChangedFlags flags) { mNotifyFlags = flags; }

		/** Checks whether the component wants to received the specified transform changed message. */
		bool supportsNotify(TransformChangedFlags flags) const { return (mNotifyFlags & flags) != 0; }

		/**
		 * Destroys this component.
		 *
		 * @param[in]	handle		Game object handle this this object.
		 * @param[in]	immediate	If true, the object will be deallocated and become unusable right away. Otherwise the 
		 *							deallocation will be delayed to the end of frame (preferred method).
		 *
		 * @note	Unlike destroy(), does not remove the component from its parent.
		 */
		void destroyInternal(GameObjectHandleBase& handle, bool immediate = false) override;
	private:
		Component(const Component& other) { }

	protected:
		HComponent mThisHandle;
		TransformChangedFlags mNotifyFlags;

	private:
		HSceneObject mParent;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ComponentRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;

	protected:
		Component() {} // Serialization only
	};

	/** @} */
}