//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEditorPrerequisites.h"
#include "BsScriptObject.h"
#include "BsPixelData.h"
#include "BsAudioClipImportOptions.h"
#include "BsMeshImportOptions.h"
#include "BsGpuProgram.h"

namespace bs
{
	/** @addtogroup ScriptInteropEditor
	 *  @{
	 */

	enum class FontRenderMode;

	/** Base class for all C++/CLR interop objects wrapping various implementations of ImportOptions. */
	class BS_SCR_BED_EXPORT ScriptImportOptionsBase : public ScriptObjectBase
	{
	public:
		/**	Returns the internal native import options. */
		SPtr<ImportOptions> getImportOptions() const { return mImportOptions; }

	protected:
		ScriptImportOptionsBase(MonoObject* instance);
		virtual ~ScriptImportOptionsBase() {}

		SPtr<ImportOptions> mImportOptions;
	};

	/**	Interop class between C++ & CLR for ImportOptions. */
	class BS_SCR_BED_EXPORT ScriptImportOptions : public ScriptObject <ScriptImportOptions, ScriptImportOptionsBase>
	{
	public:
		SCRIPT_OBJ(EDITOR_ASSEMBLY, "BansheeEditor", "ImportOptions")

		/** Creates a new managed ImportOptions instance containing the provided import options. */
		static MonoObject* create(const SPtr<ImportOptions>& importOptions);

	private:
		ScriptImportOptions(MonoObject* instance);
	};

	/**	Interop class between C++ & CLR for TextureImportOptions. */
	class BS_SCR_BED_EXPORT ScriptTextureImportOptions : public ScriptObject<ScriptTextureImportOptions, ScriptImportOptionsBase>
	{
	public:
		SCRIPT_OBJ(EDITOR_ASSEMBLY, "BansheeEditor", "TextureImportOptions")

		/** Creates a new managed TextureImportOptions instance containing the default import options for textures. */
		static MonoObject* create();

		/**	Creates a new managed TextureImportOptions instance containing the provided import options. */
		static MonoObject* create(const SPtr<TextureImportOptions>& options);

	private:
		ScriptTextureImportOptions(MonoObject* instance);

		/**	Returns the internal native import options. */
		SPtr<TextureImportOptions> getTexImportOptions();

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_CreateInstance(MonoObject* instance);
		static PixelFormat internal_GetPixelFormat(ScriptTextureImportOptions* thisPtr);
		static void internal_SetPixelFormat(ScriptTextureImportOptions* thisPtr, PixelFormat value);
		static bool internal_GetGenerateMipmaps(ScriptTextureImportOptions* thisPtr);
		static void internal_SetGenerateMipmaps(ScriptTextureImportOptions* thisPtr, bool value);
		static UINT32 internal_GetMaxMipmapLevel(ScriptTextureImportOptions* thisPtr);
		static void internal_SetMaxMipmapLevel(ScriptTextureImportOptions* thisPtr, UINT32 value);
		static bool internal_GetCPUReadable(ScriptTextureImportOptions* thisPtr);
		static void internal_SetCPUReadable(ScriptTextureImportOptions* thisPtr, bool value);
		static bool internal_GetCPUCached(ScriptTextureImportOptions* thisPtr);
		static void internal_SetCPUCached(ScriptTextureImportOptions* thisPtr, bool value);
		static bool internal_GetIsSRGB(ScriptTextureImportOptions* thisPtr);
		static void internal_SetIsSRGB(ScriptTextureImportOptions* thisPtr, bool value);
	};

	/**	Interop class between C++ & CLR for MeshImportOptions. */
	class BS_SCR_BED_EXPORT ScriptMeshImportOptions : public ScriptObject<ScriptMeshImportOptions, ScriptImportOptionsBase>
	{
	public:
		SCRIPT_OBJ(EDITOR_ASSEMBLY, "BansheeEditor", "MeshImportOptions")

		/**	Creates a new managed MeshImportOptions instance containing the default import options for textures. */
		static MonoObject* create();

		/** Creates a new managed MeshImportOptions instance containing the provided import options. */
		static MonoObject* create(const SPtr<MeshImportOptions>& options);

	private:
		ScriptMeshImportOptions(MonoObject* instance);

