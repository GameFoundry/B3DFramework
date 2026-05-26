//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "RTTI/B3DStringRTTI.h"
#include "RTTI/B3DColorGradientRTTI.h"
#include "RTTI/B3DTextureRTTI.h"
#include "RTTI/B3DTextureSurfaceRTTI.h"
#include "RTTI/B3DAnimationCurveRTTI.h"
#include "Material/B3DMaterialParameters.h"
#include "GpuBackend/B3DSamplerState.h"
#include "FileSystem/B3DDataStream.h"
#include "Animation/B3DAnimationCurve.h"
#include "Image/B3DColorGradient.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class B3D_EXPORT MaterialParamTextureDataRTTI : public TRTTIType<MaterialParamTextureData, IReflectable, MaterialParamTextureDataRTTI>
	{
	public:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(Texture, 0)
			B3D_RTTI_MEMBER(IsLoadStore, 1)
			B3D_RTTI_MEMBER(Surface, 2)
			B3D_RTTI_MEMBER(SpriteImage, 3)
		B3D_RTTI_END_MEMBERS

		const String& GetRttiName() override
		{
			static String name = "TextureParamData";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_TextureParamData;
		}

		TShared<IReflectable> NewRttiObject()
		{
			return B3DMakeShared<MaterialParamTextureData>();
		}
	};

	class B3D_EXPORT MaterialParamBufferDataRTTI : public TRTTIType<MaterialParamBufferData, IReflectable, MaterialParamBufferDataRTTI>
	{
	public:
		const String& GetRttiName() override
		{
			static String name = "BufferParamData";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_BufferParamData;
		}

		TShared<IReflectable> NewRttiObject()
		{
			return B3DMakeShared<MaterialParamBufferData>();
		}
	};

	class B3D_EXPORT MaterialParamSamplerStateDataRTTI : public TRTTIType<MaterialParamSamplerStateData, IReflectable, MaterialParamSamplerStateDataRTTI>
	{
	public:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(Value, 0)
		B3D_RTTI_END_MEMBERS

		const String& GetRttiName() override
		{
			static String name = "SamplerStateParamData";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_SamplerStateParamData;
		}

		TShared<IReflectable> NewRttiObject()
		{
			return B3DMakeShared<MaterialParamSamplerStateData>();
		}
	};

	class B3D_EXPORT MaterialParametersRTTI : public TRTTIType<MaterialParameters, IReflectable, MaterialParametersRTTI>
	{
	public:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_CONTAINER(mParamLookup, 0)
			B3D_RTTI_MEMBER_CONTAINER(mParams, 1)
			B3D_RTTI_MEMBER_CONTAINER(mDataParameterMetaData, 3)
			B3D_RTTI_MEMBER_CONTAINER(mStructParameterMetaData, 4)
			B3D_RTTI_MEMBER_CONTAINER(mTextureParameters, 5)
			B3D_RTTI_MEMBER_CONTAINER(mBufferParameters, 6)
			B3D_RTTI_MEMBER_CONTAINER(mSamplerParameters, 7)
		B3D_RTTI_END_MEMBERS

		TShared<DataStream> GetDataBuffer(MaterialParameters* obj, u32& size)
		{
			size = obj->mDataSize;

			return B3DMakeShared<MemoryDataStream>(obj->mDataParamsBuffer, obj->mDataSize);
		}

		void SetDataBuffer(MaterialParameters* obj, const TShared<DataStream>& value, u32 size)
		{
			obj->mDataParamsBuffer = obj->mAlloc.Alloc(size);

			TAsyncOp<TShared<MemoryDataStream>> readOp = value->ReadAsync((u64)value->Tell(), size, DataRange(obj->mDataParamsBuffer, size));
			readOp.BlockUntilComplete();

			obj->mDataSize = size;
		}

		MaterialParametersRTTI()
		{
			AddDataBlockField("dataBuffer", 2, &MaterialParametersRTTI::GetDataBuffer, &MaterialParametersRTTI::SetDataBuffer);
		}

		const String& GetRttiName() override
		{
			static String name = "MaterialParameters";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_MaterialParameters;
		}

		TShared<IReflectable> NewRttiObject()
		{
			return B3DMakeShared<MaterialParameters>();
		}
	};

	template <>
	struct RTTIPlainType<MaterialParametersBase::ParamData>
	{
		enum
		{
			id = TID_MaterialParamData
		};

		enum
		{
			hasDynamicSize = 0
		};

		static BitLength ToMemory(const MaterialParametersBase::ParamData& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			B3DRTTIWrite(data.Type, stream);
			B3DRTTIWrite(data.DataType, stream);
			B3DRTTIWrite(data.Index, stream);
			B3DRTTIWrite(data.ArraySize, stream);
			B3DRTTIWrite((u64)0, stream);

			return sizeof(MaterialParametersBase::ParamData);
		}

		static BitLength FromMemory(MaterialParametersBase::ParamData& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			B3DRTTIRead(data.Type, stream);
			B3DRTTIRead(data.DataType, stream);
			B3DRTTIRead(data.Index, stream);
			B3DRTTIRead(data.ArraySize, stream);
			B3DRTTIRead(data.Version, stream);

			// Not a field we should serialize, but we do because this struct is serialized as a whole
			data.Version = 1;
			return sizeof(MaterialParametersBase::ParamData);
		}

		static BitLength GetSize(const MaterialParametersBase::ParamData& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			return sizeof(MaterialParametersBase::ParamData);
		}
	};

	template <>
	struct RTTIPlainType<MaterialParametersBase::DataParamInfo>
	{
		enum
		{
			id = TID_DataParamInfo
		};

		enum
		{
			hasDynamicSize = 1
		};

		static BitLength ToMemory(const MaterialParametersBase::DataParamInfo& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			static constexpr uint32_t kVersion = 1;

			return B3DRTTIWriteWithSizeHeader(stream, data, compress, [&data, &stream]()
											   {
				BitLength size = 0;
				size += B3DRTTIWrite(kVersion, stream);
				size += B3DRTTIWrite(data.Offset, stream);

				uint32_t curveType = 0; // No curve

				if (data.FloatCurve)
					curveType = 1;
				else if (data.ColorGradient)
					curveType = 2;
				else if (data.SpriteTextureIdx != (uint32_t)-1)
					curveType = 3;

				size += B3DRTTIWrite(curveType, stream);
				if (data.FloatCurve)
					size += B3DRTTIWrite(*data.FloatCurve, stream);
				else if (data.ColorGradient)
					size += B3DRTTIWrite(*data.ColorGradient, stream);
				else if (data.SpriteTextureIdx != (uint32_t)-1)
					size += B3DRTTIWrite(data.SpriteTextureIdx, stream);

				return size; });
		}

		static BitLength FromMemory(MaterialParametersBase::DataParamInfo& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			B3DRTTIReadSizeHeader(stream, compress, size);

			uint32_t version = 0;
			B3DRTTIRead(version, stream);

			switch(version)
			{
			case 0:
			case 1:
				{
					B3DRTTIRead(data.Offset, stream);

					uint32_t curveType = 0;
					B3DRTTIRead(curveType, stream);

					data.FloatCurve = nullptr;
					data.ColorGradient = nullptr;
					data.SpriteTextureIdx = (uint32_t)-1;

					switch(curveType)
					{
					case 1:
						data.FloatCurve = B3DPoolNew<TAnimationCurve<float>>();
						B3DRTTIRead(*data.FloatCurve, stream);
						break;
					case 2:
						if(version == 0)
						{
							// Version 0 stores non-HDR gradients
							ColorGradient temp;
							B3DRTTIRead(temp, stream);

							data.ColorGradient = B3DPoolNew<ColorGradientHDR>(temp.GetKeys());
						}
						else
						{
							data.ColorGradient = B3DPoolNew<ColorGradientHDR>();
							B3DRTTIRead(*data.ColorGradient, stream);
						}

						break;
					case 3:
						B3DRTTIRead(data.SpriteTextureIdx, stream);
						break;
					default:
						break;
					}
				}
				break;
			default:
				B3D_LOG(Error, LogRTTI, "Unknown version. Unable to deserialize.");
				break;
			}

			return size;
		}

		static BitLength GetSize(const MaterialParametersBase::DataParamInfo& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size = B3DRTTISize(data.Offset) + sizeof(uint32_t) * 2;

			if(data.FloatCurve)
				size += B3DRTTISize(*data.FloatCurve);
			else if(data.ColorGradient)
				size += B3DRTTISize(*data.ColorGradient);
			else if(data.SpriteTextureIdx != (uint32_t)-1)
				size += B3DRTTISize(data.SpriteTextureIdx);

			B3DRTTIAddHeaderSize(size, compress);
			return size;
		}
	};

	
	template <>
	struct RTTIPlainType<MaterialParametersBase::StructParameterMetaData> : RTTIPlainTypeHelper<MaterialParametersBase::StructParameterMetaData, TID_StructParameterMetaData, 0, false>
	{
		template <class Processor>
		static void RTTIEnumerateFields(MaterialParametersBase::StructParameterMetaData& object, Processor& processor, u8 version)
		{
			processor(object.Offset);
			processor(object.DataSize);
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
