//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsIReflectable.h"

namespace BansheeEngine
{
	/** @addtogroup SBansheeEngine
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
		Texture2D,
		Texture3D,
		TextureCube,
		SpriteTexture,
		ManagedResource,
		PlainText,
		ScriptCode,
		Shader,
		ShaderInclude,
		Material,
		Mesh,
		Prefab,
		Font,
		StringTable,
		GUISkin,
		SceneObject,
		Component,
		PhysicsMaterial,
		PhysicsMesh,
		AudioClip,
		AnimationClip,
		ManagedComponent,
		Resource,
		Count // Keep at end
	};

	/**	Flags that are used to further define a field in a managed serializable object. */
	enum class ScriptFieldFlags
	{
		Serializable = 0x01,
		Inspectable = 0x02,
		Range = 0x04,
		Step = 0x08
	};

	/**	Contains information about a type of a managed serializable object. */
	class BS_SCR_BE_EXPORT ManagedSerializableTypeInfo : public IReflectable
	{
	public:
		virtual ~ManagedSerializableTypeInfo() {}

		/**	Checks if the current type matches the provided type. */
		virtual bool matches(const SPtr<ManagedSerializableTypeInfo>& typeInfo) const = 0;

		/**
		 * Checks does the managed type this object represents still exists.
		 *
		 * @note	For example if assemblies get refreshed user could have renamed or removed some types.
		 */
		virtual bool isTypeLoaded() const = 0;

		/**
		 * Returns the internal managed class of the type this object represents. Returns null if the type doesn't exist.
		 */
		virtual ::MonoClass* getMonoClass() const = 0;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ManagedSerializableTypeInfoRTTI;
		static RTTITypeBase* getRTTIStatic();
		virtual RTTITypeBase* getRTTI() const override;
	};

	/**	Contains information about a type of a managed serializable primitive (for example int, float, etc.). */
	class BS_SCR_BE_EXPORT ManagedSerializableTypeInfoPrimitive : public ManagedSerializableTypeInfo
	{
	public:
		/** @copydoc ManagedSerializableTypeInfo::matches */
		bool matches(const SPtr<ManagedSerializableTypeInfo>& typeInfo) const override;

		/** @copydoc ManagedSerializableTypeInfo::isTypeLoaded */
		bool isTypeLoaded() const override;

		/** @copydoc ManagedSerializableTypeInfo::getMonoClass */
		::MonoClass* getMonoClass() const override;

		ScriptPrimitiveType mType;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ManagedSerializableTypeInfoPrimitiveRTTI;
		static RTTITypeBase* getRTTIStatic();
		virtual RTTITypeBase* getRTTI() const override;
	};

	/**	Contains information about a type of a managed serializable game object or resource reference. */
	class BS_SCR_BE_EXPORT ManagedSerializableTypeInfoRef : public ManagedSerializableTypeInfo
	{
	public:
		/** @copydoc ManagedSerializableTypeInfo::matches */
		bool matches(const SPtr<ManagedSerializableTypeInfo>& typeInfo) const override;

		/** @copydoc ManagedSerializableTypeInfo::isTypeLoaded */
		bool isTypeLoaded() const override;

		/** @copydoc ManagedSerializableTypeInfo::getMonoClass */
		::MonoClass* getMonoClass() const override;

		ScriptReferenceType mType;
		String mTypeNamespace;
		String mTypeName;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ManagedSerializableTypeInfoRefRTTI;
		static RTTITypeBase* getRTTIStatic();
		virtual RTTITypeBase* getRTTI() const override;
	};

	/**	Contains information about a type of a managed serializable complex object (for example struct or class). */
	class BS_SCR_BE_EXPORT ManagedSerializableTypeInfoObject : public ManagedSerializableTypeInfo
	{
	public:
		/** @copydoc ManagedSerializableTypeInfo::matches */
		bool matches(const SPtr<ManagedSerializableTypeInfo>& typeInfo) const override;

		/** @copydoc ManagedSerializableTypeInfo::isTypeLoaded */
		bool isTypeLoaded() const override;

		/** @copydoc ManagedSerializableTypeInfo::getMonoClass */
		::MonoClass* getMonoClass() const override;

		String mTypeNamespace;
		String mTypeName;
		bool mValueType;
		UINT32 mTypeId;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ManagedSerializableTypeInfoObjectRTTI;
		static RTTITypeBase* getRTTIStatic();
		virtual RTTITypeBase* getRTTI() const override;
	};

	/**	Contains information about a type of a managed serializable Array. */
	class BS_SCR_BE_EXPORT ManagedSerializableTypeInfoArray : public ManagedSerializableTypeInfo
	{
	public:
		/** @copydoc ManagedSerializableTypeInfo::matches */
		bool matches(const SPtr<ManagedSerializableTypeInfo>& typeInfo) const override;

		/** @copydoc ManagedSerializableTypeInfo::isTypeLoaded */
		bool isTypeLoaded() const override;

		/** @copydoc ManagedSerializableTypeInfo::getMonoClass */
		::MonoClass* getMonoClass() const override;

