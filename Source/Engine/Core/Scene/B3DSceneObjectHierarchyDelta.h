//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DIReflectable.h"
#include "Scene/B3DGameObject.h"

namespace b3d
{
	struct RTTIOperationContext;

	/** @addtogroup Scene-Internal
	 *  @{
	 */

	/** Flags that mark which portion of a scene-object is modified. */
	enum class B3D_SCRIPT_EXPORT() SceneObjectDiffFlags // TODO - Deprecated. Only kept because it is still used by script code
	{
		Name = 1,
		Position = 2,
		Rotation = 4,
		Active = 16,
		Scale = 8
	};

	/** Flags that specify additional information about game object delta. */
	enum class GameObjectDeltaFlag
	{
		None = 0,
		SceneObjectDelta = 1 << 0, /**< Delta contains information about a scene object. */
		ComponentDelta = 1 << 1, /**< Delta contains information about a component. */
		ParentChanged = 1 << 2, /**< Set on scene object deltas whose parent has changed. */
	};

	using GameObjectDeltaFlags = Flags<GameObjectDeltaFlag>;
	B3D_FLAGS_OPERATORS(GameObjectDeltaFlag)

	/** Contains either a fully serialized game object, or a delta between two game objects. Used internally by SceneObjectHierarchyDelta. */
	struct B3D_EXPORT SceneObjectHierarchyDeltaObject : public IReflectable
	{
		SceneObjectHierarchyDeltaObject(const HComponent& component, const SPtr<SerializedObject>& data);
		SceneObjectHierarchyDeltaObject(const HSceneObject& sceneObject, const SPtr<SerializedObject>& data);

		UUID Id;
		UUID ParentId;
		UUID PrefabObjectId;
		UUID PrefabResourceId;

		SPtr<SerializedObject> Data;
		GameObjectDeltaFlags Flags;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

	public:
		SceneObjectHierarchyDeltaObject() = default; // RTTI only

		friend class SceneObjectHierarchyDeltaObjectRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** Flags used to control the creation of SceneObjectHierarchyDelta. */
	enum class SceneObjectHierarchyDeltaFlag
	{
		None = 0,
		PrefabDelta = 1 << 0, /**< Delta generated between the prefab root hierarchy and a prefab instance. Compares objects using prefab object ids where necessary. */
	};

	using SceneObjectHierarchyDeltaFlags = Flags<SceneObjectHierarchyDeltaFlag>;
	B3D_FLAGS_OPERATORS(SceneObjectHierarchyDeltaFlag);

	/**
	 * Contains modifications between two scene object hierarchies. The modifications are a set of added/removed children or
	 * components and per-field deltas of their components.
	 */
	class B3D_EXPORT SceneObjectHierarchyDelta : public IReflectable
	{
	public:
		/** Creates a new delta by recording changes present in @p modified, compared to @p original. */
		static SPtr<SceneObjectHierarchyDelta> Create(const HSceneObject& original, const HSceneObject& modified, SceneObjectHierarchyDeltaFlags flags = SceneObjectHierarchyDeltaFlag::None);

		/**
		 * Applies the delta to the provided object. 
		 *
		 * @note	Be aware that this method will not instantiate newly added components or scene objects. It's expected
		 *			that this method will be called on a fresh copy of a scene object hierarchy, and everything to be
		 *			instantiated at once after delta is applied.
		 */
		void Apply(const HSceneObject& original, SceneObjectHierarchyDeltaFlags flags = SceneObjectHierarchyDeltaFlag::None);

	private:
		/** Recursively generates differences between original and the modified version, for every scene object in the hierarchy. */
		static void GenerateHierarchyDelta(const HSceneObject& original, const HSceneObject& modified, RTTIOperationContext& context, SceneObjectHierarchyDeltaFlags flags, SPtr<SceneObjectHierarchyDelta>& outDelta);

		/**
		 * Generates differences between the two provided scene objects. If the object IDs match, a delta of their properties is recorded.
		 * If one of the scene objects is not valid, we record the relevant ID in the added or removed scene object list. Note this
		 * does not iterate over the child hierarchy. @p outDelta will be created if changes are found if not already created.
		 */
		static bool GenerateSceneObjectDelta(const HSceneObject& original, const HSceneObject& modified, RTTIOperationContext& context, SceneObjectHierarchyDeltaFlags flags, bool ignoreParent, SPtr<SceneObjectHierarchyDelta>& outDelta);

		/**
		 * Generates differences between components of the two provided scene objects. If components with matching ids are found, a delta
		 * of their properties is recorded. Otherwise a component is registered in either the added or removed component list. @p outDelta
		 * will be created if changes are found, if not already created.
		 */
		static void GenerateComponentDelta(const HSceneObject& original, const HSceneObject& modified, RTTIOperationContext& context, SceneObjectHierarchyDeltaFlags flags, SPtr<SceneObjectHierarchyDelta>& outDelta);

		UnorderedMap<UUID, SPtr<SceneObjectHierarchyDeltaObject>> Objects;

		Vector<UUID> AddedComponents; /**< Ordered list of added components. */
		UnorderedSet<UUID> RemovedComponents;

		Vector<UUID> AddedSceneObjects; /**< Ordered list of added scene objects. */
		Vector<UUID> RemovedSceneObjects; /**< Ordered list of removed scene objects. */

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

		Any mRTTIData;

	public:
		friend class SceneObjectHierarchyDeltaRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** @} */
} // namespace b3d
