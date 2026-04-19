//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"

namespace b3d
{
	struct RTTIOperationContext;
}

namespace b3d
{
	/** @addtogroup Serialization
	 *  @{
	 */

	/** Helper class that performs cloning of an object that implements RTTI. */
	class B3D_EXPORT BinaryCloner
	{
	public:
		/**
		 * Returns a copy of the provided object with identical data.
		 *
		 * @param[in]	object		Object to clone.
		 * @param[in]	shallow		If false then all referenced objects will be cloned as well, otherwise the references
		 *							to the original objects will be kept.
		 */
		static SPtr<IReflectable> Clone(IReflectable* object, bool shallow = false);

	private:
		struct ObjectExternalReferences;

		/** Identifier representing a single field or a container entry in an object. */
		struct ReferenceId
		{
			RTTIField* Field = nullptr;
			u32 ArrayIndex = ~0u;
			const void* MapKey = nullptr;
			u32 TupleElementIndex = ~0u;
		};

		/** A saved reference to an object with a field identifier that owns it. */
		struct ObjectReference
		{
			ReferenceId Id;
			SPtr<IReflectable> Object;
		};

		/**
		 * Contains all object references in a portion of an object belonging to a specific class (base and derived
		 * classes count as separate sub-objects).
		 */
		struct SubObjectExternalReferences
		{
			RTTIType* Rtti = nullptr;
			Vector<ObjectReference> References;
			Vector<ObjectExternalReferences> ChildObjects;
		};

		/** Contains all object references in an entire object, as well as the identifier of the field owning this object. */
		struct ObjectExternalReferences
		{
			ReferenceId Id;
			Vector<SubObjectExternalReferences> SubObjectReferences;
		};

		/** Iterates over the provided object hierarchy and retrieves all object references which are returned in a hierarchical format for easier parsing. */
		static ObjectExternalReferences GatherExternalReferences(IReflectable* object, FrameAllocator& allocator, RTTIOperationContext& rttiOperationContext);

		/**
		 * Restores a set of references retrieved by GatherExternalReferences() and applies them to a specific object. Type of the
		 * object must be the same as the type that was used when calling GatherExternalReferences().
		 */
		static void RestoreExternalReferences(IReflectable* object, FrameAllocator& allocator, const ObjectExternalReferences& externalReferences, RTTIOperationContext& rttiOperationContext);
	};

	/**
	 * Clones the provided object.
	 *
	 * @param	object		Object to clone.
	 * @param	shallow		Determines how are fields containing reflectable pointers. If true, then those pointers will keep pointing to the original
	 *						object (both the clone and original referencing the same object by the pointer). If false, then the pointer object to will be cloned as well.
	 */
	template <class T>
	SPtr<T> B3DRTTIClone(const T* const object, bool shallow = false)
	{
		static_assert((std::is_base_of_v<IReflectable, T>), "Cannot clone object. It needs to derive from b3d::IReflectable.");

		if(object == nullptr)
			return nullptr;

		BinaryCloner cloner;
		return std::static_pointer_cast<T>(cloner.Clone(const_cast<T*>(object), shallow));
	}

	/** @copydoc B3DRTTIClone(const IReflectable* const, bool) */
	template <class T>
	SPtr<T> B3DRTTIClone(const SPtr<T>& object, bool shallow = false)
	{
		static_assert((std::is_base_of_v<IReflectable, T>), "Cannot clone object. It needs to derive from b3d::IReflectable.");

		if(object == nullptr)
			return nullptr;

		BinaryCloner cloner;
		return std::static_pointer_cast<T>(cloner.Clone(const_cast<T*>(object.get()), shallow));
	}

	/** @} */
} // namespace b3d
