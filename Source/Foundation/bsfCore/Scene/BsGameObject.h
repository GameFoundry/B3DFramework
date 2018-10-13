//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsIReflectable.h"

namespace bs
{
	/** @addtogroup Scene-Internal
	 *  @{
	 */

	/** Flags used for notifying child scene object and components when a transform has been changed. */
	enum TransformChangedFlags
	{
		TCF_None = 0x00, /**< Component will not be notified about any events relating to the transform. */
		TCF_Transform = 0x01, /**< Component will be notified when the its position, rotation or scale has changed. */
		TCF_Parent = 0x02, /**< Component will be notified when its parent changes. */
		TCF_Mobility = 0x04 /**< Component will be notified when mobility state changes. */
	};

	/** @} */
	/** @addtogroup Scene
	 *  @{
	 */

	/**
	 * Type of object that can be referenced by a GameObject handle. Each object has an unique ID and is registered with 
	 * the GameObjectManager.
	 */
	class BS_CORE_EXPORT GameObject : public IReflectable
	{
	public:
		GameObject() = default;
		virtual ~GameObject() = default;

		/**	Returns the unique instance ID of the GameObject. */
		UINT64 getInstanceId() const { return mInstanceData->mInstanceId; }

		/**
		 * Returns an ID that identifies a link between this object and its equivalent in the linked prefab. This will be
		 * -1 if the object has no prefab link, or if the object is specific to the instance and has no prefab equivalent.
		 */
		UINT32 getLinkId() const { return mLinkId; }

		/**	Gets the name of the object. */
		const String& getName() const { return mName; }

		/**	Sets the name of the object. */
		void setName(const String& name) { mName = name; }

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		/**
		 * Marks the object as destroyed. Generally this means the object has been queued for destruction but it hasn't 
		 * occurred yet.
		 */
		void _setIsDestroyed() { mIsDestroyed = true; }

		/**	Checks if the object has been destroyed. */
		bool _getIsDestroyed() const { return mIsDestroyed; }

		/** Changes the prefab link ID for this object. See getLinkId(). */
		void _setLinkId(UINT32 id) { mLinkId = id; }

		/**
		 * Replaces the instance data with another objects instance data. This object will basically become the original 
		 * owner of the provided instance data as far as all game object handles referencing it are concerned.
		 *
		 * @note
		 * No alive objects should ever be sharing the same instance data. This can be used for restoring dead handles.
		 */
		virtual void _setInstanceData(GameObjectInstanceDataPtr& other);

		/** Returns instance data that identifies this GameObject and is used for referencing by game object handles. */
		virtual GameObjectInstanceDataPtr _getInstanceData() const { return mInstanceData; }

		/** @} */

	protected:
		friend class GameObjectHandleBase;
		friend class GameObjectManager;
		friend class PrefabDiff;
		friend class PrefabUtility;

		/**	Initializes the GameObject after construction. */
		void initialize(const SPtr<GameObject>& object, UINT64 instanceId);

		/**
		 * Destroys this object.
		 *
		 * @param[in]	handle		Game object handle to this object.
		 * @param[in]	immediate	If true, the object will be deallocated and become unusable right away. Otherwise the 
		 *							deallocation will be delayed to the end of frame (preferred method).
		 */
		virtual void destroyInternal(GameObjectHandleBase& handle, bool immediate = false) = 0;

	protected:
		String mName;
		UINT32 mLinkId = (UINT32)-1;

		Any mRTTIData; // RTTI only
	private:
		friend class Prefab;
		GameObjectInstanceDataPtr mInstanceData;
		bool mIsDestroyed = false;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
		friend class ComponentRTTI;
		friend class SceneObjectRTTI;

	public:
		friend class GameObjectRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** @} */
}