		/**	Returns the internal native import options. */
		SPtr<MeshImportOptions> getMeshImportOptions();

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_CreateInstance(MonoObject* instance);
		static bool internal_GetCPUReadable(ScriptMeshImportOptions* thisPtr);
		static void internal_SetCPUReadable(ScriptMeshImportOptions* thisPtr, bool value);
		static bool internal_GetCPUCached(ScriptMeshImportOptions* thisPtr);
		static void internal_SetCPUCached(ScriptMeshImportOptions* thisPtr, bool value);
		static bool internal_GetImportNormals(ScriptMeshImportOptions* thisPtr);
		static void internal_SetImportNormals(ScriptMeshImportOptions* thisPtr, bool value);
		static bool internal_GetImportTangents(ScriptMeshImportOptions* thisPtr);
		static void internal_SetImportTangents(ScriptMeshImportOptions* thisPtr, bool value);
		static bool internal_GetImportSkin(ScriptMeshImportOptions* thisPtr);
		static void internal_SetImportSkin(ScriptMeshImportOptions* thisPtr, bool value);
		static bool internal_GetImportAnimation(ScriptMeshImportOptions* thisPtr);
		static void internal_SetImportAnimation(ScriptMeshImportOptions* thisPtr, bool value);
		static bool internal_GetImportBlendShapes(ScriptMeshImportOptions* thisPtr);
		static void internal_SetImportBlendShapes(ScriptMeshImportOptions* thisPtr, bool value);
		static bool internal_GetKeyFrameReduction(ScriptMeshImportOptions* thisPtr);
		static void internal_SetKeyFrameReduction(ScriptMeshImportOptions* thisPtr, bool value);
		static bool internal_GetRootMotion(ScriptMeshImportOptions* thisPtr);
		static void internal_SetRootMotion(ScriptMeshImportOptions* thisPtr, bool value);
		static float internal_GetScale(ScriptMeshImportOptions* thisPtr);
		static void internal_SetScale(ScriptMeshImportOptions* thisPtr, float value);
		static int internal_GetCollisionMeshType(ScriptMeshImportOptions* thisPtr);
		static void internal_SetCollisionMeshType(ScriptMeshImportOptions* thisPtr, int value);
		static MonoArray* internal_GetAnimationClipSplits(ScriptMeshImportOptions* thisPtr);
		static void internal_SetAnimationClipSplits(ScriptMeshImportOptions* thisPtr, MonoArray* value);
		static MonoArray* internal_GetAnimationEvents(ScriptMeshImportOptions* thisPtr);
		static void internal_SetAnimationEvents(ScriptMeshImportOptions* thisPtr, MonoArray* value);
	};

	/**	Interop class between C++ & CLR for FontImportOptions. */
	class BS_SCR_BED_EXPORT ScriptFontImportOptions : public ScriptObject <ScriptFontImportOptions, ScriptImportOptionsBase>
	{
	public:
		SCRIPT_OBJ(EDITOR_ASSEMBLY, "BansheeEditor", "FontImportOptions")

		/** Creates a new managed FontImportOptions instance containing the default import options for fonts. */
		static MonoObject* create();

		/**	Creates a new managed FontImportOptions instance containing the provided import options. */
		static MonoObject* create(const SPtr<FontImportOptions>& options);

	private:
		ScriptFontImportOptions(MonoObject* instance);

		/**	Returns the internal native import options. */
		SPtr<FontImportOptions> getFontImportOptions();

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_CreateInstance(MonoObject* instance);
		static MonoArray* internal_GetFontSizes(ScriptFontImportOptions* thisPtr);
		static void internal_SetFontSizes(ScriptFontImportOptions* thisPtr, MonoArray* value);
		static UINT32 internal_GetDPI(ScriptFontImportOptions* thisPtr);
		static void internal_SetDPI(ScriptFontImportOptions* thisPtr, UINT32 value);
		static FontRenderMode internal_GetRenderMode(ScriptFontImportOptions* thisPtr);
		static void internal_SetRenderMode(ScriptFontImportOptions* thisPtr, FontRenderMode value);
		static bool internal_GetBold(ScriptFontImportOptions* thisPtr);
		static void internal_SetBold(ScriptFontImportOptions* thisPtr, bool value);
		static bool internal_GetItalic(ScriptFontImportOptions* thisPtr);
		static void internal_SetItalic(ScriptFontImportOptions* thisPtr, bool value);
		static MonoArray* internal_GetCharRanges(ScriptFontImportOptions* thisPtr);
		static void internal_SetCharRanges(ScriptFontImportOptions* thisPtr, MonoArray* value);
	};

