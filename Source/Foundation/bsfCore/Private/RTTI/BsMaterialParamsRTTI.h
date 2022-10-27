//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "RTTI/BsStringRTTI.h"
#include "RTTI/BsColorGradientRTTI.h"
#include "Private/RTTI/BsTextureRTTI.h"
#include "Private/RTTI/BsAnimationCurveRTTI.h"
#include "Material/BsMaterialParams.h"
#include "RenderAPI/BsSamplerState.h"
#include "FileSystem/BsDataStream.h"
#include "Animation/BsAnimationCurve.h"
#include "Image/BsColorGradient.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class BS_CORE_EXPORT MaterialParamTextureDataRTTI : public RTTIType<MaterialParamTextureData, IReflectable, MaterialParamTextureDataRTTI>
	{
	public:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_REFL(Texture, 0)
			BS_RTTI_MEMBER_PLAIN(IsLoadStore, 1)
			BS_RTTI_MEMBER_PLAIN(Surface, 2)
			BS_RTTI_MEMBER_REFL(SpriteTexture, 3)
		BS_END_RTTI_MEMBERS

		const String& GetRttiName() override
		{
			static String name = "TextureParamData";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_TextureParamData;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return bs_shared_ptr_new<MaterialParamTextureData>();
		}
	};

	class BS_CORE_EXPORT MaterialParamStructDataRTTI : public RTTIType<MaterialParamStructData, IReflectable, MaterialParamStructDataRTTI>
	{
	public:
		SPtr<DataStream> GetDataBuffer(MaterialParamStructData* obj, u32& size)
		{
			size = obj->DataSize;

			return bs_shared_ptr_new<MemoryDataStream>(obj->Data, obj->DataSize);
		}

		void SetDataBuffer(MaterialParamStructData* obj, const SPtr<DataStream>& value, u32 size)
		{
			obj->Data = (u8*)bs_alloc(size);
			value->Read(obj->Data, size);

			obj->DataSize = size;
		}

		MaterialParamStructDataRTTI()
		{
			AddDataBlockField("dataBuffer", 0, &MaterialParamStructDataRTTI::GetDataBuffer, &MaterialParamStructDataRTTI::SetDataBuffer);
		}

		const String& GetRttiName() override
		{
			static String name = "StructParamData";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_StructParamData;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return bs_shared_ptr_new<MaterialParamStructData>();
		}
	};

	class BS_CORE_EXPORT MaterialParamsRTTI : public RTTIType<MaterialParams, IReflectable, MaterialParamsRTTI>
	{
	public:
		struct MaterialParam
		{
			String Name;
			u32 Index;
			MaterialParams::ParamData Data;
		};

		MaterialParam& GetParamData(MaterialParams* obj, u32 idx)
		{
			return mMatParams[idx];
		}

		void SetParamData(MaterialParams* obj, u32 idx, MaterialParam& param)
		{
			u32 paramIdx = param.Index;

			// Older saved files might not have indices preserved
			if(paramIdx == (u32)-1)
				paramIdx = mNextParamIdx++;

			if(obj->mParams.size() <= (size_t)paramIdx)
				obj->mParams.resize((size_t)paramIdx + 1);

			obj->mParams[paramIdx] = param.Data;
			obj->mParamLookup[param.Name] = paramIdx;
		}

		u32 GetParamDataArraySize(MaterialParams* obj)
		{
			return (u32)mMatParams.size();
		}

		void SetParamDataArraySize(MaterialParams* obj, u32 size)
		{
			obj->mParams.resize(size);
		}

		SPtr<DataStream> GetDataBuffer(MaterialParams* obj, u32& size)
		{
			size = obj->mDataSize;

			return bs_shared_ptr_new<MemoryDataStream>(obj->mDataParamsBuffer, obj->mDataSize);
		}

		void SetDataBuffer(MaterialParams* obj, const SPtr<DataStream>& value, u32 size)
		{
			obj->mDataParamsBuffer = obj->mAlloc.Alloc(size);
			value->Read(obj->mDataParamsBuffer, size);

			obj->mDataSize = size;
		}

		MaterialParamStructData& GetStructParam(MaterialParams* obj, u32 idx) { return obj->mStructParams[idx]; }

		void SetStructParam(MaterialParams* obj, u32 idx, MaterialParamStructData& param)
		{
			MaterialParamStructData& newStructParam = obj->mStructParams[idx];
			newStructParam.Data = (u8*)obj->mAlloc.Alloc(param.DataSize);
			memcpy(newStructParam.Data, param.Data, param.DataSize);
			newStructParam.DataSize = param.DataSize;

			bs_free(param.Data);
			param.Data = nullptr;
		}

		u32 GetStructArraySize(MaterialParams* obj) { return (u32)obj->mNumStructParams; }

		void SetStructArraySize(MaterialParams* obj, u32 size)
		{
			obj->mNumStructParams = size;
			obj->mStructParams = obj->mAlloc.Construct<MaterialParamStructData>(size);
		}

		MaterialParamTextureData& GetTextureParam(MaterialParams* obj, u32 idx) { return obj->mTextureParams[idx]; }

		void SetTextureParam(MaterialParams* obj, u32 idx, MaterialParamTextureData& param) { obj->mTextureParams[idx] = param; }

		u32 GetTextureArraySize(MaterialParams* obj) { return (u32)obj->mNumTextureParams; }

		void SetTextureArraySize(MaterialParams* obj, u32 size)
		{
			obj->mNumTextureParams = size;
			obj->mTextureParams = obj->mAlloc.Construct<MaterialParamTextureData>(size);
		}

		SPtr<SamplerState> GetSamplerStateParam(MaterialParams* obj, u32 idx) { return obj->mSamplerStateParams[idx].Value; }

		void SetSamplerStateParam(MaterialParams* obj, u32 idx, SPtr<SamplerState> param) { obj->mSamplerStateParams[idx].Value = param; }

		u32 GetSamplerStateArraySize(MaterialParams* obj) { return (u32)obj->mNumSamplerParams; }

		void SetSamplerStateArraySize(MaterialParams* obj, u32 size)
		{
			obj->mNumSamplerParams = size;
			obj->mSamplerStateParams = obj->mAlloc.Construct<MaterialParamSamplerStateData>(size);
		}

		u32& GetNumBufferParams(MaterialParams* obj)
		{
			return obj->mNumBufferParams;
		}

		void SetNumBufferParams(MaterialParams* obj, u32& value)
		{
			obj->mNumBufferParams = value;
			obj->mBufferParams = obj->mAlloc.Construct<MaterialParamBufferData>(value);
		}

		MaterialParamsBase::DataParamInfo& GetDataParam(MaterialParams* obj, u32 idx) { return obj->mDataParams[idx]; }

		void SetDataParam(MaterialParams* obj, u32 idx, MaterialParamsBase::DataParamInfo& param) { obj->mDataParams[idx] = param; }

		u32 GetDataParamArraySize(MaterialParams* obj) { return (u32)obj->mNumDataParams; }

		void SetDataParamArraySize(MaterialParams* obj, u32 size)
		{
			obj->mNumDataParams = size;
			obj->mDataParams = obj->mAlloc.Construct<MaterialParamsBase::DataParamInfo>(size);
		}

		MaterialParamsRTTI()
		{
			AddPlainArrayField("paramData", 0, &MaterialParamsRTTI::GetParamData, &MaterialParamsRTTI::GetParamDataArraySize, &MaterialParamsRTTI::SetParamData, &MaterialParamsRTTI::SetParamDataArraySize);
			AddDataBlockField("dataBuffer", 1, &MaterialParamsRTTI::GetDataBuffer, &MaterialParamsRTTI::SetDataBuffer);
			AddReflectableArrayField("structParams", 2, &MaterialParamsRTTI::GetStructParam, &MaterialParamsRTTI::GetStructArraySize, &MaterialParamsRTTI::SetStructParam, &MaterialParamsRTTI::SetStructArraySize);
			AddReflectableArrayField("textureParams", 3, &MaterialParamsRTTI::GetTextureParam, &MaterialParamsRTTI::GetTextureArraySize, &MaterialParamsRTTI::SetTextureParam, &MaterialParamsRTTI::SetTextureArraySize);
			AddReflectablePtrArrayField("samplerStateParams", 4, &MaterialParamsRTTI::GetSamplerStateParam, &MaterialParamsRTTI::GetSamplerStateArraySize, &MaterialParamsRTTI::SetSamplerStateParam, &MaterialParamsRTTI::SetSamplerStateArraySize);
			AddPlainField("numBufferParams", 5, &MaterialParamsRTTI::GetNumBufferParams, &MaterialParamsRTTI::SetNumBufferParams);
			AddPlainArrayField("dataParams", 6, &MaterialParamsRTTI::GetDataParam, &MaterialParamsRTTI::GetDataParamArraySize, &MaterialParamsRTTI::SetDataParam, &MaterialParamsRTTI::SetDataParamArraySize);
		}

		void OnSerializationStarted(IReflectable* obj, SerializationContext* context) override
		{
			MaterialParams* paramsObj = static_cast<MaterialParams*>(obj);

			for(auto& entry : paramsObj->mParamLookup)
			{
				u32 paramIdx = entry.second;
				mMatParams.push_back({ entry.first, paramIdx, paramsObj->mParams[paramIdx] });
			}
		}

		void OnDeserializationEnded(IReflectable* obj, SerializationContext* context) override
		{
			MaterialParams* paramsObj = static_cast<MaterialParams*>(obj);

			// This field was added in later versions of the file format, so generate valid data for it if loading from
			// an older serialized version
			if(!paramsObj->mDataParams)
			{
				paramsObj->mNumDataParams = 0;
				for(auto& entry : paramsObj->mParams)
				{
					if(entry.Type != MaterialParams::ParamType::Data)
						continue;

					paramsObj->mNumDataParams++;
				}

				paramsObj->mDataParams = (MaterialParams::DataParamInfo*)paramsObj->mAlloc.Alloc(
					paramsObj->mNumDataParams * sizeof(MaterialParams::DataParamInfo));
				memset(paramsObj->mDataParams, 0, paramsObj->mNumDataParams * sizeof(MaterialParams::DataParamInfo));

				u32 paramIdx = 0;
				u32 dataBufferIdx = 0;
				for(auto& entry : paramsObj->mParams)
				{
					if(entry.Type != MaterialParams::ParamType::Data)
						continue;

					const GpuParamDataTypeInfo& typeInfo = GpuParams::kParamSizes.Lookup[(int)entry.DataType];
					const u32 paramSize = typeInfo.NumColumns * typeInfo.NumRows * typeInfo.BaseTypeSize;
					for(u32 i = 0; i < entry.ArraySize; i++)
					{
						paramsObj->mDataParams[paramIdx + i].Offset = dataBufferIdx;

						dataBufferIdx += paramSize;
					}

					entry.Index = paramIdx;
					paramIdx += entry.ArraySize;
				}
			}
		}

		const String& GetRttiName() override
		{
			static String name = "MaterialParams";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_MaterialParams;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return bs_shared_ptr_new<MaterialParams>();
		}

	private:
		Vector<MaterialParam> mMatParams;
		u32 mNextParamIdx = 0;
	};

	template <>
	struct RTTIPlainType<MaterialParamsBase::ParamData>
	{
		enum
		{
			id = TID_MaterialParamData
		};

		enum
		{
			hasDynamicSize = 0
		};

		static BitLength ToMemory(const MaterialParamsBase::ParamData& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			rtti_write(data.Type, stream);
			rtti_write(data.DataType, stream);
			rtti_write(data.Index, stream);
			rtti_write(data.ArraySize, stream);
			rtti_write((u64)0, stream);

			return sizeof(MaterialParamsBase::ParamData);
		}

		static BitLength FromMemory(MaterialParamsBase::ParamData& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			rtti_read(data.Type, stream);
			rtti_read(data.DataType, stream);
			rtti_read(data.Index, stream);
			rtti_read(data.ArraySize, stream);
			rtti_read(data.Version, stream);

			// Not a field we should serialize, but we do because this struct is serialized as a whole
			data.Version = 1;
			return sizeof(MaterialParamsBase::ParamData);
		}

		static BitLength GetSize(const MaterialParamsBase::ParamData& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			return sizeof(MaterialParamsBase::ParamData);
		}
	};

	template <>
	struct RTTIPlainType<MaterialParamsBase::DataParamInfo>
	{
		enum
		{
			id = TID_DataParamInfo
		};

		enum
		{
			hasDynamicSize = 1
		};

		static BitLength ToMemory(const MaterialParamsBase::DataParamInfo& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			static constexpr uint32_t kVersion = 1;

			return rtti_write_with_size_header(stream, data, compress, [&data, &stream]()
											   {
				BitLength size = 0;
				size += rtti_write(kVersion, stream);
				size += rtti_write(data.Offset, stream);

				uint32_t curveType = 0; // No curve

				if (data.FloatCurve)
					curveType = 1;
				else if (data.ColorGradient)
					curveType = 2;
				else if (data.SpriteTextureIdx != (uint32_t)-1)
					curveType = 3;

				size += rtti_write(curveType, stream);
				if (data.FloatCurve)
					size += rtti_write(*data.FloatCurve, stream);
				else if (data.ColorGradient)
					size += rtti_write(*data.ColorGradient, stream);
				else if (data.SpriteTextureIdx != (uint32_t)-1)
					size += rtti_write(data.SpriteTextureIdx, stream);

				return size; });
		}

		static BitLength FromMemory(MaterialParamsBase::DataParamInfo& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			rtti_read_size_header(stream, compress, size);

			uint32_t version = 0;
			rtti_read(version, stream);

			switch(version)
			{
			case 0:
			case 1:
				{
					rtti_read(data.Offset, stream);

					uint32_t curveType = 0;
					rtti_read(curveType, stream);

					data.FloatCurve = nullptr;
					data.ColorGradient = nullptr;
					data.SpriteTextureIdx = (uint32_t)-1;

					switch(curveType)
					{
					case 1:
						data.FloatCurve = bs_pool_new<TAnimationCurve<float>>();
						rtti_read(*data.FloatCurve, stream);
						break;
					case 2:
						if(version == 0)
						{
							// Version 0 stores non-HDR gradients
							ColorGradient temp;
							rtti_read(temp, stream);

							data.ColorGradient = bs_pool_new<ColorGradientHDR>(temp.GetKeys());
						}
						else
						{
							data.ColorGradient = bs_pool_new<ColorGradientHDR>();
							rtti_read(*data.ColorGradient, stream);
						}

						break;
					case 3:
						rtti_read(data.SpriteTextureIdx, stream);
						break;
					default:
						break;
					}
				}
				break;
			default:
				BS_LOG(Error, RTTI, "Unknown version. Unable to deserialize.");
				break;
			}

			return size;
		}

		static BitLength GetSize(const MaterialParamsBase::DataParamInfo& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size = rtti_size(data.Offset) + sizeof(uint32_t) * 2;

			if(data.FloatCurve)
				size += rtti_size(*data.FloatCurve);
			else if(data.ColorGradient)
				size += rtti_size(*data.ColorGradient);
			else if(data.SpriteTextureIdx != (uint32_t)-1)
				size += rtti_size(data.SpriteTextureIdx);

			rtti_add_header_size(size, compress);
			return size;
		}
	};

	template <>
	struct RTTIPlainType<MaterialParamsRTTI::MaterialParam>
	{
		enum
		{
			id = TID_MaterialRTTIParam
		};

		enum
		{
			hasDynamicSize = 1
		};

		static BitLength ToMemory(const MaterialParamsRTTI::MaterialParam& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			static constexpr u32 kVersion = 1;

			return rtti_write_with_size_header(stream, data, compress, [&data, &stream]()
											   {
				BitLength size = 0;
				size += rtti_write(data.Name, stream);
				size += rtti_write(data.Data, stream);

				// Version 1 data
				size += rtti_write(kVersion, stream);
				size += rtti_write(data.Index, stream);

				return size; });
		}

		static BitLength FromMemory(MaterialParamsRTTI::MaterialParam& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;

			BitLength sizeRead = rtti_read_size_header(stream, compress, size);
			sizeRead += rtti_read(data.Name, stream);
			sizeRead += rtti_read(data.Data, stream);

			// More fields means a newer version of the data format
			if(size > sizeRead)
			{
				uint32_t version = 0;
				rtti_read(version, stream);

				switch(version)
				{
				case 1:
					rtti_read(data.Index, stream);
					break;
				default:
					BS_LOG(Error, RTTI, "Unknown version. Unable to deserialize.");
					break;
				}
			}
			else
				data.Index = (uint32_t)-1; // Lets the other code know that index needs to be generated

			return size;
		}

		static BitLength GetSize(const MaterialParamsRTTI::MaterialParam& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size = rtti_size(data.Name) + rtti_size(data.Data) + rtti_size(data.Index) +
				sizeof(uint32_t) * 1;

			rtti_add_header_size(size, compress);
			return size;
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
