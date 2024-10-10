//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Reflection/BsIReflectable.h"

namespace bs
{
	/** @addtogroup bsfScript
	 *  @{
	 */

	/**	Valid serializable script types. */
	enum class ScriptPrimitiveType
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
	enum class ScriptReferenceType
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
	enum class ScriptFieldFlag
	{
		Serializable = 1 << 0,
		Inspectable = 1 << 1,
		Range = 1 << 2,
		Step = 1 << 3,
		Animable = 1 << 4,
		AsLayerMask = 1 << 5,
		PassByCopy = 1 << 6,
		NotNull = 1 << 7,
		NativeWrapper = 1 << 8,
		ApplyOnDirty = 1 << 9,
		AsQuaternion = 1 << 10,
		Category = 1 << 11,
		Order = 1 << 12,
		Inline = 1 << 13,
		LoadOnAssign = 1 << 14,
		HDR = 1 << 15,
	};

	typedef Flags<ScriptFieldFlag> ScriptFieldFlags;
	B3D_FLAGS_OPERATORS(ScriptFieldFlag);

	/** Flags that are used to further desribe a type of a managed serializable object. */
	enum class ScriptTypeFlag
	{
		Serializable = 1 << 0,
		Inspectable = 1 << 1
	};

	typedef Flags<ScriptTypeFlag> ScriptTypeFlags;
	B3D_FLAGS_OPERATORS(ScriptTypeFlag);

	/**	Contains information about a type of a managed object. */
	class B3D_SCRIPT_INTEROP_EXPORT ManagedTypeInfo : public IReflectable
	{
	public:
		virtual ~ManagedTypeInfo() = default;

		/**	Checks if the current type matches the provided type. */
		virtual bool Matches(const SPtr<ManagedTypeInfo>& typeInfo) const = 0;

		/**
		 * Checks does the managed type this object represents still exists.
		 *
		 * @note	For example if assemblies get refreshed user could have renamed or removed some types.
		 */
		virtual bool IsTypeLoaded() const = 0;

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
	class B3D_SCRIPT_INTEROP_EXPORT ManagedTypeInfoPrimitive : public ManagedTypeInfo
	{
	public:
		bool Matches(const SPtr<ManagedTypeInfo>& typeInfo) const override;
		bool IsTypeLoaded() const override;
		::MonoClass* GetMonoClass() const override;

		ScriptPrimitiveType Type;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ManagedTypeInfoPrimitiveRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/**	Contains information about a type of a managed enum. */
	class B3D_SCRIPT_INTEROP_EXPORT ManagedTypeInfoEnum : public ManagedTypeInfo
	{
	public:
		bool Matches(const SPtr<ManagedTypeInfo>& typeInfo) const override;
		bool IsTypeLoaded() const override;
		::MonoClass* GetMonoClass() const override;

		ScriptPrimitiveType UnderlyingType;
		String TypeNamespace;
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
	class B3D_SCRIPT_INTEROP_EXPORT ManagedTypeInfoReference : public ManagedTypeInfo
	{
	public:
		bool Matches(const SPtr<ManagedTypeInfo>& typeInfo) const override;
		bool IsTypeLoaded() const override;
		::MonoClass* GetMonoClass() const override;

		ScriptReferenceType Type;
		u32 RtiiTypeId;
		String TypeNamespace;
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
	class B3D_SCRIPT_INTEROP_EXPORT ManagedTypeInfoResourceReference : public ManagedTypeInfo
	{
	public:
		bool Matches(const SPtr<ManagedTypeInfo>& typeInfo) const override;
		bool IsTypeLoaded() const override;
		::MonoClass* GetMonoClass() const override;

		SPtr<ManagedTypeInfo> ResourceType;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ManagedTypeInfoResourceReferenceRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/**	Contains information about a type of a generic managed object (for example struct or class). */
	class B3D_SCRIPT_INTEROP_EXPORT ManagedTypeInfoObject : public ManagedTypeInfo
	{
	public:
		bool Matches(const SPtr<ManagedTypeInfo>& typeInfo) const override;
		bool IsTypeLoaded() const override;
		::MonoClass* GetMonoClass() const override;

		String TypeNamespace;
		String TypeName;
		bool ValueType;
		u32 RttiTypeId;
		ScriptTypeFlags Flags;
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
	class B3D_SCRIPT_INTEROP_EXPORT ManagedTypeInfoArray : public ManagedTypeInfo
	{
	public:
		bool Matches(const SPtr<ManagedTypeInfo>& typeInfo) const override;
		bool IsTypeLoaded() const override;
		::MonoClass* GetMonoClass() const override;

