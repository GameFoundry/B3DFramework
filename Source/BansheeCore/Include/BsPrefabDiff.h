//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsIReflectable.h"
#include "BsGameObject.h"
#include "BsVector3.h"
#include "BsQuaternion.h"

namespace bs
{
	/** @addtogroup Scene-Internal
	 *  @{
	 */

	/**
	 * Contains differences between two components of the same type.
	 *
	 * @see		PrefabDiff
	 */
	struct BS_CORE_EXPORT PrefabComponentDiff : public IReflectable
	{
		INT32 id;
		SPtr<SerializedObject> data;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

	public:
		friend class PrefabComponentDiffRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** Flags that mark which portion of a scene-object is modified. */
	enum class SceneObjectDiffFlags
	{
		Name = 0x01,
		Position = 0x02,
		Rotation = 0x04,
		Scale = 0x08,
		Active = 0x10
	};

	/**
	 * Contains a set of prefab differences for a single scene object.
	 *
	 * @see		PrefabDiff
	 */
	struct BS_CORE_EXPORT PrefabObjectDiff : public IReflectable
	{
		PrefabObjectDiff() { }

		UINT32 id = 0;

		String name;
		Vector3 position = Vector3::ZERO;
		Quaternion rotation = Quaternion::IDENTITY;
		Vector3 scale = Vector3::ZERO;
		bool isActive = false;
		UINT32 soFlags = 0;

		Vector<SPtr<PrefabComponentDiff>> componentDiffs;
		Vector<UINT32> removedComponents;
		Vector<SPtr<SerializedObject>> addedComponents;

		Vector<SPtr<PrefabObjectDiff>> childDiffs;
		Vector<UINT32> removedChildren;
		Vector<SPtr<SerializedObject>> addedChildren;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

	public:
		friend class PrefabObjectDiffRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/**
	 * Contains modifications between an prefab and its instance. The modifications are a set of added/removed children or
	 * components and per-field "diffs" of their components.
	 */
	class BS_CORE_EXPORT PrefabDiff : public IReflectable
	{
	public:
		/**
		 * Creates a new prefab diff by comparing the provided instanced scene object hierarchy with the prefab scene 
		 * object hierarchy.
		 */
		static SPtr<PrefabDiff> create(const HSceneObject& prefab, const HSceneObject& instance);

		/**
		 * Applies the internal prefab diff to the provided object. The object should have similar hierarchy as the prefab
		 * the diff was created for, otherwise the results are undefined.
		 *
		 * @note	Be aware that this method will not instantiate newly added components or scene objects. It's expected
		 *			that this method will be called on a fresh copy of a scene object hierarchy, and everything to be
		 *			instantiated at once after diff is applied.
		 */
		void apply(const HSceneObject& object);

	private:
		/** A reference to a renamed game object instance data, and its original ID so it may be restored later. */
		struct RenamedGameObject
		{
			GameObjectInstanceDataPtr instanceData;
			UINT64 originalId;
		};

		/**
		 * Recurses over every scene object in the prefab a generates differences between itself and the instanced version.
		 *
		 * @see		create
		 */
		static SPtr<PrefabObjectDiff> generateDiff(const HSceneObject& prefab, const HSceneObject& instance);

		/**
		 * Recursively applies a per-object set of prefab differences to a specific object.
		 *
		 * @see		apply			
		 */
		static void applyDiff(const SPtr<PrefabObjectDiff>& diff, const HSceneObject& object);

		/**
		 * Renames all game objects in the provided instance so that IDs of the objects will match the IDs of their 
		 * counterparts in the prefab. 
		 *
		 * @note	
		 * This is a temporary action and should be undone by calling restoreInstanceIds() and providing  it with the 
		 * output of this method. 
		 * @note
		 * By doing this before calling generateDiff() we ensure that any game object handles pointing to objects within 
		 * the prefab instance hierarchy aren't recorded by the diff system, since we want those to remain as they are 
		 * after applying the diff.
		 */
		static void renameInstanceIds(const HSceneObject& prefab, const HSceneObject& instance, Vector<RenamedGameObject>& output);

		/**
		 * Restores any instance IDs that were modified by the renameInstanceIds() method.
		 *
		 * @see		renameInstanceIds
		 */
		static void restoreInstanceIds(const Vector<RenamedGameObject>& renamedObjects);

		SPtr<PrefabObjectDiff> mRoot;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

	public:
		friend class PrefabDiffRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** @} */
}