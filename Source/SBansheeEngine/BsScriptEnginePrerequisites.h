//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsMonoPrerequisites.h"

// DLL export
#if BS_PLATFORM == BS_PLATFORM_WIN32 // Windows
#  if BS_COMPILER == BS_COMPILER_MSVC
#    if defined(BS_STATIC_LIB)
#      define BS_SCR_BE_EXPORT
#    else
#      if defined(BS_SCR_BE_EXPORTS)
#        define BS_SCR_BE_EXPORT __declspec(dllexport)
#      else
#        define BS_SCR_BE_EXPORT __declspec(dllimport)
#      endif
#	 endif
#  else
#    if defined(BS_STATIC_LIB)
#      define BS_SCR_BE_EXPORT
#    else
#      if defined(BS_SCR_BE_EXPORTS)
#        define BS_SCR_BE_EXPORT __attribute__ ((dllexport))
#      else
#        define BS_SCR_BE_EXPORT __attribute__ ((dllimport))
#      endif
#	 endif
#  endif
#  define BS_SCR_BE_HIDDEN
#else // Linux/Mac settings
#  define BS_SCR_BE_EXPORT __attribute__ ((visibility ("default")))
#  define BS_SCR_BE_HIDDEN __attribute__ ((visibility ("hidden")))
#endif

/** @addtogroup Plugins
 *  @{
 */

/** @defgroup SBansheeEngine SBansheeEngine
 *	Contains script interop objects and other scripting functionality for the engine layer.
 *  @{
 */

/** @defgroup ScriptInteropEngine Interop
 *	Script interop objects for communicating between native code and MBansheeEngine managed assembly.
 */

/** @cond RTTI */
/** @defgroup RTTI-Impl-SEngine RTTI types
 *	Types containing RTTI for specific classes.
 */
/** @endcond */

/** @} */
/** @} */

namespace bs
{
	class ScriptObjectBase;
	class ScriptResourceManager;
	class ScriptResourceBase;
	class ScriptFont;
	class ScriptSpriteTexture;
	class ScriptShaderInclude;
	class ScriptTexture;
	class ScriptPlainText;
	class ScriptScriptCode;
	class ScriptShader;
	class ScriptMaterial;
	class ScriptMesh;
	class ScriptPrefab;
	class ScriptStringTable;
	class ScriptGUIElementStyle;
	class ScriptGUIElementStateStyle;
	class ScriptGUILayout;
	class ScriptGUILabel;
	class ScriptGUIScrollArea;
	class ScriptGUIScrollAreaLayout;
	class ScriptGameObjectBase;
	class ScriptSceneObject;
	class ScriptComponentBase;
	class ScriptComponent;
	class ScriptManagedComponent;
	class ScriptManagedResource;
	class ScriptRenderTarget;
	class ScriptRenderTexture;
	class ManagedComponent;
	class ManagedSerializableFieldData;
	class ManagedSerializableFieldKey;
	class ManagedSerializableFieldDataEntry;
	class ManagedSerializableTypeInfo;
	class ManagedSerializableTypeInfoPrimitive;
	class ManagedSerializableTypeInfoObject;
	class ManagedSerializableTypeInfoArray;
	class ManagedSerializableTypeInfoList;
	class ManagedSerializableTypeInfoDictionary;
	class ManagedSerializableObject;
	class ManagedSerializableArray;
	class ManagedSerializableList;
	class ManagedSerializableDictionary;
	class ManagedSerializableAssemblyInfo;
	class ManagedSerializableObjectInfo;
	class ManagedSerializableMemberInfo;
	class ManagedSerializableObjectData;
	class ManagedSerializableDiff;
	class ManagedResource;
	class ManagedResourceMetaData;
	class ScriptSerializableProperty;
	class ScriptAssemblyManager;
	class ScriptHString;
	class ScriptContextMenu;
	class ScriptGUISkin;
	class ScriptResourceRef;
	class ScriptPhysicsMaterial;
	class ScriptPhysicsMesh;
	class ScriptRigidbody;
	class ScriptColliderBase;
	class ScriptAudioClip;
	struct ScriptMeta;

