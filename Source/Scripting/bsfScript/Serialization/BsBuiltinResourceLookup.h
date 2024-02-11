//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#if !BS_SBGEN
#	include "BsScriptEnginePrerequisites.h"
#	include "BsScriptMeta.h"
#	include "BsMonoClass.h"
#	include "Reflection/BsRTTIType.h"

#	include "Image/BsSpriteTexture.h"
#	include "Mesh/BsMesh.h"
#	include "Text/BsFont.h"
#	include "Material/BsShader.h"
#	include "Material/BsShaderInclude.h"
#	include "Material/BsMaterial.h"
#	include "Scene/BsPrefab.h"
#	include "Resources/BsPlainText.h"
#	include "Resources/BsScriptCode.h"
#	include "Localization/BsStringTable.h"
#	include "GUI/BsGUISkin.h"
#	include "Physics/BsPhysicsMaterial.h"
#	include "Physics/BsPhysicsMesh.h"
#	include "Audio/BsAudioClip.h"
#	include "Animation/BsAnimationClip.h"
#	include "Particles/BsVectorField.h"

#	include "Wrappers/BsScriptPlainText.h"
#	include "Wrappers/BsScriptScriptCode.h"
#	include "Wrappers/BsScriptShaderInclude.h"
#	include "Wrappers/BsScriptPrefab.h"
#	include "Wrappers/BsScriptRRefBase.h"

#	include "Generated/BsScriptTexture.generated.h"
#	include "Generated/BsScriptMesh.generated.h"
#	include "Generated/BsScriptPhysicsMesh.generated.h"
#	include "Generated/BsScriptPhysicsMaterial.generated.h"
#	include "Generated/BsScriptAnimationClip.generated.h"
#	include "Generated/BsScriptAudioClip.generated.h"
#	include "Generated/BsScriptShader.generated.h"
#	include "Generated/BsScriptMaterial.generated.h"
#	include "Generated/BsScriptFont.generated.h"
#	include "Generated/BsScriptSpriteTexture.generated.h"
#	include "Generated/BsScriptStringTable.generated.h"
#	include "Generated/BsScriptVectorField.generated.h"
#	include "Generated/BsScriptSpriteGlyph.generated.h"
#	include "Generated/BsScriptSpriteVectorPath.generated.h"
#	include "Generated/BsScriptVectorPath.generated.h"

namespace bs
{
	/** Begins the definition for the builtin resource lookup table. */
#	define LOOKUP_BEGIN                                                                              \
		class BuiltinResourceTypes                                                                    \
		{                                                                                             \
		private:                                                                                      \
			struct META_FirstEntry                                                                    \
			{};                                                                                       \
			static void META_GetPrevEntries(Vector<BuiltinResourceInfo>& entries, META_FirstEntry id) \
			{}                                                                                        \
                                                                                                      \
			typedef META_FirstEntry

