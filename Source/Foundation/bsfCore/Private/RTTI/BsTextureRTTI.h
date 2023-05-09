//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Reflection/BsRTTIPlain.h"
#include "Image/BsTexture.h"
#include "Math/BsMath.h"
#include "CoreThread/BsCoreThread.h"
#include "Managers/BsTextureManager.h"
#include "Image/BsPixelData.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class B3D_CORE_EXPORT TextureRTTI : public RTTIType<Texture, Resource, TextureRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_PLAIN(mSize, 0)
			B3D_RTTI_MEMBER_PLAIN_NAMED(height, mProperties.Height, 2)
			B3D_RTTI_MEMBER_PLAIN_NAMED(width, mProperties.Width, 3)
			B3D_RTTI_MEMBER_PLAIN_NAMED(depth, mProperties.Depth, 4)
			B3D_RTTI_MEMBER_PLAIN_NAMED(numMips, mProperties.MipMapCount, 5)
			B3D_RTTI_MEMBER_PLAIN_NAMED(hwGamma, mProperties.UseHardwareSRGB, 6)
			B3D_RTTI_MEMBER_PLAIN_NAMED(numSamples, mProperties.SampleCount, 7)
			B3D_RTTI_MEMBER_PLAIN_NAMED(type, mProperties.Type, 9)
			B3D_RTTI_MEMBER_PLAIN_NAMED(format, mProperties.Format, 10)
		B3D_RTTI_END_MEMBERS

		i32& GetUsage(Texture* obj) { return obj->mProperties.Usage; }

		void SetUsage(Texture* obj, i32& val)
		{
			// Render target and depth stencil texture formats are for in-memory use only
			// and don't make sense when serialized
			if((val & (TU_DEPTHSTENCIL | TU_RENDERTARGET)) != 0)
			{
				obj->mProperties.Usage &= ~(TU_DEPTHSTENCIL | TU_RENDERTARGET);
				obj->mProperties.Usage |= TU_STATIC;
			}
			else
				obj->mProperties.Usage = val;
		}

		SPtr<PixelData> GetPixelData(Texture* obj, u32 idx)
		{
			u32 face = (size_t)Math::Floor(idx / (float)(obj->mProperties.MipMapCount + 1));
			u32 mipmap = idx % (obj->mProperties.MipMapCount + 1);

			SPtr<PixelData> pixelData = obj->mProperties.AllocBuffer(face, mipmap);

			obj->ReadData(pixelData, face, mipmap);
			GetCoreThread().SubmitAll(true);

			return pixelData;
		}

		void SetPixelData(Texture* obj, u32 idx, SPtr<PixelData> data)
		{
			mPixelData[idx] = data;
		}

		u32 GetPixelDataArraySize(Texture* obj)
		{
			return obj->mProperties.GetFaceCount() * (obj->mProperties.MipMapCount + 1);
		}

		void SetPixelDataArraySize(Texture* obj, u32 size)
		{
			mPixelData.resize(size);
		}

	public:
		TextureRTTI()
		{
			AddPlainField("mUsage", 11, &TextureRTTI::GetUsage, &TextureRTTI::SetUsage);

			AddReflectablePtrArrayField("mPixelData", 12, &TextureRTTI::GetPixelData, &TextureRTTI::GetPixelDataArraySize, &TextureRTTI::SetPixelData, &TextureRTTI::SetPixelDataArraySize, RTTIFieldInfo(RTTIFieldFlag::SkipInReferenceSearch));
		}

		void OnDeserializationEnded(IReflectable* obj, SerializationContext* context) override
		{
			Texture* texture = static_cast<Texture*>(obj);
			TextureProperties& texProps = texture->mProperties;

			// Update pixel format if needed as it's possible the original texture was saved using some other render API
			// that has an unsupported format.
			PixelFormat originalFormat = texProps.Format;
			PixelFormat validFormat = TextureManager::Instance().GetNativeFormat(
				texProps.Type, texProps.Format, texProps.Usage, texProps.UseHardwareSRGB);

			if(originalFormat != validFormat)
			{
				texProps.Format = validFormat;

				for(size_t i = 0; i < mPixelData.size(); i++)
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
				u32 face = (size_t)Math::Floor(i / (float)(texProps.MipMapCount + 1));
				u32 mipmap = i % (texProps.MipMapCount + 1);

				texture->WriteData(mPixelData[i], face, mipmap, false);
			}
		}

		const String& GetRttiName() override
		{
			static String name = "Texture";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_Texture;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return Texture::CreateEmpty();
		}

	private:
		Vector<SPtr<PixelData>> mPixelData;
	};

	/** @} */
	/** @endcond */
} // namespace bs