	/**	Interop class between C++ & CLR for ScriptCodeImportOptions. */
	class BS_SCR_BED_EXPORT ScriptScriptCodeImportOptions : public ScriptObject <ScriptScriptCodeImportOptions, ScriptImportOptionsBase>
	{
	public:
		SCRIPT_OBJ(EDITOR_ASSEMBLY, "BansheeEditor", "ScriptCodeImportOptions")

		/**
		 * Creates a new managed ScriptCodeImportOptions instance containing the default import options for script code 
		 * files.
		 */
		static MonoObject* create();

		/** Creates a new managed ScriptCodeImportOptions instance containing the provided import options. */
		static MonoObject* create(const SPtr<ScriptCodeImportOptions>& options);

	private:
		ScriptScriptCodeImportOptions(MonoObject* instance);

		/**	Returns the internal native import options. */
		SPtr<ScriptCodeImportOptions> getCodeImportOptions();

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_CreateInstance(MonoObject* instance);
		static bool internal_IsEditorScript(ScriptScriptCodeImportOptions* thisPtr);
		static void internal_SetEditorScript(ScriptScriptCodeImportOptions* thisPtr, bool value);
	};

	/**	Interop class between C++ & CLR for AudioClipImportOptions. */
	class BS_SCR_BED_EXPORT ScriptAudioClipImportOptions : public ScriptObject <ScriptAudioClipImportOptions, ScriptImportOptionsBase>
	{
	public:
		SCRIPT_OBJ(EDITOR_ASSEMBLY, "BansheeEditor", "AudioClipImportOptions")

		/** Creates a new managed AudioClipImportOptions instance containing the default import options for audio files. */
		static MonoObject* create();

		/** Creates a new managed AudioClipImportOptions instance containing the provided import options. */
		static MonoObject* create(const SPtr<AudioClipImportOptions>& options);

	private:
		ScriptAudioClipImportOptions(MonoObject* instance);

		/**	Returns the internal native import options. */
		SPtr<AudioClipImportOptions> getClipImportOptions();

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_CreateInstance(MonoObject* instance);
		static AudioFormat internal_GetFormat(ScriptAudioClipImportOptions* thisPtr);
		static void internal_SetFormat(ScriptAudioClipImportOptions* thisPtr, AudioFormat format);
		static AudioReadMode internal_GetReadMode(ScriptAudioClipImportOptions* thisPtr);
		static void internal_SetReadMode(ScriptAudioClipImportOptions* thisPtr, AudioReadMode readMode);
		static bool internal_GetIs3D(ScriptAudioClipImportOptions* thisPtr);
		static void internal_SetIs3D(ScriptAudioClipImportOptions* thisPtr, bool is3d);
		static UINT32 internal_GetBitDepth(ScriptAudioClipImportOptions* thisPtr);
		static void internal_SetBitDepth(ScriptAudioClipImportOptions* thisPtr, UINT32 bitDepth);
	};

	/** Helper class for dealing with AnimationSplitInfo structure. */
	class BS_SCR_BED_EXPORT ScriptAnimationSplitInfo : public ScriptObject<ScriptAnimationSplitInfo>
	{
	public:
		SCRIPT_OBJ(EDITOR_ASSEMBLY, "BansheeEditor", "AnimationSplitInfo")

		/** Converts managed split info to its native counterpart. */
		static AnimationSplitInfo fromManaged(MonoObject* object);

		/** Converts native split info to its managed counterpart. */
		static MonoObject* toManaged(const AnimationSplitInfo& splitInfo);

	private:
		ScriptAnimationSplitInfo(MonoObject* instance);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static MonoField* nameField;
		static MonoField* startFrameField;
		static MonoField* endFrameField;
		static MonoField* isAdditiveField;
	};

	/** Helper class for dealing with ImportedAnimationEvents structure. */
	class BS_SCR_BED_EXPORT ScriptImportedAnimationEvents : public ScriptObject<ScriptImportedAnimationEvents>
	{
	public:
		SCRIPT_OBJ(EDITOR_ASSEMBLY, "BansheeEditor", "ImportedAnimationEvents")

		/** Converts managed events info to its native counterpart. */
		static ImportedAnimationEvents fromManaged(MonoObject* object);

		/** Converts native events info to its managed counterpart. */
		static MonoObject* toManaged(const ImportedAnimationEvents& events);

	private:
		ScriptImportedAnimationEvents(MonoObject* instance);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static MonoField* nameField;
		static MonoField* eventsField;
	};

	/** @} */
}