		SPtr<ManagedTypeInfo> ElementType;
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
	class B3D_SCRIPT_INTEROP_EXPORT ManagedTypeInfoList : public ManagedTypeInfo
	{
	public:
		bool Matches(const SPtr<ManagedTypeInfo>& typeInfo) const override;
		bool IsTypeLoaded() const override;
		::MonoClass* GetMonoClass() const override;

		SPtr<ManagedTypeInfo> ElementType;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ManagedTypeInfoListRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/**	Contains information about a type of a managed Dictionary. */
	class B3D_SCRIPT_INTEROP_EXPORT ManagedTypeInfoDictionary : public ManagedTypeInfo
	{
	public:
		bool Matches(const SPtr<ManagedTypeInfo>& typeInfo) const override;
		bool IsTypeLoaded() const override;
		::MonoClass* GetMonoClass() const override;

		SPtr<ManagedTypeInfo> KeyType;
		SPtr<ManagedTypeInfo> ValueType;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ManagedTypeInfoDictionaryRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/**	Contains data about a single member (field or property) in a managed object (class or struct). */
	class B3D_SCRIPT_INTEROP_EXPORT ManagedMemberInfo : public IReflectable
	{
	public:
		ManagedMemberInfo() = default;
		virtual ~ManagedMemberInfo() = default;

		/**	Determines should the member be serialized when serializing the parent object. */
		bool IsSerializable() const { return Flags.IsSet(ScriptFieldFlag::Serializable); }

		/**
		 * Returns a boxed value contained in the member in the specified object instance.
		 *
		 * @param[in]	instance	Object instance to access the member on.
		 * @return					A boxed value of the member.
		 */
		virtual MonoObject* GetValue(MonoObject* instance) const = 0;

		/**
		 * Sets a value of the member in the specified object instance.
		 *
		 * @param[in]	instance	Object instance to access the member on.
		 * @param[in]	value		Value to set on the property. For value type it should be a pointer to the value and for
		 *							reference type it should be a pointer to MonoObject.
		 */
		virtual void SetValue(MonoObject* instance, void* value) const = 0;

		/**
		 * Checks if the attribute of the provided type exists on the member and returns it, or returns null if the
		 * attribute is not present.
		 */
		virtual MonoObject* GetAttribute(MonoClass* monoClass) = 0;

		String Name;
		u32 FieldId = 0;
		u32 ParentTypeId;

		SPtr<ManagedTypeInfo> TypeInfo;
		ScriptFieldFlags Flags;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ManagedMemberInfoRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/**	Contains data about a single field in a managed object (class or struct). */
	class B3D_SCRIPT_INTEROP_EXPORT ManagedFieldInfo : public ManagedMemberInfo
	{
	public:
		ManagedFieldInfo() = default;

		MonoObject* GetAttribute(MonoClass* monoClass) override;
		MonoObject* GetValue(MonoObject* instance) const override;
		void SetValue(MonoObject* instance, void* value) const override;

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
	class B3D_SCRIPT_INTEROP_EXPORT ManagedPropertyInfo : public ManagedMemberInfo
	{
	public:
		ManagedPropertyInfo() = default;

		MonoObject* GetAttribute(MonoClass* monoClass) override;
		MonoObject* GetValue(MonoObject* instance) const override;
		void SetValue(MonoObject* instance, void* value) const override;

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
	class B3D_SCRIPT_INTEROP_EXPORT ManagedObjectInfo : public IReflectable
	{
	public:
		ManagedObjectInfo() = default;

		/** Returns the managed type name of the object's type, including the namespace in format "namespace.typename". */
		String GetFullTypeName() const { return TypeInfo->TypeNamespace + "." + TypeInfo->TypeName; }

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
		SPtr<ManagedMemberInfo> FindMatchingField(const SPtr<ManagedMemberInfo>& fieldInfo, const SPtr<ManagedTypeInfo>& fieldTypeInfo) const;

		SPtr<ManagedTypeInfoObject> TypeInfo;
		MonoClass* ScriptClass = nullptr;

		UnorderedMap<String, u32> FieldNameToId;
		UnorderedMap<u32, SPtr<ManagedMemberInfo>> Fields;

		SPtr<ManagedObjectInfo> BaseClass;
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
		UnorderedMap<u32, SPtr<ManagedObjectInfo>> ObjectInfos;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ManagedAssemblyInfoRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** @} */
} // namespace bs
