//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "Reflection/B3DRTTIPlain.h"
#include "RTTI/B3DFlagsRTTI.h"
#include "Image/B3DTexture.h"
#include "Math/B3DMath.h"
#include "CoreObject/B3DRenderThread.h"
#include "Managers/B3DTextureManager.h"
#include "Image/B3DPixelData.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class B3D_EXPORT TextureRTTI : public TRTTIType<Texture, Resource, TextureRTTI>
	{
	private:
		Vector<TShared<PixelData>> mPixelData;

		B3D_RTTI_BEGIN_MEMBERS
			//B3D_RTTI_MEMBER(mSize, 0)
			B3D_RTTI_MEMBER_NAMED(height, mProperties.Height, 2)
			B3D_RTTI_MEMBER_NAMED(width, mProperties.Width, 3)
			B3D_RTTI_MEMBER_NAMED(depth, mProperties.Depth, 4)
			B3D_RTTI_MEMBER_NAMED(numMips, mProperties.MipMapCount, 5)
			B3D_RTTI_MEMBER_NAMED(hwGamma, mProperties.UseHardwareSRGB, 6)
			B3D_RTTI_MEMBER_NAMED(numSamples, mProperties.SampleCount, 7)
			B3D_RTTI_MEMBER_NAMED(type, mProperties.Type, 9)
			B3D_RTTI_MEMBER_NAMED(format, mProperties.Format, 10)
			B3D_RTTI_GENERATED_MEMBER_CONTAINER(mPixelData, 12)
		B3D_RTTI_END_MEMBERS


		UPtrRTTIIterator<TextureUsageFlags, false> GetUsageIterator(Texture& object, FrameAllocator& frameAllocator)
		{
			return CreateRTTIIterator<TextureUsageFlags, false>(frameAllocator, object.mProperties.Usage);
		}

		const TextureUsageFlags& GetUsage(Texture& object, FrameAllocator& frameAllocator, TRTTIIterator<TextureUsageFlags, false>& iterator)
		{
			return *iterator;
		}

		void SetUsage(Texture& object, FrameAllocator& frameAllocator, TRTTIIterator<TextureUsageFlags, false>& iterator, const TextureUsageFlags& value)
		{
			// Render target and depth stencil texture formats are for in-memory use only
			// and don't make sense when serialized
			TextureUsageFlags finalValue(value);
			if(finalValue.IsSetAny(TextureUsageFlag::DepthStencil | TextureUsageFlag::RenderTarget))
			{
				finalValue.Unset(TextureUsageFlag::DepthStencil);
				finalValue.Unset(TextureUsageFlag::RenderTarget);
				finalValue.Set(TextureUsageFlag::StoreOnGPU);
			}

			iterator = finalValue;
		}

	public:
		TextureRTTI()
		{
			AddField("mUsage", 11, &TextureRTTI::GetUsageIterator, &TextureRTTI::GetUsage, &TextureRTTI::SetUsage);
		}

		void OnOperationStarted(Texture& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(operationType.IsSet(RTTIOperationType::ReadBit) && operationType != RTTIOperationType::GatherReferences)
			{
				const u32 faceCount = object.GetProperties().GetFaceCount();
				const u32 mipLevelCount = object.GetProperties().MipMapCount + 1;

				const u32 surfaceCount = faceCount * mipLevelCount; 
				mPixelData.reserve(surfaceCount);
				for(u32 surfaceIndex = 0; surfaceIndex < surfaceCount; ++surfaceIndex)
				{
					u32 face = surfaceIndex / mipLevelCount;
					u32 mipmap = surfaceIndex % mipLevelCount;

					TShared<PixelData> pixelData = object.GetProperties().AllocBuffer(face, mipmap);

					object.ReadData(pixelData, face, mipmap);
					GetRenderThread().PostCommand([] {}, "TextureRTTI::GetPixelData", true, object.GetName());

					mPixelData.push_back(pixelData);
				}
			}
		}

		void OnOperationEnded(Texture& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(operationType.IsSet(RTTIOperationType::WriteBit) && !operationType.IsSet(RTTIOperationType::PreExistingObjectBit))
			{
				TextureProperties& texProps = object.mProperties;

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
						TShared<PixelData> origData = mPixelData[i];
						TShared<PixelData> newData = PixelData::Create(origData->GetWidth(), origData->GetHeight(), origData->GetDepth(), validFormat);

						PixelUtility::BulkPixelConversion(*origData, *newData);
						mPixelData[i] = newData;
					}
				}

				// Inherit name from resource if not set
				if(texProps.Name.empty())
					texProps.Name = object.GetName();

				// A bit clumsy initializing with already set values, but I feel its better than complicating things and storing the values
				// in mRTTIData.
				object.Initialize();

				for(size_t i = 0; i < mPixelData.size(); i++)
				{
					u32 face = (size_t)Math::Floor(i / (float)(texProps.MipMapCount + 1));
					u32 mipmap = i % (texProps.MipMapCount + 1);

					object.WriteData(mPixelData[i], face, mipmap, false);
				}
			}
		}

		const String& GetRttiName() override
		{
			static String name = "Texture";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_Texture;
		}

		TShared<IReflectable> NewRttiObject()
		{
			return Texture::CreateEmpty();
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
