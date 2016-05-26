//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsImportOptions.h"
#include "BsAudioClip.h"

namespace BansheeEngine
{
	/** @addtogroup Importer
	 *  @{
	 */

	/** Contains import options you may use to control how an audio clip is imported. */
	class BS_CORE_EXPORT AudioClipImportOptions : public ImportOptions
	{
	public:
		AudioClipImportOptions();

		/** Returns the audio format to import the audio clip as. */
		AudioFormat getFormat() const { return mFormat; }

		/** Sets the audio format to import the audio clip as. */
		void setFormat(AudioFormat format) { mFormat = format; }

		/** Returns read mode that determines how is audio data loaded into memory. */
		AudioReadMode getReadMode() const { return mReadMode; }

		/** Sets read mode that determines how is audio data loaded into memory. */
		void setReadMode(AudioReadMode readMode) { mReadMode = readMode; }

		/** Checks should the clip be played as spatial (3D) audio or as normal audio. */
		bool getIs3D() const { return mIs3D; }

		/** 
		 * Sets whether the clip should be played as spatial (3D) audio or as normal audio. 3D clips will be converted
		 * to mono on import.
		 */
		void setIs3D(bool is3d) { mIs3D = is3d; }

		/** Returns the size of a single sample in bits. */
		UINT32 getBitDepth() const { return mBitDepth; }

		/** Sets the size of a single sample in bits. The clip will be converted to this bit depth on import. */
		void setBitDepth(UINT32 bitDepth) { mBitDepth = bitDepth; }

		// Note: Add options to resample to a different frequency

	private:
		AudioFormat mFormat;
		AudioReadMode mReadMode;
		bool mIs3D;
		UINT32 mBitDepth;

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