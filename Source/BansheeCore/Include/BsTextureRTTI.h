//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsRTTIType.h"
#include "BsTexture.h"
#include "BsMath.h"
#include "BsCoreThread.h"
#include "BsRenderAPI.h"
#include "BsTextureManager.h"
#include "BsPixelData.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class BS_CORE_EXPORT TextureRTTI : public RTTIType<Texture, Resource, TextureRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN(mSize, 0)
			BS_RTTI_MEMBER_PLAIN_NAMED(height, mProperties.mDesc.height, 2)
			BS_RTTI_MEMBER_PLAIN_NAMED(width, mProperties.mDesc.width, 3)
			BS_RTTI_MEMBER_PLAIN_NAMED(depth, mProperties.mDesc.depth, 4)
			BS_RTTI_MEMBER_PLAIN_NAMED(numMips, mProperties.mDesc.numMips, 5)
			BS_RTTI_MEMBER_PLAIN_NAMED(hwGamma, mProperties.mDesc.hwGamma, 6)
			BS_RTTI_MEMBER_PLAIN_NAMED(numSamples, mProperties.mDesc.numSamples, 7)
			BS_RTTI_MEMBER_PLAIN_NAMED(type, mProperties.mDesc.type, 9)
			BS_RTTI_MEMBER_PLAIN_NAMED(format, mProperties.mDesc.format, 10)
		BS_END_RTTI_MEMBERS

		INT32& getUsage(Texture* obj) { return obj->mProperties.mDesc.usage; }
		void setUsage(Texture* obj, INT32& val) 
		{ 
			// Render target and depth stencil texture formats are for in-memory use only
			// and don't make sense when serialized
			if ((val & (TU_DEPTHSTENCIL | TU_RENDERTARGET)) != 0)
			{
				obj->mProperties.mDesc.usage &= ~(TU_DEPTHSTENCIL | TU_RENDERTARGET);
				obj->mProperties.mDesc.usage |= TU_STATIC;
			}
			else
				obj->mProperties.mDesc.usage = val;
		}

#define BS_ADD_PLAINFIELD(name, id, parentType) \
	addPlainField(#name, id##, &##parentType##::get##name, &##parentType##::Set##name);

		SPtr<PixelData> getPixelData(Texture* obj, UINT32 idx)
		{
			UINT32 face = (size_t)Math::floor(idx / (float)(obj->mProperties.getNumMipmaps() + 1));
			UINT32 mipmap = idx % (obj->mProperties.getNumMipmaps() + 1);

			SPtr<PixelData> pixelData = obj->mProperties.allocBuffer(face, mipmap);

			int usage = obj->getProperties().getUsage();

			if (usage & TU_CPUREADABLE || BS_EDITOR_BUILD)
			{
				obj->readData(pixelData, face, mipmap);
				gCoreThread().submit(true);

				return pixelData;
			}

			if(usage & TU_CPUCACHED)
			{
				obj->readCachedData(*pixelData, face, mipmap);
				return pixelData;
			}

			LOGERR("Attempting to save a texture that isn't flagged with either TU_CPUCACHED OR TU_GPUREADABLE flags.");
			return pixelData;
		}

		void setPixelData(Texture* obj, UINT32 idx, SPtr<PixelData> data)
		{
			Vector<SPtr<PixelData>>* pixelData = any_cast<Vector<SPtr<PixelData>>*>(obj->mRTTIData);

			(*pixelData)[idx] = data;
		}

		UINT32 getPixelDataArraySize(Texture* obj)
		{
			return obj->mProperties.getNumFaces() * (obj->mProperties.getNumMipmaps() + 1);
		}

		void setPixelDataArraySize(Texture* obj, UINT32 size)
		{
			Vector<SPtr<PixelData>>* pixelData = any_cast<Vector<SPtr<PixelData>>*>(obj->mRTTIData);

			pixelData->resize(size);
		}

	public:
		TextureRTTI()
			:mInitMembers(this)
		{
			addPlainField("mUsage", 11, &TextureRTTI::getUsage, &TextureRTTI::setUsage);

			addReflectablePtrArrayField("mPixelData", 12, &TextureRTTI::getPixelData, &TextureRTTI::getPixelDataArraySize, 
				&TextureRTTI::setPixelData, &TextureRTTI::setPixelDataArraySize, RTTI_Flag_SkipInReferenceSearch);
		}

		void onDeserializationStarted(IReflectable* obj, const UnorderedMap<String, UINT64>& params) override
		{
			Texture* texture = static_cast<Texture*>(obj);

			texture->mRTTIData = bs_new<Vector<SPtr<PixelData>>>();
		}

		void onDeserializationEnded(IReflectable* obj, const UnorderedMap<String, UINT64>& params) override
		{
			Texture* texture = static_cast<Texture*>(obj);

			if(texture->mRTTIData.empty())
				return;

			TextureProperties& texProps = texture->mProperties;

			// Update pixel format if needed as it's possible the original texture was saved using some other render API
			// that has an unsupported format.
			PixelFormat originalFormat = texProps.getFormat();
			PixelFormat validFormat = TextureManager::instance().getNativeFormat(
				texProps.getTextureType(), texProps.getFormat(), texProps.getUsage(), texProps.isHardwareGammaEnabled());

			Vector<SPtr<PixelData>>* pixelData = any_cast<Vector<SPtr<PixelData>>*>(texture->mRTTIData);
			if (originalFormat != validFormat)
			{
				texProps.mDesc.format = validFormat;

				for (size_t i = 0; i < pixelData->size(); i++)
				{
					SPtr<PixelData> origData = pixelData->at(i);
					SPtr<PixelData> newData = PixelData::create(origData->getWidth(), origData->getHeight(), origData->getDepth(), validFormat);

					PixelUtil::bulkPixelConversion(*origData, *newData);
					(*pixelData)[i] = newData;
				}
			}

			// A bit clumsy initializing with already set values, but I feel its better than complicating things and storing the values
			// in mRTTIData.
			texture->initialize();

			for(size_t i = 0; i < pixelData->size(); i++)
			{
				UINT32 face = (size_t)Math::floor(i / (float)(texProps.getNumMipmaps() + 1));
				UINT32 mipmap = i % (texProps.getNumMipmaps() + 1);

				texture->writeData(pixelData->at(i), face, mipmap, false);
			}

			bs_delete(pixelData);
			texture->mRTTIData = nullptr;	
		}

		const String& getRTTIName() override
		{
			static String name = "Texture";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_Texture;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return TextureManager::instance()._createEmpty();
		}
	};

	/** @} */
	/** @endcond */
}