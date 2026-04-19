//********************************* B3D Framework - Copyright 2026 Marko Pintera *****************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DMonoPrerequisites.h"

// DLL export
#if B3D_PLATFORM_WIN32 // Windows
#	if B3D_COMPILER_MSVC
#		if defined(BS_SCR_BE_STATIC_LIB) || defined(B3D_CODEGEN)
#			define B3D_SCRIPT_INTEROP_EXPORT
#		else
#			if defined(B3D_SCRIPT_INTEROP_EXPORTS)
#				define B3D_SCRIPT_INTEROP_EXPORT __declspec(dllexport)
#			else
#				define B3D_SCRIPT_INTEROP_EXPORT __declspec(dllimport)
#			endif
#		endif
#	else
#		if defined(BS_SCR_BE_STATIC_LIB) || defined(B3D_CODEGEN)
#			define B3D_SCRIPT_INTEROP_EXPORT
#		else
#			if defined(B3D_SCRIPT_INTEROP_EXPORTS)
#				define B3D_SCRIPT_INTEROP_EXPORT __attribute__((dllexport))
#			else
#				define B3D_SCRIPT_INTEROP_EXPORT __attribute__((dllimport))
#			endif
#		endif
#	endif
#	define BS_SCR_BE_HIDDEN
#else // Linux/Mac settings
#	define B3D_SCRIPT_INTEROP_EXPORT __attribute__((visibility("default")))
#	define BS_SCR_BE_HIDDEN __attribute__((visibility("hidden")))
#endif

/** @addtogroup Script
 *  @{
 */

/** @defgroup bsfScript EngineScript
 *	Contains script interop objects and other scripting functionality for the engine layer.
 *  @{
 */

/** @defgroup ScriptInteropEngine Interop
 *	Script interop objects for communicating between native code and the managed assembly.
 */

/** @cond RTTI */
/** @defgroup RTTI-Impl-SEngine RTTI types
 *	Types containing RTTI for specific classes.
 */
/** @endcond */

/** @} */
/** @} */

namespace b3d
{
#if !B3D_IS_ENGINE
	constexpr const char* ENGINE_ASSEMBLY = "bsfSharpCore";
#else
	constexpr const char* kEngineAssembly = "MBansheeEngine";
	constexpr const char* kScriptGameAssembly = "MScriptGame";
#endif
	constexpr const char* kEngineNs = "b3d";

	class ScriptObjectBase;
	class ScriptResourceManager;
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
	class ScriptSceneObject;
	class ScriptComponent;
	class ScriptManagedComponent;
	class ScriptManagedResource;
	class ScriptRenderTarget;
	class ScriptRenderTexture;
	class ManagedComponent;
	class ManagedSerializableFieldData;
	class ManagedSerializableFieldKey;
	class ManagedSerializableFieldDataEntry;
	class ManagedTypeInfo;
	class ManagedTypeInfoPrimitive;
	class ManagedTypeInfoObject;
	class ManagedTypeInfoArray;
	class ManagedTypeInfoList;
	class ManagedTypeInfoDictionary;
	class ManagedSerializableObject;
	class ManagedSerializableArray;
	class ManagedSerializableList;
	class ManagedSerializableDictionary;
	class ManagedAssemblyInfo;
	class ManagedObjectInfo;
	class ManagedMemberInfo;
	class ManagedSerializableObjectData;
	class ManagedSerializableDelta;
	class ManagedResource;
	class ManagedResourceMetaData;
	class ScriptSerializableProperty;
	class ScriptAssemblyManager;
	class ScriptLocString;
	class ScriptContextMenu;
	class ScriptGUISkin;
	class ScriptResourceRef;
	class ScriptPhysicsMaterial;
	class ScriptPhysicsMesh;
	class ScriptRigidbody;
	class ScriptColliderBase;
	class ScriptAudioClip;
	class ScriptReflectableBase;
	struct ScriptTypeMetaData;

	typedef TGameObjectHandle<ManagedComponent> HManagedComponent;
	typedef TResourceHandle<ManagedResource> HManagedResource;

	enum TypeID_bsfScript
	{
		TID_ManagedComponent = 50000,
		TID_ScriptSerializableObject = 50001,
		TID_ScriptSerializableArray = 50002,
		TID_ManagedAssemblyInfo = 50004,
		TID_ManagedObjectInfo = 50005,
		TID_ManagedMemberInfo = 50006,
		TID_ManagedTypeInfo = 50007,
		TID_ManagedTypeInfoPrimitive = 50008,
		TID_ManagedTypeInfoObject = 50009,
		TID_ManagedTypeInfoArray = 50010,
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
		TID_ManagedTypeInfoList = 50033,
		TID_ManagedTypeInfoDictionary = 50034,
		TID_ScriptSerializableList = 50035,
		TID_ScriptSerializableDictionary = 50036,
		TID_ManagedResource = 50037,
		TID_ManagedResourceMetaData = 50038,
		TID_ScriptSerializableObjectData = 50039,
		TID_ManagedSerializableDelta = 50040,
		TID_ScriptModification = 50041,
		TID_ScriptModifiedObject = 50042,
		TID_ScriptModifiedArray = 50043,
		TID_ScriptModifiedDictionary = 50044,
		TID_ScriptModifiedEntry = 50045,
		TID_ScriptModifiedField = 50046,
		TID_ScriptModifiedArrayEntry = 50047,
		TID_ScriptModifiedDictionaryEntry = 50048,
		TID_ScriptSerializableDictionaryKeyValue = 50049,
		TID_ManagedTypeInfoReference = 50050,
		TID_ManagedFieldInfo = 50051,
		TID_ManagedPropertyInfo = 50052,
		TID_ManagedTypeInfoResourceReference = 50053,
		TID_ManagedTypeInfoEnum = 50054,
		TID_SerializableFieldDataReflectableRef = 50055,
	};

	/**	Types of resources accessible from script code. */
	enum class B3D_SCRIPT_EXPORT() BuiltinResourceType
	{
		Texture = TID_Texture,
		SpriteTexture = TID_SpriteTexture,
		Mesh = TID_Mesh,
		Font = TID_Font,
		Shader = TID_Shader,
		ShaderInclude = TID_ShaderInclude,
		Material = TID_Material,
		Prefab = TID_Prefab,
		PlainText = TID_PlainText,
		ScriptCode = TID_ScriptCode,
		StringTable = TID_StringTable,
		PhysicsMaterial = TID_PhysicsMaterial,
		PhysicsMesh = TID_PhysicsMesh,
		AudioClip = TID_AudioClip,
		AnimationClip = TID_AnimationClip,
		VectorField = TID_VectorField,
		SpriteImage = TID_SpriteImage,
		SpriteGlyph = TID_SpriteGlyph,
		SpriteVectorPath = TID_SpriteVectorPath,
		VectorPath = TID_VectorPath,
		Undefined = 0
	};

} // namespace b3d
