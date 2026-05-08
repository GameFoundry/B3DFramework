//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "Reflection/B3DIReflectable.h"

namespace b3d
{
	/** @addtogroup bsfScript
	 *  @{
	 */

	/**	Valid serializable script types. */
	enum class B3D_SCRIPT_EXPORT() ManagedPrimitiveType
	{
		Bool,
		Char,
		I8,
		U8,
		I16,
		U16,
		I32,
		U32,
		I64,
		U64,
		Float,
		Double,
		String,
		Count // Keep at end
	};

	/** Valid reference script types. */
	enum class B3D_SCRIPT_EXPORT() ManagedReferenceType
	{
		BuiltinResourceBase,
		BuiltinResource,
		ManagedResourceBase,
		ManagedResource,
		BuiltinComponentBase,
		BuiltinComponent,
		ManagedComponentBase,
		ManagedComponent,
		SceneObject,
		ReflectableObject,
		Count // Keep at end
	};

	/**	Flags that are used to further define a field in a managed serializable object. */
	enum class B3D_SCRIPT_EXPORT() ManagedFieldMetaDataFlag
	{
		/** Field will be automatically serialized. */
		Serializable = 1 << 0,

		/** Field will be visible in the default inspector. */
		Inspectable = 1 << 1,

		/** Integer or floating point field with min/max range. */
		Range = 1 << 2,

		/** Integer or floating point field with a minimum increment/decrement step. */
		Step = 1 << 3,

		/** Field can be animated through the animation window. */
		Animable = 1 << 4,

		/** Integer field rendered as a layer selection dropdown. */
		AsLayerMask = 1 << 5,

		/** Field containing a reference type being passed by copy instead of by reference. */
		PassByCopy = 1 << 6,

		/** Field containing a reference type that should never be null. */
		NotNull = 1 << 7,

		/**
		 * Field represents a property that wraps a native object. Getters and setters of such a property issue calls into native code to
		 * update the native object.
		 */
		NativeWrapper = 1 << 8,

		/**
		 * When a field changes those changes need to be applied to the parent object by calling the field setter. Only applicable
		 * to properties containing reference types.
		 */
		ApplyOnDirty = 1 << 9,

		/**
		 * When a quaternion is displayed in the inspector, by default it will be displayed as converted into euler angles. Use this flag to
		 * force it to be displayed as a quaternion (4D value) with no conversion instead.
		 */
		AsQuaternion = 1 << 10,

		/**
		 * Fields contains information about a category, which is used for grouping fields under a foldout in the inspector. Retrieve the
		 * category field style for information about the category.
		 */
		Category = 1 << 11,

		/** Field contains information about its order relative to other fields. Retrieve the order field style for information about the order. */
		Order = 1 << 12,

		/**
		 * Signifies that the field containing a class/struct should display the child fields of that objects as if they were part of the
		 * parent class in the inspector.
		 */
		Inline = 1 << 13,

		/** Signifies that a resource reference should be loaded when assigned to field through the inspector. */
		LoadOnAssign = 1 << 14,

		/** Field containing a color that supports high dynamic range. */
		HDR = 1 << 15,
	};

	typedef Flags<ManagedFieldMetaDataFlag> ManagedFieldMetaDataFlags;
	B3D_FLAGS_OPERATORS(ManagedFieldMetaDataFlag);

	/** Flags that are used to further desribe a type of a managed serializable object. */
	enum class B3D_SCRIPT_EXPORT() ManagedObjectMetaDataFlag
	{
		Serializable = 1 << 0,
		Inspectable = 1 << 1
	};

	typedef Flags<ManagedObjectMetaDataFlag> ManagedObjectMetaDataFlags;
	B3D_FLAGS_OPERATORS(ManagedObjectMetaDataFlag);

	/**	Contains information about a type of a managed object. */
	class B3D_SCRIPT_INTEROP_EXPORT B3D_SCRIPT_EXPORT() ManagedTypeInfo : public IReflectable, public IScriptExportable
	{
	public:
		virtual ~ManagedTypeInfo() = default;

		/**	Checks if the current type matches the provided type. */
		B3D_SCRIPT_EXPORT()
		virtual bool Matches(const TShared<ManagedTypeInfo>& typeInfo) const = 0;

