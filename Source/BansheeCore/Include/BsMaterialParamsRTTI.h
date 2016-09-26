//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsRTTIType.h"
#include "BsMaterialParams.h"
#include "BsSamplerState.h"
#include "BsDataStream.h"

namespace BansheeEngine
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class BS_CORE_EXPORT MaterialParamTextureDataRTTI : public RTTIType<MaterialParamTextureData, IReflectable, MaterialParamTextureDataRTTI>
	{
	public:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_REFL(value, 0)
			BS_RTTI_MEMBER_PLAIN(isLoadStore, 1)
			BS_RTTI_MEMBER_PLAIN(surface, 2)
		BS_END_RTTI_MEMBERS
		
		MaterialParamTextureDataRTTI()
			:mInitMembers(this)
		{ }

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
			MaterialParams::ParamData data;
		};

		MaterialParam& getParamData(MaterialParams* obj, UINT32 idx)
		{
			Vector<MaterialParam>& params = any_cast_ref<Vector<MaterialParam>>(obj->mRTTIData);
			return params[idx];
		}

		void setParamData(MaterialParams* obj, UINT32 idx, MaterialParam& param)
		{
			UINT32 paramIdx = (UINT32)obj->mParams.size();
			obj->mParams.push_back(param.data);

			obj->mParamLookup[param.name] = paramIdx;
		}

		UINT32 getParamDataArraySize(MaterialParams* obj)
		{
			Vector<MaterialParam>& params = any_cast_ref<Vector<MaterialParam>>(obj->mRTTIData);
			return (UINT32)params.size();
		}

		void setParamDataArraySize(MaterialParams* obj, UINT32 size)
		{
			// Do nothing
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
		}

		void onSerializationStarted(IReflectable* obj, const UnorderedMap<String, UINT64>& params) override
		{
			MaterialParams* paramsObj = static_cast<MaterialParams*>(obj);

			Vector<MaterialParam> matParams;
			for (auto& entry : paramsObj->mParamLookup)
			{
				UINT32 paramIdx = entry.second;
				matParams.push_back({ entry.first, paramsObj->mParams[paramIdx] });
			}

			paramsObj->mRTTIData = matParams;
		}

		void onSerializationEnded(IReflectable* obj, const UnorderedMap<String, UINT64>& params) override
		{
			MaterialParams* paramsObj = static_cast<MaterialParams*>(obj);
			paramsObj->mRTTIData = nullptr;
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
			data.dirtyFlags = (UINT32)-1;

			return size;
		}

		static UINT32 getDynamicSize(const MaterialParamsBase::ParamData& data)
		{
			assert(false);
			return 0;
		}
	};

	template<> struct RTTIPlainType<MaterialParamsRTTI::MaterialParam>
	{	
		enum { id = TID_MaterialRTTIParam }; enum { hasDynamicSize = 1 };

		static void toMemory(const MaterialParamsRTTI::MaterialParam& data, char* memory)
		{ 
			UINT32 size = getDynamicSize(data);

			memory = rttiWriteElem(size, memory);
			memory = rttiWriteElem(data.name, memory);
			memory = rttiWriteElem(data.data, memory);
		}

		static UINT32 fromMemory(MaterialParamsRTTI::MaterialParam& data, char* memory)
		{ 
			UINT32 size = 0;
			
			memory = rttiReadElem(size, memory);
			memory = rttiReadElem(data.name, memory);
			memory = rttiReadElem(data.data, memory);

			return size;
		}

		static UINT32 getDynamicSize(const MaterialParamsRTTI::MaterialParam& data)
		{ 
			UINT64 dataSize = rttiGetElemSize(data.name) + rttiGetElemSize(data.data) + sizeof(UINT32);

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