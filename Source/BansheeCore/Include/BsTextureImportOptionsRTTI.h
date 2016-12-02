//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsRTTIType.h"
#include "BsTextureImportOptions.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class BS_CORE_EXPORT TextureImportOptionsRTTI : public RTTIType<TextureImportOptions, ImportOptions, TextureImportOptionsRTTI>
	{
	private:
		PixelFormat& getPixelFormat(TextureImportOptions* obj) { return obj->mFormat; }
		void setPixelFormat(TextureImportOptions* obj, PixelFormat& value) { obj->mFormat = value; }

		bool& getGenerateMips(TextureImportOptions* obj) { return obj->mGenerateMips; }
		void setGenerateMips(TextureImportOptions* obj, bool& value) { obj->mGenerateMips = value; }

		UINT32& getMaxMip(TextureImportOptions* obj) { return obj->mMaxMip; }
		void setMaxMip(TextureImportOptions* obj, UINT32& value) { obj->mMaxMip = value; }

		bool& getCPUCached(TextureImportOptions* obj) { return obj->mCPUCached; }
		void setCPUCached(TextureImportOptions* obj, bool& value) { obj->mCPUCached = value; }

		bool& getSRGB(TextureImportOptions* obj) { return obj->mSRGB; }
		void setSRGB(TextureImportOptions* obj, bool& value) { obj->mSRGB = value; }

	public:
		TextureImportOptionsRTTI()
		{
			addPlainField("mPixelFormat", 0, &TextureImportOptionsRTTI::getPixelFormat, &TextureImportOptionsRTTI::setPixelFormat);
			addPlainField("mGenerateMips", 1, &TextureImportOptionsRTTI::getGenerateMips, &TextureImportOptionsRTTI::setGenerateMips);
			addPlainField("mMaxMip", 2, &TextureImportOptionsRTTI::getMaxMip, &TextureImportOptionsRTTI::setMaxMip);
			addPlainField("mCPUCached", 3, &TextureImportOptionsRTTI::getCPUCached, &TextureImportOptionsRTTI::setCPUCached);
			addPlainField("mSRGB", 4, &TextureImportOptionsRTTI::getSRGB, &TextureImportOptionsRTTI::setSRGB);
		}

		const String& getRTTIName() override
		{
			static String name = "TextureImportOptions";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_TextureImportOptions;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<TextureImportOptions>();
		}
	};

	/** @} */
	/** @endcond */
}