		/**
		 * Checks does the managed type this object represents still exists.
		 *
		 * @note	For example if assemblies get refreshed user could have renamed or removed some types.
		 */
		B3D_SCRIPT_EXPORT()
		virtual bool IsTypeLoaded() const = 0;

		B3D_SCRIPT_EXPORT()
		MonoReflectionType* GetReflectionType() const;

		/**
		 * Returns the internal managed class of the type this object represents. Returns null if the type doesn't exist.
		 */
		virtual ::MonoClass* GetMonoClass() const = 0;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ManagedTypeInfoRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/**	Contains information about a type of a managed primitive (for example int, float, etc.). */
	class B3D_SCRIPT_INTEROP_EXPORT B3D_SCRIPT_EXPORT() ManagedTypeInfoPrimitive : public ManagedTypeInfo
	{
	public:
		bool Matches(const TShared<ManagedTypeInfo>& typeInfo) const override;
		bool IsTypeLoaded() const override;
		::MonoClass* GetMonoClass() const override;

		B3D_SCRIPT_EXPORT()
		ManagedPrimitiveType PrimitiveType;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ManagedTypeInfoPrimitiveRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/**	Contains information about a type of a managed enum. */
	class B3D_SCRIPT_INTEROP_EXPORT B3D_SCRIPT_EXPORT() ManagedTypeInfoEnum : public ManagedTypeInfo
	{
	public:
		bool Matches(const TShared<ManagedTypeInfo>& typeInfo) const override;
		bool IsTypeLoaded() const override;
		::MonoClass* GetMonoClass() const override;

		B3D_SCRIPT_EXPORT()
		ManagedPrimitiveType UnderlyingType;

		B3D_SCRIPT_EXPORT()
		String TypeNamespace;

		B3D_SCRIPT_EXPORT()
		String TypeName;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ManagedTypeInfoEnumRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/**	Contains information about a type of a managed game object or resource. */
	class B3D_SCRIPT_INTEROP_EXPORT B3D_SCRIPT_EXPORT() ManagedTypeInfoReference : public ManagedTypeInfo
	{
	public:
		bool Matches(const TShared<ManagedTypeInfo>& typeInfo) const override;
		bool IsTypeLoaded() const override;
		::MonoClass* GetMonoClass() const override;

		B3D_SCRIPT_EXPORT()
		ManagedReferenceType ReferenceType;

		B3D_SCRIPT_EXPORT()
		u32 TypeRTTIId;

		B3D_SCRIPT_EXPORT()
		String TypeNamespace;

		B3D_SCRIPT_EXPORT()
		String TypeName;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ManagedTypeInfoReferenceRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/**	Contains information about a type of a reference to a resource. */
	class B3D_SCRIPT_INTEROP_EXPORT B3D_SCRIPT_EXPORT() ManagedTypeInfoResourceReference : public ManagedTypeInfo
	{
	public:
		bool Matches(const TShared<ManagedTypeInfo>& typeInfo) const override;
		bool IsTypeLoaded() const override;
		::MonoClass* GetMonoClass() const override;

		B3D_SCRIPT_EXPORT()
		TShared<ManagedTypeInfo> ResourceType;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ManagedTypeInfoResourceReferenceRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/**	Contains information about a type of a generic managed object (for example struct or class). */
	class B3D_SCRIPT_INTEROP_EXPORT B3D_SCRIPT_EXPORT() ManagedTypeInfoObject : public ManagedTypeInfo
	{
	public:
		bool Matches(const TShared<ManagedTypeInfo>& typeInfo) const override;
		bool IsTypeLoaded() const override;
		::MonoClass* GetMonoClass() const override;

		B3D_SCRIPT_EXPORT()
		String TypeNamespace;

		B3D_SCRIPT_EXPORT()
		String TypeName;

		B3D_SCRIPT_EXPORT()
		bool IsValueType;

		B3D_SCRIPT_EXPORT()
		u32 TypeRTTIId;

		B3D_SCRIPT_EXPORT()
		ManagedObjectMetaDataFlags MetaDataFlags;

		u32 TypeId;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ManagedTypeInfoObjectRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/**	Contains information about a type of a managed Array. */
	class B3D_SCRIPT_INTEROP_EXPORT B3D_SCRIPT_EXPORT() ManagedTypeInfoArray : public ManagedTypeInfo
	{
	public:
		bool Matches(const TShared<ManagedTypeInfo>& typeInfo) const override;
		bool IsTypeLoaded() const override;
		::MonoClass* GetMonoClass() const override;