		SPtr<ManagedSerializableTypeInfo> mElementType;
		UINT32 mRank;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ManagedSerializableTypeInfoArrayRTTI;
		static RTTITypeBase* getRTTIStatic();
		virtual RTTITypeBase* getRTTI() const override;
	};

	/**	Contains information about a type of a managed serializable List. */
	class BS_SCR_BE_EXPORT ManagedSerializableTypeInfoList : public ManagedSerializableTypeInfo
	{
	public:
		/** @copydoc ManagedSerializableTypeInfo::matches */
		bool matches(const SPtr<ManagedSerializableTypeInfo>& typeInfo) const override;

		/** @copydoc ManagedSerializableTypeInfo::isTypeLoaded */
		bool isTypeLoaded() const override;

		/** @copydoc ManagedSerializableTypeInfo::getMonoClass */
		::MonoClass* getMonoClass() const override;

		SPtr<ManagedSerializableTypeInfo> mElementType;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ManagedSerializableTypeInfoListRTTI;
		static RTTITypeBase* getRTTIStatic();
		virtual RTTITypeBase* getRTTI() const override;
	};

	/**	Contains information about a type of a managed serializable Dictionary. */
	class BS_SCR_BE_EXPORT ManagedSerializableTypeInfoDictionary : public ManagedSerializableTypeInfo
	{
	public:
		/** @copydoc ManagedSerializableTypeInfo::matches */
		bool matches(const SPtr<ManagedSerializableTypeInfo>& typeInfo) const override;

		/** @copydoc ManagedSerializableTypeInfo::isTypeLoaded */
		bool isTypeLoaded() const override;

		/** @copydoc ManagedSerializableTypeInfo::getMonoClass */
		::MonoClass* getMonoClass() const override;

		SPtr<ManagedSerializableTypeInfo> mKeyType;
		SPtr<ManagedSerializableTypeInfo> mValueType;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ManagedSerializableTypeInfoDictionaryRTTI;
		static RTTITypeBase* getRTTIStatic();
		virtual RTTITypeBase* getRTTI() const override;
	};

	/**	Contains data about a single field in a managed complex object. */
	class BS_SCR_BE_EXPORT ManagedSerializableFieldInfo : public IReflectable
	{
	public:
		ManagedSerializableFieldInfo();

		/**	Determines should the field be serialized when serializing the parent object. */
		bool isSerializable() const { return ((UINT32)mFlags & (UINT32)ScriptFieldFlags::Serializable) != 0; }

		/** Returns the minimum value associated to a Range attribute */
		float getRangeMinimum() const;

		/** Returns the maximum value associated to a Range attribute */
		float getRangeMaximum() const;

		/** Returns the step value of a Range attribute */
		float getStep() const;

		String mName;
		UINT32 mFieldId;
		UINT32 mParentTypeId;

		SPtr<ManagedSerializableTypeInfo> mTypeInfo;
		ScriptFieldFlags mFlags;

		MonoField* mMonoField;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ManagedSerializableFieldInfoRTTI;
		static RTTITypeBase* getRTTIStatic();
		virtual RTTITypeBase* getRTTI() const override;
	};

	/** Contains data about fields of a complex object, and the object's class hierarchy if it belongs to one. */
	class BS_SCR_BE_EXPORT ManagedSerializableObjectInfo : public IReflectable
	{
	public:
		ManagedSerializableObjectInfo();

		/** Returns the managed type name of the object's type, including the namespace in format "namespace.typename". */
		String getFullTypeName() const { return mTypeInfo->mTypeNamespace + "." + mTypeInfo->mTypeName; }

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
		SPtr<ManagedSerializableFieldInfo> findMatchingField(const SPtr<ManagedSerializableFieldInfo>& fieldInfo,
			const SPtr<ManagedSerializableTypeInfo>& fieldTypeInfo) const;

		SPtr<ManagedSerializableTypeInfoObject> mTypeInfo;
		MonoClass* mMonoClass;

		UnorderedMap<String, UINT32> mFieldNameToId;
		UnorderedMap<UINT32, SPtr<ManagedSerializableFieldInfo>> mFields;

		SPtr<ManagedSerializableObjectInfo> mBaseClass;
		Vector<std::weak_ptr<ManagedSerializableObjectInfo>> mDerivedClasses;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ManagedSerializableObjectInfoRTTI;
		static RTTITypeBase* getRTTIStatic();
		virtual RTTITypeBase* getRTTI() const override;
	};

	/**	Contains information about all managed serializable objects in a specific managed assembly. */
	class BS_SCR_BE_EXPORT ManagedSerializableAssemblyInfo : public IReflectable
	{
	public:
		String mName;

		UnorderedMap<String, UINT32> mTypeNameToId;
		UnorderedMap<UINT32, SPtr<ManagedSerializableObjectInfo>> mObjectInfos;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ManagedSerializableAssemblyInfoRTTI;
		static RTTITypeBase* getRTTIStatic();
		virtual RTTITypeBase* getRTTI() const override;
	};

	/** @} */
}