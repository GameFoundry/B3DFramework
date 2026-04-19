//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Importer/B3DImportOptions.h"
#include "Audio/B3DAudioClip.h"

namespace b3d
{
	/** @addtogroup Importer
	 *  @{
	 */

	/** Contains import options you may use to control how an audio clip is imported. */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Importer), API(Framework), API(Editor)) AudioClipImportOptions : public ImportOptions
	{
	public:
		AudioClipImportOptions() = default;

		/** Audio format to import the audio clip as. */
		B3D_SCRIPT_EXPORT()
		AudioFormat Format = AudioFormat::PCM;

		/** Determines how is audio data loaded into memory. */
		B3D_SCRIPT_EXPORT()
		AudioReadMode ReadMode = AudioReadMode::LoadDecompressed;

		/**
		 * Determines should the clip be played as spatial (3D) audio or as normal audio. 3D clips will be converted
		 * to mono on import.
		 */
		B3D_SCRIPT_EXPORT()
		bool Is3D = true;

		/** Size of a single sample in bits. The clip will be converted to this bit depth on import. */
		B3D_SCRIPT_EXPORT()
		u32 BitDepth = 16;

		// Note: Add options to resample to a different frequency

		/** Creates a new import options object that allows you to customize how are audio clips imported. */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(T))
		static SPtr<AudioClipImportOptions> Create();

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class AudioClipImportOptionsRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const;
	};

	/** @} */
} // namespace b3d
