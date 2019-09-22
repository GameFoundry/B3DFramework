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
			BS_RTTI_MEMBER_REFL(texture, 0)
			BS_RTTI_MEMBER_PLAIN(isLoadStore, 1)
			BS_RTTI_MEMBER_PLAIN(surface, 2)
			BS_RTTI_MEMBER_REFL(spriteTexture, 3)
		BS_END_RTTI_MEMBERS

		const String& getRTTIName() override
		{
			static String name = "TextureParamData";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_TextureParamData;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<MaterialParamTextureData>();
		}
	};

	class BS_CORE_EXPORT MaterialParamStructDataRTTI : public RTTIType<MaterialParamStructData, IReflectable, MaterialParamStructDataRTTI>
	{
	public:
		SPtr<DataStream> getDataBuffer(MaterialParamStructData* obj, UINT32& size)
		{
			size = obj->dataSize;

			return bs_shared_ptr_new<MemoryDataStream>(obj->data, obj->dataSize);
		}

		void setDataBuffer(MaterialParamStructData* obj, const SPtr<DataStream>& value, UINT32 size)
		{
			obj->data = (UINT8*)bs_alloc(size);
			value->read(obj->data, size);

			obj->dataSize = size;
		}

		MaterialParamStructDataRTTI()
		{
			addDataBlockField("dataBuffer", 0, &MaterialParamStructDataRTTI::getDataBuffer, &MaterialParamStructDataRTTI::setDataBuffer);
		}

		const String& getRTTIName() override
		{
			static String name = "StructParamData";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_StructParamData;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<MaterialParamStructData>();
		}
	};

	class BS_CORE_EXPORT MaterialParamsRTTI : public RTTIType<MaterialParams, IReflectable, MaterialParamsRTTI>
	{
	public:
		struct MaterialParam
		{
			String name;
			UINT32 index;
			MaterialParams::ParamData data;
		};

		MaterialParam& getParamData(MaterialParams* obj, UINT32 idx)
		{
			return mMatParams[idx];
		}

		void setParamData(MaterialParams* obj, UINT32 idx, MaterialParam& param)
		{
			UINT32 paramIdx = param.index;

			// Older saved files might not have indices preserved
			if(paramIdx == (UINT32)-1)
				paramIdx = mNextParamIdx++;

			if (obj->mParams.size() <= (size_t)paramIdx)
				obj->mParams.resize((size_t)paramIdx + 1);
			
			obj->mParams[paramIdx] = param.data;
			obj->mParamLookup[param.name] = paramIdx;
		}

		UINT32 getParamDataArraySize(MaterialParams* obj)
		{
			return (UINT32)mMatParams.size();
		}

		void setParamDataArraySize(MaterialParams* obj, UINT32 size)
		{
			obj->mParams.resize(size);
		}

		SPtr<DataStream> getDataBuffer(MaterialParams* obj, UINT32& size)
		{
			size = obj->mDataSize;

			return bs_shared_ptr_new<MemoryDataStream>(obj->mDataParamsBuffer, obj->mDataSize);
		}

		void setDataBuffer(MaterialParams* obj, const SPtr<DataStream>& value, UINT32 size)
		{
			obj->mDataParamsBuffer = obj->mAlloc.alloc(size);
			value->read(obj->mDataParamsBuffer, size);

			obj->mDataSize = size;
		}

		MaterialParamStructData& getStructParam(MaterialParams* obj, UINT32 idx) { return obj->mStructParams[idx]; }
		void setStructParam(MaterialParams* obj, UINT32 idx, MaterialParamStructData& param)
		{
			MaterialParamStructData& newStructParam = obj->mStructParams[idx];
			newStructParam.data = (UINT8*)obj->mAlloc.alloc(param.dataSize);
			memcpy(newStructParam.data, param.data, param.dataSize);
			newStructParam.dataSize = param.dataSize;

			bs_free(param.data);
			param.data = nullptr;
		}
		UINT32 getStructArraySize(MaterialParams* obj) { return (UINT32)obj->mNumStructParams; }
		void setStructArraySize(MaterialParams* obj, UINT32 size)
		{
			obj->mNumStructParams = size;
			obj->mStructParams = obj->mAlloc.construct<MaterialParamStructData>(size);
		}

		MaterialParamTextureData& getTextureParam(MaterialParams* obj, UINT32 idx) { return obj->mTextureParams[idx]; }
		void setTextureParam(MaterialParams* obj, UINT32 idx, MaterialParamTextureData& param) { obj->mTextureParams[idx] = param; }
		UINT32 getTextureArraySize(MaterialParams* obj) { return (UINT32)obj->mNumTextureParams; }
		void setTextureArraySize(MaterialParams* obj, UINT32 size)
		{
			obj->mNumTextureParams = size;
			obj->mTextureParams = obj->mAlloc.construct<MaterialParamTextureData>(size);
		}

		SPtr<SamplerState> getSamplerStateParam(MaterialParams* obj, UINT32 idx) { return obj->mSamplerStateParams[idx].value; }
		void setSamplerStateParam(MaterialParams* obj, UINT32 idx, SPtr<SamplerState> param) { obj->mSamplerStateParams[idx].value = param; }
		UINT32 getSamplerStateArraySize(MaterialParams* obj) { return (UINT32)obj->mNumSamplerParams; }
		void setSamplerStateArraySize(MaterialParams* obj, UINT32 size)
		{
			obj->mNumSamplerParams = size;
			obj->mSamplerStateParams = obj->mAlloc.construct<MaterialParamSamplerStateData>(size);
		}

		UINT32& getNumBufferParams(MaterialParams* obj)
		{
			return obj->mNumBufferParams;
		}

		void setNumBufferParams(MaterialParams* obj, UINT32& value)
		{
			obj->mNumBufferParams = value;
			obj->mBufferParams = obj->mAlloc.construct<MaterialParamBufferData>(value);
		}

		MaterialParamsBase::DataParamInfo& getDataParam(MaterialParams* obj, UINT32 idx) { return obj->mDataParams[idx]; }
		void setDataParam(MaterialParams* obj, UINT32 idx, MaterialParamsBase::DataParamInfo& param) { obj->mDataParams[idx] = param; }
		UINT32 getDataParamArraySize(MaterialParams* obj) { return (UINT32)obj->mNumDataParams; }
		void setDataParamArraySize(MaterialParams* obj, UINT32 size)
		{
			obj->mNumDataParams = size;
			obj->mDataParams = obj->mAlloc.construct<MaterialParamsBase::DataParamInfo>(size);
		}

		MaterialParamsRTTI()
		{
			addPlainArrayField("paramData", 0, &MaterialParamsRTTI::getParamData, &MaterialParamsRTTI::getParamDataArraySize,
				&MaterialParamsRTTI::setParamData, &MaterialParamsRTTI::setParamDataArraySize);

			addDataBlockField("dataBuffer", 1, &MaterialParamsRTTI::getDataBuffer, &MaterialParamsRTTI::setDataBuffer);

			addReflectableArrayField("structParams", 2, &MaterialParamsRTTI::getStructParam,
				&MaterialParamsRTTI::getStructArraySize, &MaterialParamsRTTI::setStructParam, &MaterialParamsRTTI::setStructArraySize);

			addReflectableArrayField("textureParams", 3, &MaterialParamsRTTI::getTextureParam,
				&MaterialParamsRTTI::getTextureArraySize, &MaterialParamsRTTI::setTextureParam, &MaterialParamsRTTI::setTextureArraySize);

			addReflectablePtrArrayField("samplerStateParams", 4, &MaterialParamsRTTI::getSamplerStateParam,
				&MaterialParamsRTTI::getSamplerStateArraySize, &MaterialParamsRTTI::setSamplerStateParam, &MaterialParamsRTTI::setSamplerStateArraySize);

			addPlainField("numBufferParams", 5, &MaterialParamsRTTI::getNumBufferParams, &MaterialParamsRTTI::setNumBufferParams);

			addPlainArrayField("dataParams", 6, &MaterialParamsRTTI::getDataParam,
				&MaterialParamsRTTI::getDataParamArraySize, &MaterialParamsRTTI::setDataParam, &MaterialParamsRTTI::setDataParamArraySize);
		}

		void onSerializationStarted(IReflectable* obj, SerializationContext* context) override
		{
			MaterialParams* paramsObj = static_cast<MaterialParams*>(obj);

			for (auto& entry : paramsObj->mParamLookup)
			{
				UINT32 paramIdx = entry.second;
				mMatParams.push_back({ entry.first, paramIdx, paramsObj->mParams[paramIdx] });
			}
		}

		void onDeserializationEnded(IReflectable* obj, SerializationContext* context) override
		{
			MaterialParams* paramsObj = static_cast<MaterialParams*>(obj);

			// This field was added in later versions of the file format, so generate valid data for it if loading from
			// an older serialized version
			if(!paramsObj->mDataParams)
			{
				paramsObj->mNumDataParams = 0;
				for(auto& entry : paramsObj->mParams)
				{
					if(entry.type != MaterialParams::ParamType::Data)
						continue;

					paramsObj->mNumDataParams++;
				}

				paramsObj->mDataParams = (MaterialParams::DataParamInfo*)paramsObj->mAlloc.alloc(
					paramsObj->mNumDataParams * sizeof(MaterialParams::DataParamInfo));
				memset(paramsObj->mDataParams, 0, paramsObj->mNumDataParams * sizeof(MaterialParams::DataParamInfo));

				UINT32 paramIdx = 0;
				UINT32 dataBufferIdx = 0;
				for(auto& entry : paramsObj->mParams)
				{
					if(entry.type != MaterialParams::ParamType::Data)
						continue;

					const GpuParamDataTypeInfo& typeInfo = GpuParams::PARAM_SIZES.lookup[(int)entry.dataType];
					const UINT32 paramSize = typeInfo.numColumns * typeInfo.numRows * typeInfo.baseTypeSize;
					for (UINT32 i = 0; i < entry.arraySize; i++)
					{
						paramsObj->mDataParams[paramIdx + i].offset = dataBufferIdx;

						dataBufferIdx += paramSize;
					}

					entry.index = paramIdx;
					paramIdx += entry.arraySize;
				}
			}
		}

		const String& getRTTIName() override
		{
			static String name = "MaterialParams";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_MaterialParams;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<MaterialParams>();
		}

	private:
		Vector<MaterialParam> mMatParams;
		UINT32 mNextParamIdx = 0;
	};

	template<> struct RTTIPlainType<MaterialParamsBase::ParamData>
	{
		enum { id = TID_MaterialParamData }; enum { hasDynamicSize = 0 };

		static BitLength toMemory(const MaterialParamsBase::ParamData& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			rtti_write(data.type, stream);
			rtti_write(data.dataType, stream);
			rtti_write(data.index, stream);
			rtti_write(data.arraySize, stream);
			rtti_write((UINT64)0, stream);

			return sizeof(MaterialParamsBase::ParamData);
		}

		static BitLength fromMemory(MaterialParamsBase::ParamData& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			rtti_read(data.type, stream);
			rtti_read(data.dataType, stream);
			rtti_read(data.index, stream);
			rtti_read(data.arraySize, stream);
			rtti_read(data.version, stream);

			// Not a field we should serialize, but we do because this struct is serialized as a whole
			data.version = 1;
			return sizeof(MaterialParamsBase::ParamData);
		}

		static BitLength getSize(const MaterialParamsBase::ParamData& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			return sizeof(MaterialParamsBase::ParamData);
		}
	};

	template<> struct RTTIPlainType<MaterialParamsBase::DataParamInfo>
	{
		enum { id = TID_DataParamInfo }; enum { hasDynamicSize = 1 };

		static BitLength toMemory(const MaterialParamsBase::DataParamInfo& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			static constexpr uint32_t VERSION = 1;

			return rtti_write_with_size_header(stream, data, compress, [&data, &stream]()
			{
				BitLength size = 0;
				size += rtti_write(VERSION, stream);
				size += rtti_write(data.offset, stream);

				uint32_t curveType = 0; // No curve

				if (data.floatCurve)
					curveType = 1;
				else if (data.colorGradient)
					curveType = 2;
				else if (data.spriteTextureIdx != (uint32_t)-1)
					curveType = 3;

				size += rtti_write(curveType, stream);
				if (data.floatCurve)
					size += rtti_write(*data.floatCurve, stream);
				else if (data.colorGradient)
					size += rtti_write(*data.colorGradient, stream);
				else if (data.spriteTextureIdx != (uint32_t)-1)
					size += rtti_write(data.spriteTextureIdx, stream);

				return size;
			});
		}

		static BitLength fromMemory(MaterialParamsBase::DataParamInfo& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
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
				rtti_read(data.offset, stream);
				
				uint32_t curveType = 0;
				rtti_read(curveType, stream);

				data.floatCurve = nullptr;
				data.colorGradient = nullptr;
				data.spriteTextureIdx = (uint32_t)-1;

				switch(curveType)
				{
				case 1:
					data.floatCurve = bs_pool_new<TAnimationCurve<float>>();
					rtti_read(*data.floatCurve, stream);
					break;
				case 2:
					if(version == 0)
					{
						// Version 0 stores non-HDR gradients
						ColorGradient temp;
						rtti_read(temp, stream);

						data.colorGradient = bs_pool_new<ColorGradientHDR>(temp.getKeys());
					}
					else
					{
						data.colorGradient = bs_pool_new<ColorGradientHDR>();
						rtti_read(*data.colorGradient, stream);
					}

					break;
				case 3:
					rtti_read(data.spriteTextureIdx, stream);
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

		static BitLength getSize(const MaterialParamsBase::DataParamInfo& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size = rtti_size(data.offset) + sizeof(uint32_t) * 2;

			if(data.floatCurve)
				size += rtti_size(*data.floatCurve);
			else if(data.colorGradient)
				size += rtti_size(*data.colorGradient);
			else if(data.spriteTextureIdx != (uint32_t)-1)
				size += rtti_size(data.spriteTextureIdx);

			rtti_add_header_size(size, compress);
			return size;
		}
	};

	template<> struct RTTIPlainType<MaterialParamsRTTI::MaterialParam>
	{	
		enum { id = TID_MaterialRTTIParam }; enum { hasDynamicSize = 1 };

		static BitLength toMemory(const MaterialParamsRTTI::MaterialParam& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			static constexpr UINT32 VERSION = 1;

			return rtti_write_with_size_header(stream, data, compress, [&data, &stream]()
			{
				BitLength size = 0;
				size += rtti_write(data.name, stream);
				size += rtti_write(data.data, stream);

				// Version 1 data
				size += rtti_write(VERSION, stream);
				size += rtti_write(data.index, stream);

				return size;
			});
		}

		static BitLength fromMemory(MaterialParamsRTTI::MaterialParam& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			
			BitLength sizeRead = rtti_read_size_header(stream, compress, size);
			sizeRead += rtti_read(data.name, stream);
			sizeRead += rtti_read(data.data, stream);

			// More fields means a newer version of the data format
			if(size > sizeRead)
			{
				uint32_t version = 0;
				rtti_read(version, stream);

				switch (version)
				{
				case 1:
					rtti_read(data.index, stream);
					break;
				default:
					BS_LOG(Error, RTTI, "Unknown version. Unable to deserialize.");
					break;
				}
			}
			else
				data.index = (uint32_t)-1; // Lets the other code know that index needs to be generated

			return size;
		}

		static BitLength getSize(const MaterialParamsRTTI::MaterialParam& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size = rtti_size(data.name) + rtti_size(data.data) + rtti_size(data.index) +
				sizeof(uint32_t) * 1;

			rtti_add_header_size(size, compress);
			return size;
		}	
	};

	/** @} */
	/** @endcond */
}
