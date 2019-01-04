//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Material/BsMaterialParams.h"
#include "RenderAPI/BsSamplerState.h"
#include "FileSystem/BsDataStream.h"
#include "Animation/BsAnimationCurve.h"
#include "Image/BsColorGradient.h"
#include "Private/RTTI/BsAnimationCurveRTTI.h"
#include "Private/RTTI/BsColorGradientRTTI.h"

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

			return bs_shared_ptr_new<MemoryDataStream>(obj->data, obj->dataSize, false);
		}

		void setDataBuffer(MaterialParamStructData* obj, const SPtr<DataStream>& value, UINT32 size)
		{
			obj->data = (UINT8*)bs_alloc(size);
			value->read(obj->data, size);

			obj->dataSize = size;
		}

		MaterialParamStructDataRTTI()
		{
			addDataBlockField("dataBuffer", 0, &MaterialParamStructDataRTTI::getDataBuffer, &MaterialParamStructDataRTTI::setDataBuffer, 0);
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
			if(paramIdx == (UINT32)-1)
				paramIdx = (UINT32)obj->mParams.size();

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

			return bs_shared_ptr_new<MemoryDataStream>(obj->mDataParamsBuffer, obj->mDataSize, false);
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

			addDataBlockField("dataBuffer", 1, &MaterialParamsRTTI::getDataBuffer, &MaterialParamsRTTI::setDataBuffer, 0);

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
	};

	template<> struct RTTIPlainType<MaterialParamsBase::ParamData>
	{
		enum { id = TID_MaterialParamData }; enum { hasDynamicSize = 0 };

		static void toMemory(const MaterialParamsBase::ParamData& data, char* memory)
		{
			memory = rttiWriteElem(data.type, memory);
			memory = rttiWriteElem(data.dataType, memory);
			memory = rttiWriteElem(data.index, memory);
			memory = rttiWriteElem(data.arraySize, memory);
		}

		static UINT32 fromMemory(MaterialParamsBase::ParamData& data, char* memory)
		{
			UINT32 size = 0;
			memory = rttiReadElem(data.type, memory, size);
			memory = rttiReadElem(data.dataType, memory, size);
			memory = rttiReadElem(data.index, memory, size);
			memory = rttiReadElem(data.arraySize, memory, size);

			data.version = 1;
			size += sizeof(data.version);

			return size;
		}

		static UINT32 getDynamicSize(const MaterialParamsBase::ParamData& data)
		{
			assert(false);
			return 0;
		}
	};

	template<> struct RTTIPlainType<MaterialParamsBase::DataParamInfo>
	{
		enum { id = TID_DataParamInfo }; enum { hasDynamicSize = 1 };

		static void toMemory(const MaterialParamsBase::DataParamInfo& data, char* memory)
		{
			static constexpr UINT32 VERSION = 0;

			const UINT32 size = getDynamicSize(data);
			memory = rttiWriteElem(size, memory);
			memory = rttiWriteElem(VERSION, memory);

			memory = rttiWriteElem(data.offset, memory);

			UINT32 curveType = 0; // No curve

			if(data.floatCurve)
				curveType = 1;
			else if(data.colorGradient)
				curveType = 2;
			else if(data.spriteTextureIdx != (UINT32)-1)
				curveType = 3;

			memory = rttiWriteElem(curveType, memory);
			if(data.floatCurve)
				memory = rttiWriteElem(*data.floatCurve, memory);
			else if(data.colorGradient)
				memory = rttiWriteElem(*data.colorGradient, memory);
			else if(data.spriteTextureIdx != (UINT32)-1)
				memory = rttiWriteElem(data.spriteTextureIdx, memory);
		}

		static UINT32 fromMemory(MaterialParamsBase::DataParamInfo& data, char* memory)
		{
			UINT32 size = 0;
			memory = rttiReadElem(size, memory);

			UINT32 version = 0;
			memory = rttiReadElem(version, memory);

			switch(version)
			{
			case 0:
			{
				memory = rttiReadElem(data.offset, memory);
				
				UINT32 curveType = 0;
				memory = rttiReadElem(curveType, memory);

				data.floatCurve = nullptr;
				data.colorGradient = nullptr;
				data.spriteTextureIdx = (UINT32)-1;

				switch(curveType)
				{
				case 1:
					data.floatCurve = bs_pool_new<TAnimationCurve<float>>();
					memory = rttiReadElem(*data.floatCurve, memory);
					break;
				case 2:
					data.colorGradient = bs_pool_new<ColorGradient>();
					memory = rttiReadElem(*data.colorGradient, memory);
					break;
				case 3:
					memory = rttiReadElem(data.spriteTextureIdx, memory);
					break;
				default:
					break;
				}
			}
				break;
			default:
				LOGERR("Unknown version. Unable to deserialize.");
				break;
			}

			return size;
		}

		static UINT32 getDynamicSize(const MaterialParamsBase::DataParamInfo& data)
		{
			UINT32 size = sizeof(UINT32) * 3 + rttiGetElemSize(data.offset);

			if(data.floatCurve)
				size += rttiGetElemSize(*data.floatCurve);
			else if(data.colorGradient)
				size += rttiGetElemSize(*data.colorGradient);
			else if(data.spriteTextureIdx != (UINT32)-1)
				size += rttiGetElemSize(data.spriteTextureIdx);

			return size;
		}
	};

	template<> struct RTTIPlainType<MaterialParamsRTTI::MaterialParam>
	{	
		enum { id = TID_MaterialRTTIParam }; enum { hasDynamicSize = 1 };

		static void toMemory(const MaterialParamsRTTI::MaterialParam& data, char* memory)
		{ 
			static constexpr UINT32 VERSION = 1;

			const UINT32 size = getDynamicSize(data);

			memory = rttiWriteElem(size, memory);
			memory = rttiWriteElem(data.name, memory);
			memory = rttiWriteElem(data.data, memory);

			// Version 1 data
			memory = rttiWriteElem(VERSION, memory);
			memory = rttiWriteElem(data.index, memory);
		}

		static UINT32 fromMemory(MaterialParamsRTTI::MaterialParam& data, char* memory)
		{ 
			UINT32 size = 0;
			UINT32 sizeRead = 0;
			
			memory = rttiReadElem(size, memory, sizeRead);
			memory = rttiReadElem(data.name, memory, sizeRead);
			memory = rttiReadElem(data.data, memory, sizeRead);

			// More fields means a newer version of the data format
			if(size > sizeRead)
			{
				UINT32 version = 0;
				memory = rttiReadElem(version, memory);

				switch (version)
				{
				case 1:
					memory = rttiReadElem(data.index, memory);
					break;
				default:
					LOGERR("Unknown version. Unable to deserialize.");
					break;
				}
			}
			else
				data.index = (UINT32)-1; // Lets the other code know that index needs to be generated

			return size;
		}

		static UINT32 getDynamicSize(const MaterialParamsRTTI::MaterialParam& data)
		{ 
			const UINT64 dataSize = rttiGetElemSize(data.name) + rttiGetElemSize(data.data) + rttiGetElemSize(data.index) + 
				sizeof(UINT32) * 2;

#if BS_DEBUG_MODE
			if(dataSize > std::numeric_limits<UINT32>::max())
			{
				__string_throwDataOverflowException();
			}
#endif

			return (UINT32)dataSize;
		}	
	}; 

	/** @} */
	/** @endcond */
}
