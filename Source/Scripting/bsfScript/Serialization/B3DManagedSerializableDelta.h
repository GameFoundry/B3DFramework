//********************************* B3D Framework - Copyright 2026 Marko Pintera *****************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "Reflection/B3DIReflectable.h"

namespace b3d
{
	/** @addtogroup bsfScript
	 *  @{
	 */

	/**
	 * Handles creation and applying of managed deltas. A delta contains differences between two objects of identical types.
	 * If the initial state of an object is known the recorded differences can be saved and applied to the original state to
	 * restore the modified object.
	 *
	 * Differences are recorded per primitive field in an object. Complex objects are recursed. Special handling is
	 * implemented to properly generate deltas for individual entries within arrays, lists and dictionaries.
	 *
	 * All primitive types supported by managed serialization are supported (see ScriptPrimitiveType).
	 */
	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableDelta : public IReflectable
	{
	public:
		/**	A base class for all modifications recorded in a diff. */
		struct Modification : IReflectable
		{
			virtual ~Modification() = default;

			/************************************************************************/
			/* 								RTTI		                     		*/
			/************************************************************************/
		public:
			friend class ModificationRTTI;
			static RTTIType* GetRttiStatic();
			RTTIType* GetRtti() const override;
		};

		/**
		 * Contains a modification of a specific field in an object along with information about the field and its parent
		 * object.
		 */
		struct ModifiedField : IReflectable
		{
			ModifiedField() = default;
			ModifiedField(const SPtr<ManagedTypeInfo>& parentType, const SPtr<ManagedMemberInfo>& fieldType, const SPtr<Modification>& modification);

			SPtr<ManagedTypeInfo> ParentType; /**< Type of the parent object the field belongs to. */
			SPtr<ManagedMemberInfo> FieldType; /**< Data type of the field. */
			SPtr<Modification> Modification; /**< Recorded modification(s) on the field. */

			/************************************************************************/
			/* 								RTTI		                     		*/
			/************************************************************************/
		public:
			friend class ModifiedFieldRTTI;
			static RTTIType* GetRttiStatic();
			RTTIType* GetRtti() const override;
		};

		/**	Represents a single modified array or list entry. */
		struct ModifiedArrayEntry : IReflectable
		{
			ModifiedArrayEntry() = default;
			ModifiedArrayEntry(u32 idx, const SPtr<Modification>& modification);

			u32 Idx; /**< Index of the array/list entry that is modified. */
			SPtr<Modification> Modification; /**< Recorded modification(s) on the entry. */

			/************************************************************************/
			/* 								RTTI		                     		*/
			/************************************************************************/
		public:
			friend class ModifiedArrayEntryRTTI;
			static RTTIType* GetRttiStatic();
			RTTIType* GetRtti() const override;
		};

		/**	Represents a single modified dictionary entry. */
		struct ModifiedDictionaryEntry : IReflectable
		{
			ModifiedDictionaryEntry() = default;
			ModifiedDictionaryEntry(const SPtr<ManagedSerializableFieldData>& key, const SPtr<Modification>& modification);

			SPtr<ManagedSerializableFieldData> Key; /**< Serialized value of the key for the modified entry. */
			SPtr<Modification> Modification; /**< Recorded modification(s) on the dictionary entry value. */

			/************************************************************************/
			/* 								RTTI		                     		*/
			/************************************************************************/
		public:
			friend class ModifiedArrayEntryRTTI;
			static RTTIType* GetRttiStatic();
			RTTIType* GetRtti() const override;
		};

		/**
		 * Contains data about all modifications in a single complex object (aside from arrays, list, dictionaries which are
		 * handled specially).
		 */
		struct ModifiedObject : Modification
		{
			static SPtr<ModifiedObject> Create();

			Vector<ModifiedField> Entries; /**< A list of entries containing each modified field in the object. */

			/************************************************************************/
			/* 								RTTI		                     		*/
			/************************************************************************/
		public:
			friend class ModifiedObjectRTTI;
			static RTTIType* GetRttiStatic();
			RTTIType* GetRtti() const;
		};

		/**	Contains data about all modifications in an array or a list. */
		struct ModifiedArray : Modification
		{
			static SPtr<ModifiedArray> Create();

			Vector<ModifiedArrayEntry> Entries; /**< A list of all modified array/list entries along with their indices. */
			Vector<u32> OrigSizes; /**< Original size of the array/list (one size per dimension). */
			Vector<u32> NewSizes; /**< New size of the array/list (one size per dimension). */

			/************************************************************************/
			/* 								RTTI		                     		*/
			/************************************************************************/
		public:
			friend class ModifiedArrayRTTI;
			static RTTIType* GetRttiStatic();
			RTTIType* GetRtti() const;
		};

		/**	Contains data about all modifications in a dictionary. */
		struct ModifiedDictionary : Modification
		{
			static SPtr<ModifiedDictionary> Create();

			/** A list of modified entries in the dictionary. */
			Vector<ModifiedDictionaryEntry> Entries;
			/** A list of keys for entries that were removed from the dictionary. */
			Vector<SPtr<ManagedSerializableFieldData>> Removed;

			/************************************************************************/
			/* 								RTTI		                     		*/
			/************************************************************************/
		public:
			friend class ModifiedDictionaryRTTI;
			static RTTIType* GetRttiStatic();
			RTTIType* GetRtti() const;
		};

		/** Contains data about modification of a primitive field (field's new value). */
		struct ModifiedEntry : Modification
		{
			ModifiedEntry() = default;
			ModifiedEntry(const SPtr<ManagedSerializableFieldData>& value);

			static SPtr<ModifiedEntry> Create(const SPtr<ManagedSerializableFieldData>& value);

			SPtr<ManagedSerializableFieldData> Value;

			/************************************************************************/
			/* 								RTTI		                     		*/
			/************************************************************************/
		public:
			friend class ModifiedEntryRTTI;
			static RTTIType* GetRttiStatic();
			RTTIType* GetRtti() const;
		};

	public:
		ManagedSerializableDelta();
		~ManagedSerializableDelta() = default;

		/**
		 * Generates a new managed delta by comparing two objects. Caller must ensure both objects are not null and of identical types.
		 *
		 * @param	original	Original object to compared with @p modified. This is the object you can apply the delta to to convert it to @p modified.
		 * @param	modified	Modified object. Any values in this object that differ from the original object will be recorded in the delta.
		 * @return				Returns null if objects are identical.
		 */
		static SPtr<ManagedSerializableDelta> Create(const ManagedSerializableObject* original, const ManagedSerializableObject* modified, RTTIOperationContext* context = nullptr);

		/**
		 * Applies the delta stored in this object to the specified object, modifying all fields in the object to correspond to the delta.
		 */
		void Apply(const SPtr<ManagedSerializableObject>& object);

	private:
		/** Recursively generates a delta between all fields of the specified objects. Returns null if objects are identical. */
		SPtr<ModifiedObject> GenerateObjectDelta(const ManagedSerializableObject* original, const ManagedSerializableObject* modified, RTTIOperationContext* context);

		/**
		 * Generates a delta between two fields. Fields can be of any type and the system will generate the delta appropriately. Delta is generated recursively on all complex objects.
		 * Returns null if fields contain identical data.
		 */
		SPtr<Modification> GenerateFieldDelta(const SPtr<ManagedSerializableFieldData>& original, const SPtr<ManagedSerializableFieldData>& modified, u32 fieldTypeId, RTTIOperationContext* context);

		/**
		 * Applies an object modification to a managed object. Modifications are applied recursively.
		 *
		 * @param	delta	Object modification to apply.
		 * @param	object	Object to apply the modification to.
		 * @return			New field data in the case modification needed the object to be re-created instead of just modified.
		 */
		SPtr<ManagedSerializableFieldData> ApplyObjectDelta(const SPtr<ModifiedObject>& delta, const SPtr<ManagedSerializableObject>& object);

		/**
		 * Applies an array modification to a managed array. Modifications are applied recursively.
		 *
		 * @param	delta	Array modification to apply.
		 * @param	object	Array to apply the modification to.
		 * @return			New field data in the case modification needed the array to be re-created instead of just modified.
		 */
		SPtr<ManagedSerializableFieldData> ApplyArrayDelta(const SPtr<ModifiedArray>& delta, const SPtr<ManagedSerializableArray>& object);

		/**
		 * Applies an list modification to a managed list. Modifications are applied recursively.
		 *
		 * @param	delta	List modification to apply.
		 * @param	object	List to apply the modification to.
		 * @return			New field data in the case modification needed the list to be re-created instead of just modified.
		 */
		SPtr<ManagedSerializableFieldData> ApplyListDelta(const SPtr<ModifiedArray>& delta, const SPtr<ManagedSerializableList>& object);

		/**
		 * Applies an dictionary modification to a managed dictionary. Modifications are applied recursively.
		 *
		 * @param	delta	Dictionary modification to apply.
		 * @param	object	Dictionary to apply the modification to.
		 * @return			New field data in the case modification needed the dictionary to be re-created instead of just modified.
		 */
		SPtr<ManagedSerializableFieldData> ApplyDictionaryDelta(const SPtr<ModifiedDictionary>& delta, const SPtr<ManagedSerializableDictionary>& object);

		/**
		 * Applies a modification to a single field. Field type is determined and the modification is applied to the specific field type as needed. Modifications are applied recursively.
		 *
		 * @param	delta		Modification to apply.
		 * @param	fieldType	Type of the field we're applying the modification to.
		 * @param	fieldData	Original data of the field, to apply the modification to.
		 * @return				New field data in the case modification needed the field data to be re-created instead of just modified.
		 */
		SPtr<ManagedSerializableFieldData> ApplyDiff(const SPtr<Modification>& delta, const SPtr<ManagedTypeInfo>& fieldType, const SPtr<ManagedSerializableFieldData>& fieldData);

		SPtr<ModifiedObject> mModificationRoot;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ManagedSerializableDeltaRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** @} */
} // namespace b3d
