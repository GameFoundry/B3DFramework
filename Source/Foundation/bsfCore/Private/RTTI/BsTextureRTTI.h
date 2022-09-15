//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Reflection/BsRTTIPlain.h"
#include "Image/BsTexture.h"
#include "Math/BsMath.h"
#include "CoreThread/BsCoreThread.h"
#include "RenderAPI/BsRenderAPI.h"
#include "Managers/BsTextureManager.h"
#include "Image/BsPixelData.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	BS_ALLOW_MEMCPY_SERIALIZATION(TextureSurface);

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

		INT32& GetUsage(Texture* obj) { return obj->mProperties.mDesc.usage; }
		void SetUsage(Texture* obj, INT32& val)
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

		SPtr<PixelData> GetPixelData(Texture* obj, UINT32 idx)
		{
			UINT32 face = (size_t)Math::Floor(idx / (float)(obj->mProperties.GetNumMipmaps() + 1));
			UINT32 mipmap = idx % (obj->mProperties.GetNumMipmaps() + 1);

			SPtr<PixelData> pixelData = obj->mProperties.AllocBuffer(face, mipmap);

			obj->ReadData(pixelData, face, mipmap);
			gCoreThread().SubmitAll(true);

			return pixelData;
		}

		void SetPixelData(Texture* obj, UINT32 idx, SPtr<PixelData> data)
		{
			mPixelData[idx] = data;
		}

		UINT32 GetPixelDataArraySize(Texture* obj)
		{
			return obj->mProperties.GetNumFaces() * (obj->mProperties.GetNumMipmaps() + 1);
		}

		void SetPixelDataArraySize(Texture* obj, UINT32 size)
		{
			mPixelData.resize(size);
		}

	public:
		TextureRTTI()
		{
			AddPlainField("mUsage", 11, &TextureRTTI::GetUsage, &TextureRTTI::SetUsage);

			AddReflectablePtrArrayField("mPixelData", 12, &TextureRTTI::GetPixelData, &TextureRTTI::GetPixelDataArraySize,
				&TextureRTTI::SetPixelData, &TextureRTTI::SetPixelDataArraySize, RTTIFieldInfo(RTTIFieldFlag::SkipInReferenceSearch));
		}

		void OnDeserializationEnded(IReflectable* obj, SerializationContext* context) override
		{
			Texture* texture = static_cast<Texture*>(obj);
			TextureProperties& texProps = texture->mProperties;

			// Update pixel format if needed as it's possible the original texture was saved using some other render API
			// that has an unsupported format.
			PixelFormat originalFormat = texProps.GetFormat();
			PixelFormat validFormat = TextureManager::Instance().GetNativeFormat(
				texProps.GetTextureType(), texProps.GetFormat(), texProps.GetUsage(), texProps.IsHardwareGammaEnabled());

			if (originalFormat != validFormat)
			{
				texProps.mDesc.format = validFormat;

				for (size_t i = 0; i < mPixelData.size(); i++)
				{
					SPtr<PixelData> origData = mPixelData[i];
					SPtr<PixelData> newData = PixelData::Create(origData->GetWidth(), origData->GetHeight(), origData->GetDepth(), validFormat);

					PixelUtil::BulkPixelConversion(*origData, *newData);
					mPixelData[i] = newData;
				}
			}

			// A bit clumsy initializing with already set values, but I feel its better than complicating things and storing the values
			// in mRTTIData.
			texture->Initialize();

			for(size_t i = 0; i < mPixelData.size(); i++)
			{
				UINT32 face = (size_t)Math::Floor(i / (float)(texProps.GetNumMipmaps() + 1));
				UINT32 mipmap = i % (texProps.GetNumMipmaps() + 1);

				texture->WriteData(mPixelData[i], face, mipmap, false);
			}
		}

		const String& GetRttiName() override
		{
			static String name = "Texture";
			return name;
		}

		UINT32 GetRttiId() override
		{
			return TID_Texture;
		}

		SPtr<IReflectable> NewRttiObject() 
		{
			return TextureManager::Instance().CreateEmptyInternal();
		}

	private:
		Vector<SPtr<PixelData>> mPixelData;
	};

	/** @} */
	/** @endcond */
}