		B3D_SCRIPT_EXPORT()
		TShared<ManagedTypeInfo> ElementType;

		B3D_SCRIPT_EXPORT()
		u32 Rank;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ManagedTypeInfoArrayRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/**	Contains information about a type of a managed List. */
	class B3D_SCRIPT_INTEROP_EXPORT B3D_SCRIPT_EXPORT() ManagedTypeInfoList : public ManagedTypeInfo
	{
	public:
		bool Matches(const TShared<ManagedTypeInfo>& typeInfo) const override;
		bool IsTypeLoaded() const override;
		::MonoClass* GetMonoClass() const override;

		B3D_SCRIPT_EXPORT()
		TShared<ManagedTypeInfo> ElementType;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ManagedTypeInfoListRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/**	Contains information about a type of a managed Dictionary. */
	class B3D_SCRIPT_INTEROP_EXPORT B3D_SCRIPT_EXPORT() ManagedTypeInfoDictionary : public ManagedTypeInfo
	{
	public:
		bool Matches(const TShared<ManagedTypeInfo>& typeInfo) const override;
		bool IsTypeLoaded() const override;
		::MonoClass* GetMonoClass() const override;

		B3D_SCRIPT_EXPORT()
		TShared<ManagedTypeInfo> KeyType;

		B3D_SCRIPT_EXPORT()
		TShared<ManagedTypeInfo> ValueType;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ManagedTypeInfoDictionaryRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** Contains information about a style of a serializable field. */
	struct B3D_SCRIPT_EXPORT(ExportAsStruct(true)) ManagedMemberStyle
	{
		/** Returns the lower bound of the range. Only relevant if @see hasRange is true. */
		float RangeMin = 0;

		/** Returns the upper bound of the range. Only relevant if @see hasRange is true. */
		float RangeMax = 0;

		/** Minimum increment the field value can be increment/decremented by. Only relevant if @see hasStep is true. */
		float StepIncrement = 0;

		/** If true, number fields will be displayed as sliders instead of regular input boxes. */
		bool DisplayAsSlider = false;

		/** Name of the category to display in inspector, if the member is part of one. */
		String CategoryName;

		/** Determines ordering in inspector relative to other members. */
		int Order = 0;
	};

	/**	Contains data about a single member (field or property) in a managed object (class or struct). */
	class B3D_SCRIPT_INTEROP_EXPORT B3D_SCRIPT_EXPORT() ManagedMemberInfo : public IReflectable, public IScriptExportable
	{
	public:
		ManagedMemberInfo() = default;
		virtual ~ManagedMemberInfo() = default;

		/**	Determines should the member be serialized when serializing the parent object. */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(IsSerializable))
		bool IsSerializable() const { return MetaDataFlags.IsSet(ManagedFieldMetaDataFlag::Serializable); }

		/** Parses style attributes for this members and returns a structure holding all the relevant style information. */
		B3D_SCRIPT_EXPORT()
		ManagedMemberStyle ParseStyle() const;

		/**
		 * Returns a boxed value contained in the member in the specified object instance.
		 *
		 * @param	instance	Object instance to access the member on.
		 * @return				A boxed value of the member.
		 */
		B3D_SCRIPT_EXPORT()
		virtual MonoObject* GetValue(MonoObject* instance) const = 0;

		/**
		 * Sets a value of the member in the specified object instance.
		 *
		 * @param	instance	Object instance to access the member on.
		 * @param	value		Value to set on the member. For value type it should be a pointer to the value and for
		 *						reference type it should be a pointer to MonoObject.
		 */
		virtual void SetUnboxedValue(MonoObject* instance, void* value) const = 0;

		/**
		 * Sets a value of the member in the specified object instance.
		 *
		 * @param	instance	Object instance to access the member on.
		 * @param	value		Boxed value to set on the member. 
		 */
		B3D_SCRIPT_EXPORT()
		void SetValue(MonoObject* instance, MonoObject* value) const;

		/**
		 * Checks if the attribute of the provided type exists on the member and returns it, or returns null if the
		 * attribute is not present.
		 */
		virtual MonoObject* GetAttribute(MonoClass* monoClass) const = 0;

