//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsIReflectable.h"
#include "Scene/BsGameObject.h"
#include "Math/BsVector3.h"
#include "Math/BsQuaternion.h"

namespace bs
{
	struct SerializationContext;

	/** @addtogroup Scene-Internal
	 *  @{
	 */

	/** Contains either a fully serialized game object, or a delta between two game objects. Used internally by SceneObjectHierarchyDelta. */
	struct B3D_CORE_EXPORT SceneObjectHierarchyDeltaObject : public IReflectable
	{
		SceneObjectHierarchyDeltaObject(const HComponent& component, const SPtr<SerializedObject>& data);
		SceneObjectHierarchyDeltaObject(const HSceneObject& sceneObject, const SPtr<SerializedObject>& data);

		UUID Id;
		UUID ParentId;
		UUID PrefabObjectId;
		UUID PrefabResourceId;

		SPtr<SerializedObject> Data;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

	public:
		friend class SceneObjectHierarchyDeltaObjectRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const override;
	};

	/**
	 * Contains modifications between two scene object hierarchies. The modifications are a set of added/removed children or
	 * components and per-field deltas of their components.
	 */
	class B3D_CORE_EXPORT SceneObjectHierarchyDelta : public IReflectable
	{
	public:
		/** Creates a new delta by recording changes present in @p modified, compared to @p original. */
		static SPtr<SceneObjectHierarchyDelta> Create(const HSceneObject& original, const HSceneObject& modified);

		/**
		 * Applies the delta to the provided object. 
		 *
		 * @note	Be aware that this method will not instantiate newly added components or scene objects. It's expected
		 *			that this method will be called on a fresh copy of a scene object hierarchy, and everything to be
		 *			instantiated at once after delta is applied.
		 */
		void Apply(const HSceneObject& original);

	private:
		/** Recursively generates differences between original and the modified version, for every scene object in the hierarchy. */
		static SPtr<SceneObjectHierarchyDeltaObject> GenerateDelta(const HSceneObject& original, const HSceneObject& modified, SPtr<SceneObjectHierarchyDeltaObject>& outDelta);

		/**
		 * Generates differences between components of the two provided scene objects. If components with matching ids are found, a delta
		 * of their properties is recorded. Otherwise a component is registered in either the added or removed component list. @p outDelta
		 * will be created if changes are found, if not already created.
		 */
		static void GenerateComponentDelta(const HSceneObject& original, const HSceneObject& modified, SPtr<SceneObjectHierarchyDelta>& outDelta);

		/**
		 * Generates differences between the two provided scene objects. If the object ids match, a delta of their properties is recorded.
		 * If one of the scene objects is not valid, we record the relevant id in the added or removed scene object list. Note this
		 * not compare scene object components, nor does it iterate over the child hierarchy.@p outDelta will be created if changes are found,
		 * if not already created.
		 */
		static void GenerateSceneObjectDelta(const HSceneObject& original, const HSceneObject& modified, SPtr<SceneObjectHierarchyDelta>& outDelta);

		/** Recursively applies a per-object set of differences to a specific object.  */
		static void ApplyDiff(const SPtr<SceneObjectHierarchyDeltaObject>& delta, const HSceneObject& original, SerializationContext* context);

		UnorderedMap<UUID, SPtr<SceneObjectHierarchyDeltaObject>> Objects;

		UnorderedSet<UUID> AddedComponents;
		UnorderedSet<UUID> RemovedComponents;

		UnorderedSet<UUID> AddedSceneObjects;
		UnorderedSet<UUID> RemovedSceneObjects;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

		Any mRTTIData;

	public:
		friend class SceneObjectHierarchyDeltaRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const override;
	};

	/** @} */
} // namespace bs
