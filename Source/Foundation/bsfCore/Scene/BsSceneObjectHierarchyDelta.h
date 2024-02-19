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

	/** Contains differences between two components of the same type. */
	struct B3D_CORE_EXPORT ComponentDelta : public IReflectable
	{
		UUID Id;
		UUID ParentId;
		UUID PrefabObjectId;
		SPtr<SerializedObject> Data;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

	public:
		friend class ComponentDeltaRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const override;
	};

	/** Flags that mark which portion of a scene-object is modified. */
	enum class B3D_SCRIPT_EXPORT(API(Editor), DocumentationGroup(Utility - Editor)) SceneObjectDiffFlags
	{
		Name = 0x01,
		Position = 0x02,
		Rotation = 0x04,
		Scale = 0x08,
		Active = 0x10
	};

	/** Contains a set of differences between two scene objects. */
	struct B3D_CORE_EXPORT SceneObjectDelta : public IReflectable
	{
		UUID Id;
		UUID ParentId;
		UUID PrefabObjectId;
		UUID PrefabResourceId;

		// TODO - Serialize these as part of SerializedObject
		String Name;
		Vector3 Position = Vector3::kZero;
		Quaternion Rotation = Quaternion::kIdentity;
		Vector3 Scale = Vector3::kZero;
		bool IsActive = false;
		u32 SoFlags = 0;

		Vector<SPtr<ComponentDelta>> ComponentDeltas;
		Vector<UUID> RemovedComponents;
		Vector<SPtr<SerializedObject>> AddedComponents;

		Vector<SPtr<SceneObjectDelta>> ChildDeltas;
		Vector<UUID> RemovedChildren;
		Vector<SPtr<SerializedObject>> AddedChildren;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

	public:
		friend class SceneObjectDeltaRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const override;
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
	class B3D_CORE_EXPORT SceneObjectHierarchyDelta : public IReflectable
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
		void Apply(const HSceneObject& original);

	private:
		/** Recursively generates differences between original and the modified version, for every scene object in the hierarchy. */
		static SPtr<SceneObjectDelta> GenerateDelta(const HSceneObject& original, const HSceneObject& modified, SceneObjectHierarchyDeltaFlags flags);

		/** Recursively applies a per-object set of differences to a specific object.  */
		static void ApplyDiff(const SPtr<SceneObjectDelta>& delta, const HSceneObject& original, SerializationContext* context);

		SPtr<SceneObjectDelta> mRoot;

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