		B3D_SCRIPT_EXPORT()
		String Name;

		B3D_SCRIPT_EXPORT()
		TShared<ManagedTypeInfo> TypeInfo;

		B3D_SCRIPT_EXPORT()
		ManagedFieldMetaDataFlags MetaDataFlags;

		u32 FieldId = 0;
		u32 ParentTypeId;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ManagedMemberInfoRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/**	Contains data about a single field in a managed object (class or struct). */
	class B3D_SCRIPT_INTEROP_EXPORT B3D_SCRIPT_EXPORT() ManagedFieldInfo : public ManagedMemberInfo
	{
	public:
		ManagedFieldInfo() = default;

		MonoObject* GetAttribute(MonoClass* monoClass) const override;
		MonoObject* GetValue(MonoObject* instance) const override;
		void SetUnboxedValue(MonoObject* instance, void* value) const override;

		MonoField* ScriptField = nullptr;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ManagedFieldInfoRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/**	Contains data about a single property in a managed object (class or struct). */
	class B3D_SCRIPT_INTEROP_EXPORT B3D_SCRIPT_EXPORT() ManagedPropertyInfo : public ManagedMemberInfo
	{
	public:
		ManagedPropertyInfo() = default;

		MonoObject* GetAttribute(MonoClass* monoClass) const override;
		MonoObject* GetValue(MonoObject* instance) const override;
		void SetUnboxedValue(MonoObject* instance, void* value) const override;

		MonoProperty* ScriptProperty = nullptr;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ManagedPropertyInfoRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/**
	 * Contains data about serializable fields of a managed object, and the object's class hierarchy if it belongs to one.
	 * All public fields are by default serializable if their type support serialization. Type is serializable if it has the SerializeObject attribute,
	 * or is one of the built-in supported serializable types (primitive such as int or bool, game object, resource or resource reference).
	 * Array/List/Dictionary of serializable types is also considered serializable.
	 * Public field can be made non-serializable via the DontSerializeField attribute.
	 * Private/protected/internal field can be made serializable by specifying the SerializeField attribute.
	 */
	class B3D_SCRIPT_INTEROP_EXPORT B3D_SCRIPT_EXPORT() ManagedObjectInfo : public IReflectable, public IScriptExportable
	{
	public:
		ManagedObjectInfo() = default;

		/** Returns the managed type name of the object's type, including the namespace in format "namespace.typename". */
		String GetFullTypeName() const { return TypeInfo->TypeNamespace + "." + TypeInfo->TypeName; }

		B3D_SCRIPT_EXPORT()
		MonoReflectionType* GetReflectionType() const;

		/**
		 * Attempts to find a field part of this object that matches the provided parameters.
		 *
		 * @param[in]	fieldInfo		Object describing the managed field. Normally this will be a field that was
		 *								deserialized and you need to ensure it still exists in its parent type, while
		 *								retrieving the new field info.
		 * @param[in]	fieldTypeInfo	Type information about the type containing the object. Used for debug purposes to
		 *								ensure the current object's type matches.
		 * @return						Found field info within this object, or null if not found.
		 */
		TShared<ManagedMemberInfo> FindMatchingField(const TShared<ManagedMemberInfo>& fieldInfo, const TShared<ManagedTypeInfo>& fieldTypeInfo) const;

		B3D_SCRIPT_EXPORT()
		TShared<ManagedTypeInfoObject> TypeInfo;
		MonoClass* ScriptClass = nullptr;

		UnorderedMap<String, u32> MemberNameToIndex;

		B3D_SCRIPT_EXPORT()
		Vector<TShared<ManagedMemberInfo>> Members;

		B3D_SCRIPT_EXPORT()
		TShared<ManagedObjectInfo> BaseClass;
		Vector<std::weak_ptr<ManagedObjectInfo>> DerivedClasses;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ManagedObjectInfoRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/**	Contains information about all managed serializable objects in a specific managed assembly. Object is considered serializable if it has the SerializeObject attribute. */
	class B3D_SCRIPT_INTEROP_EXPORT ManagedAssemblyInfo : public IReflectable
	{
	public:
		String Name;

		UnorderedMap<String, u32> TypeNameToId;
		UnorderedMap<u32, TShared<ManagedObjectInfo>> ObjectInfos;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ManagedAssemblyInfoRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** @} */
} // namespace b3d
