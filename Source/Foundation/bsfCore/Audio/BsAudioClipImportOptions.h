//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Importer/BsImportOptions.h"
#include "Audio/BsAudioClip.h"

namespace bs
{
	/** @addtogroup Importer
	 *  @{
	 */

	/** Contains import options you may use to control how an audio clip is imported. */
	class BS_CORE_EXPORT BS_SCRIPT_EXPORT(m:Importer,api:bsf,api:bed) AudioClipImportOptions : public ImportOptions
	{
	public:
		AudioClipImportOptions() = default;

		/** Audio format to import the audio clip as. */
		BS_SCRIPT_EXPORT()
		AudioFormat format = AudioFormat::PCM;

		/** Determines how is audio data loaded into memory. */
		BS_SCRIPT_EXPORT()
		AudioReadMode readMode = AudioReadMode::LoadDecompressed;

		/**
		 * Determines should the clip be played as spatial (3D) audio or as normal audio. 3D clips will be converted
		 * to mono on import. 
		 */
		BS_SCRIPT_EXPORT()
		bool is3D = true;

		/** Size of a single sample in bits. The clip will be converted to this bit depth on import. */
		BS_SCRIPT_EXPORT()
		UINT32 bitDepth = 16;

		// Note: Add options to resample to a different frequency

		/** Creates a new import options object that allows you to customize how are audio clips imported. */
		BS_SCRIPT_EXPORT(ec:T)
		static SPtr<AudioClipImportOptions> create();

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class AudioClipImportOptionsRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** @} */
}