	/** Registers a new entry in the resource lookup table. */
#	define ADD_ENTRY(ResourceType, ScriptType, ResourceTypeEnum)                                              \
		META_Entry_##ScriptType;                                                                               \
                                                                                                               \
	public:                                                                                                    \
		static ScriptResourceBase* Create##ScriptType(const HResource& resource, MonoObject* existingInstance) \
		{                                                                                                      \
			MonoObject* managedInstance;                                                                       \
			if(existingInstance != nullptr)                                                                    \
				managedInstance = existingInstance;                                                            \
			else                                                                                               \
				managedInstance = ScriptType::CreateInstance();                                                \
                                                                                                               \
			ResourceHandle<ResourceType> castHandle = B3DStaticResourceCast<ResourceType>(resource);            \
			ScriptType* scriptResource = new(B3DAllocate<ScriptType>()) ScriptType(managedInstance, castHandle);  \
                                                                                                               \
			return scriptResource;                                                                             \
		}                                                                                                      \
                                                                                                               \
		struct META_NextEntry_##ScriptType                                                                     \
		{};                                                                                                    \
		static void META_GetPrevEntries(Vector<BuiltinResourceInfo>& entries, META_NextEntry_##ScriptType id)  \
		{                                                                                                      \
			META_GetPrevEntries(entries, META_Entry_##ScriptType());                                           \
                                                                                                               \
			BuiltinResourceInfo entry;                                                                         \
			entry.MetaData = ScriptType::GetMetaData();                                                        \
			entry.TypeId = ResourceType::GetRttiStatic()->GetRttiId();                                         \
			entry.MonoClass = nullptr;                                                                         \
			entry.ResType = ResourceTypeEnum;                                                                  \
			entry.CreateCallback = &Create##ScriptType;                                                        \
                                                                                                               \
			entries.push_back(entry);                                                                          \
		}                                                                                                      \
                                                                                                               \
		typedef META_NextEntry_##ScriptType

	/** End the definition for the builtin resource lookup table. */
#	define LOOKUP_END                                      \
		META_LastEntry;                                     \
                                                            \
	public:                                                 \
		static Vector<BuiltinResourceInfo> GetEntries()     \
		{                                                   \
			Vector<BuiltinResourceInfo> entries;            \
			META_GetPrevEntries(entries, META_LastEntry()); \
			return entries;                                 \
		}                                                   \
		}                                                   \
		;

	// Note: This resource registration code is only here because resource lookup auto-generation isn't yet hooked up
	LOOKUP_BEGIN
	ADD_ENTRY(Texture, ScriptTexture, ScriptResourceType::Texture)
	ADD_ENTRY(SpriteTexture, ScriptSpriteTexture, ScriptResourceType::SpriteTexture)
	ADD_ENTRY(Mesh, ScriptMesh, ScriptResourceType::Mesh)
	ADD_ENTRY(Font, ScriptFont, ScriptResourceType::Font)
	ADD_ENTRY(Shader, ScriptShader, ScriptResourceType::Shader)
	ADD_ENTRY(ShaderInclude, ScriptShaderInclude, ScriptResourceType::ShaderInclude)
	ADD_ENTRY(Material, ScriptMaterial, ScriptResourceType::Material)
	ADD_ENTRY(Prefab, ScriptPrefab, ScriptResourceType::Prefab)
	ADD_ENTRY(PlainText, ScriptPlainText, ScriptResourceType::PlainText)
	ADD_ENTRY(ScriptCode, ScriptScriptCode, ScriptResourceType::ScriptCode)
	ADD_ENTRY(StringTable, ScriptStringTable, ScriptResourceType::StringTable)
	ADD_ENTRY(PhysicsMaterial, ScriptPhysicsMaterial, ScriptResourceType::PhysicsMaterial)
	ADD_ENTRY(PhysicsMesh, ScriptPhysicsMesh, ScriptResourceType::PhysicsMesh)
	ADD_ENTRY(AudioClip, ScriptAudioClip, ScriptResourceType::AudioClip)
	ADD_ENTRY(AnimationClip, ScriptAnimationClip, ScriptResourceType::AnimationClip)
	ADD_ENTRY(VectorField, ScriptVectorField, ScriptResourceType::VectorField)
	ADD_ENTRY(SpriteImage, ScriptSpriteImage, ScriptResourceType::SpriteImage)
	ADD_ENTRY(SpriteGlyph, ScriptSpriteGlyph, ScriptResourceType::SpriteGlyph)
	ADD_ENTRY(SpriteVectorPath, ScriptSpriteVectorPath, ScriptResourceType::SpriteVectorPath)
	ADD_ENTRY(VectorPath, ScriptVectorPath, ScriptResourceType::VectorPath)
	LOOKUP_END

#	undef LOOKUP_BEGIN
#	undef ADD_ENTRY
#	undef LOOKUP_END

} // namespace bs
#endif
