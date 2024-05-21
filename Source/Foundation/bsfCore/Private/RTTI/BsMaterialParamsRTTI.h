//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "RTTI/BsStringRTTI.h"
#include "RTTI/BsColorGradientRTTI.h"
#include "Private/RTTI/BsTextureRTTI.h"
#include "Private/RTTI/BsTextureSurfaceRTTI.h"
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

	class B3D_CORE_EXPORT MaterialParamTextureDataRTTI : public RTTIType<MaterialParamTextureData, IReflectable, MaterialParamTextureDataRTTI>
	{
	public:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_REFL(Texture, 0)
			B3D_RTTI_MEMBER_PLAIN(IsLoadStore, 1)
			B3D_RTTI_MEMBER_PLAIN(Surface, 2)
			B3D_RTTI_MEMBER_REFL(SpriteImage, 3)
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

		SPtr<IReflectable> NewRttiObject()
		{
			return B3DMakeShared<MaterialParamTextureData>();
		}
	};

	class B3D_CORE_EXPORT MaterialParamStructDataRTTI : public RTTIType<MaterialParamStructData, IReflectable, MaterialParamStructDataRTTI>
	{
	public:
		SPtr<DataStream> GetDataBuffer(MaterialParamStructData* obj, u32& size)
		{
			size = obj->DataSize;

			return B3DMakeShared<MemoryDataStream>(obj->Data, obj->DataSize);
		}

		void SetDataBuffer(MaterialParamStructData* obj, const SPtr<DataStream>& value, u32 size)
		{
			obj->Data = (u8*)B3DAllocate(size);
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

		u32 GetRttiId() const override
		{
			return TID_StructParamData;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return B3DMakeShared<MaterialParamStructData>();
		}
	};

	class B3D_CORE_EXPORT MaterialParamsRTTI : public RTTIType<MaterialParams, IReflectable, MaterialParamsRTTI>
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

			return B3DMakeShared<MemoryDataStream>(obj->mDataParamsBuffer, obj->mDataSize);
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

			B3DFree(param.Data);
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

		void OnOperationStarted(MaterialParams& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(operationType.IsSet(RTTIOperationType::ReadBit))
			{
				for(auto& entry : object.mParamLookup)
				{
					u32 paramIdx = entry.second;
					mMatParams.push_back({ entry.first, paramIdx, object.mParams[paramIdx] });
				}
			}
		}

		void OnDeserializationEnded(IReflectable* obj, RTTIOperationContext* context) override
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

					const GpuDataParameterTypeInformation& typeInfo = GpuParameters::kParamSizes.Lookup[(int)entry.DataType];
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

		u32 GetRttiId() const override
		{
			return TID_MaterialParams;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return B3DMakeShared<MaterialParams>();
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
			B3DRTTIWrite(data.Type, stream);
			B3DRTTIWrite(data.DataType, stream);
			B3DRTTIWrite(data.Index, stream);
			B3DRTTIWrite(data.ArraySize, stream);
			B3DRTTIWrite((u64)0, stream);

			return sizeof(MaterialParamsBase::ParamData);
		}

		static BitLength FromMemory(MaterialParamsBase::ParamData& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			B3DRTTIRead(data.Type, stream);
			B3DRTTIRead(data.DataType, stream);
			B3DRTTIRead(data.Index, stream);
			B3DRTTIRead(data.ArraySize, stream);
			B3DRTTIRead(data.Version, stream);

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

		static BitLength FromMemory(MaterialParamsBase::DataParamInfo& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
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
				B3D_LOG(Error, RTTI, "Unknown version. Unable to deserialize.");
				break;
			}

			return size;
		}

		static BitLength GetSize(const MaterialParamsBase::DataParamInfo& data, const RTTIFieldInfo& fieldInfo, bool compress)
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

			return B3DRTTIWriteWithSizeHeader(stream, data, compress, [&data, &stream]()
											   {
				BitLength size = 0;
				size += B3DRTTIWrite(data.Name, stream);
				size += B3DRTTIWrite(data.Data, stream);

				// Version 1 data
				size += B3DRTTIWrite(kVersion, stream);
				size += B3DRTTIWrite(data.Index, stream);

				return size; });
		}

		static BitLength FromMemory(MaterialParamsRTTI::MaterialParam& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;

			BitLength sizeRead = B3DRTTIReadSizeHeader(stream, compress, size);
			sizeRead += B3DRTTIRead(data.Name, stream);
			sizeRead += B3DRTTIRead(data.Data, stream);

			// More fields means a newer version of the data format
			if(size > sizeRead)
			{
				uint32_t version = 0;
				B3DRTTIRead(version, stream);

				switch(version)
				{
				case 1:
					B3DRTTIRead(data.Index, stream);
					break;
				default:
					B3D_LOG(Error, RTTI, "Unknown version. Unable to deserialize.");
					break;
				}
			}
			else
				data.Index = (uint32_t)-1; // Lets the other code know that index needs to be generated

			return size;
		}

		static BitLength GetSize(const MaterialParamsRTTI::MaterialParam& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size = B3DRTTISize(data.Name) + B3DRTTISize(data.Data) + B3DRTTISize(data.Index) +
				sizeof(uint32_t) * 1;

			B3DRTTIAddHeaderSize(size, compress);
			return size;
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