	typedef GameObjectHandle<ManagedComponent> HManagedComponent;
	typedef ResourceHandle<ManagedResource> HManagedResource;

	enum TypeID_BansheeScript
	{
		TID_ManagedComponent = 50000,
		TID_ScriptSerializableObject = 50001,
		TID_ScriptSerializableArray = 50002,
		TID_SerializableAssemblyInfo = 50004,
		TID_SerializableObjectInfo = 50005,
		TID_SerializableMemberInfo = 50006,
		TID_SerializableTypeInfo = 50007,
		TID_SerializableTypeInfoPrimitive = 50008,
		TID_SerializableTypeInfoObject = 50009,
		TID_SerializableTypeInfoArray = 50010,
		TID_SerializableFieldData = 50011,
		TID_SerializableFieldKey = 50012,
		TID_SerializableFieldDataEntry = 50013,
		TID_SerializableFieldDataBool = 50014,
		TID_SerializableFieldDataChar = 50015,
		TID_SerializableFieldDataI8 = 50016,
		TID_SerializableFieldDataU8 = 50017,
		TID_SerializableFieldDataI16 = 50018,
		TID_SerializableFieldDataU16 = 50019,
		TID_SerializableFieldDataI32 = 50020,
		TID_SerializableFieldDataU32 = 50021,
		TID_SerializableFieldDataI64 = 50022,
		TID_SerializableFieldDataU64 = 50023,
		TID_SerializableFieldDataFloat = 50024,
		TID_SerializableFieldDataDouble = 50025,
		TID_SerializableFieldDataString = 50026,
		TID_SerializableFieldDataResourceRef = 50027,
		TID_SerializableFieldDataGameObjectRef = 50028,
		TID_SerializableFieldDataObject = 50029,
		TID_SerializableFieldDataArray = 50030,
		TID_SerializableFieldDataList = 50031,
		TID_SerializableFieldDataDictionary = 50032,
		TID_SerializableTypeInfoList = 50033,
		TID_SerializableTypeInfoDictionary = 50034,
		TID_ScriptSerializableList = 50035,
		TID_ScriptSerializableDictionary = 50036,
		TID_ManagedResource = 50037,
		TID_ManagedResourceMetaData = 50038,
		TID_ScriptSerializableObjectData = 50039,
		TID_ScriptSerializableDiff = 50040,
		TID_ScriptModification = 50041,
		TID_ScriptModifiedObject = 50042,
		TID_ScriptModifiedArray = 50043,
		TID_ScriptModifiedDictionary = 50044,
		TID_ScriptModifiedEntry = 50045,
		TID_ScriptModifiedField = 50046,
		TID_ScriptModifiedArrayEntry = 50047,
		TID_ScriptModifiedDictionaryEntry = 50048,
		TID_ScriptSerializableDictionaryKeyValue = 50049,
		TID_SerializableTypeInfoRef = 50050,
		TID_SerializableFieldInfo = 50051,
		TID_SerializablePropertyInfo = 50052
	};

	/** Information about a builtin component wrapped as a script object. */
	struct BuiltinComponentInfo
	{
		const ScriptMeta* metaData;
		UINT32 typeId;
		MonoClass* monoClass;
		std::function<ScriptComponentBase*(const HComponent&)> createCallback;
	};

	/**	Types of resources accessible from script code. */
	enum class ScriptResourceType // Note: Must be the same as C# enum ResourceType
	{
		Texture, SpriteTexture, Mesh, Font, Shader, ShaderInclude, Material, Prefab,
		PlainText, ScriptCode, StringTable, GUISkin, PhysicsMaterial, PhysicsMesh, AudioClip, AnimationClip, Undefined
	};

	/** Information about a builtin resource wrapped as a script object. */
	struct BuiltinResourceInfo
	{
		const ScriptMeta* metaData;
		UINT32 typeId;
		MonoClass* monoClass;
		ScriptResourceType resType;
		std::function<ScriptResourceBase*(const HResource&, MonoObject*)> createCallback;
	};
}