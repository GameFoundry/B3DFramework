//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Material/BsShader.h"
#include "Material/BsMaterial.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	template<> struct RTTIPlainType<SHADER_DATA_PARAM_DESC>
	{	
		enum { id = TID_SHADER_DATA_PARAM_DESC }; enum { hasDynamicSize = 1 };

		static void toMemory(const SHADER_DATA_PARAM_DESC& data, char* memory)
		{ 
			UINT32 size = getDynamicSize(data);

			UINT32 curSize = sizeof(UINT32);
			memcpy(memory, &size, curSize);
			memory += curSize;

			memory = rttiWriteElem(data.arraySize, memory);
			memory = rttiWriteElem(data.rendererSemantic, memory);
			memory = rttiWriteElem(data.type, memory);
			memory = rttiWriteElem(data.name, memory);
			memory = rttiWriteElem(data.gpuVariableName, memory);
			memory = rttiWriteElem(data.elementSize, memory);
			memory = rttiWriteElem(data.defaultValueIdx, memory);
		}

		static UINT32 fromMemory(SHADER_DATA_PARAM_DESC& data, char* memory)
		{ 
			UINT32 size;
			memcpy(&size, memory, sizeof(UINT32)); 
			memory += sizeof(UINT32);

			memory = rttiReadElem(data.arraySize, memory);
			memory = rttiReadElem(data.rendererSemantic, memory);
			memory = rttiReadElem(data.type, memory);
			memory = rttiReadElem(data.name, memory);
			memory = rttiReadElem(data.gpuVariableName, memory);
			memory = rttiReadElem(data.elementSize, memory);
			memory = rttiReadElem(data.defaultValueIdx, memory);

			return size;
		}

		static UINT32 getDynamicSize(const SHADER_DATA_PARAM_DESC& data)	
		{ 
			UINT64 dataSize = rttiGetElemSize(data.arraySize) + rttiGetElemSize(data.rendererSemantic) + rttiGetElemSize(data.type) +
				rttiGetElemSize(data.name) + rttiGetElemSize(data.gpuVariableName) + rttiGetElemSize(data.elementSize) + 
				rttiGetElemSize(data.defaultValueIdx) + sizeof(UINT32);

#if BS_DEBUG_MODE
			if(dataSize > std::numeric_limits<UINT32>::max())
			{
				BS_EXCEPT(InternalErrorException, "Data overflow! Size doesn't fit into 32 bits.");
			}
#endif

			return (UINT32)dataSize;
		}	
	}; 

	template<> struct RTTIPlainType<SHADER_OBJECT_PARAM_DESC>
	{	
		enum { id = TID_SHADER_OBJECT_PARAM_DESC }; enum { hasDynamicSize = 1 };

		static void toMemory(const SHADER_OBJECT_PARAM_DESC& data, char* memory)
		{ 
			UINT32 size = getDynamicSize(data);

			UINT32 curSize = sizeof(UINT32);
			memcpy(memory, &size, curSize);
			memory += curSize;

			memory = rttiWriteElem(data.rendererSemantic, memory);
			memory = rttiWriteElem(data.type, memory);
			memory = rttiWriteElem(data.name, memory);
			memory = rttiWriteElem(data.gpuVariableNames, memory);
			memory = rttiWriteElem(data.defaultValueIdx, memory);
		}

		static UINT32 fromMemory(SHADER_OBJECT_PARAM_DESC& data, char* memory)
		{ 
			UINT32 size;
			memcpy(&size, memory, sizeof(UINT32)); 
			memory += sizeof(UINT32);

			memory = rttiReadElem(data.rendererSemantic, memory);
			memory = rttiReadElem(data.type, memory);
			memory = rttiReadElem(data.name, memory);
			memory = rttiReadElem(data.gpuVariableNames, memory);
			memory = rttiReadElem(data.defaultValueIdx, memory);

			return size;
		}

		static UINT32 getDynamicSize(const SHADER_OBJECT_PARAM_DESC& data)	
		{ 
			UINT64 dataSize = rttiGetElemSize(data.rendererSemantic) + rttiGetElemSize(data.type) +
				rttiGetElemSize(data.name) + rttiGetElemSize(data.gpuVariableNames) + 
				rttiGetElemSize(data.defaultValueIdx) + sizeof(UINT32);

#if BS_DEBUG_MODE
			if(dataSize > std::numeric_limits<UINT32>::max())
			{
				BS_EXCEPT(InternalErrorException, "Data overflow! Size doesn't fit into 32 bits.");
			}
#endif

			return (UINT32)dataSize;
		}	
	}; 

	template<> struct RTTIPlainType<SHADER_PARAM_BLOCK_DESC>
	{	
		enum { id = TID_SHADER_PARAM_BLOCK_DESC }; enum { hasDynamicSize = 1 };

		static void toMemory(const SHADER_PARAM_BLOCK_DESC& data, char* memory)
		{ 
			UINT32 size = getDynamicSize(data);

			UINT32 curSize = sizeof(UINT32);
			memcpy(memory, &size, curSize);
			memory += curSize;

			memory = rttiWriteElem(data.shared, memory);
			memory = rttiWriteElem(data.usage, memory);
			memory = rttiWriteElem(data.name, memory);
			memory = rttiWriteElem(data.rendererSemantic, memory);
		}

		static UINT32 fromMemory(SHADER_PARAM_BLOCK_DESC& data, char* memory)
		{ 
			UINT32 size;
			memcpy(&size, memory, sizeof(UINT32)); 
			memory += sizeof(UINT32);

			memory = rttiReadElem(data.shared, memory);
			memory = rttiReadElem(data.usage, memory);
			memory = rttiReadElem(data.name, memory);
			memory = rttiReadElem(data.rendererSemantic, memory);

			return size;
		}

		static UINT32 getDynamicSize(const SHADER_PARAM_BLOCK_DESC& data)	
		{ 
			UINT64 dataSize = rttiGetElemSize(data.shared) + rttiGetElemSize(data.usage) + 
				rttiGetElemSize(data.name) + rttiGetElemSize(data.rendererSemantic) + sizeof(UINT32);

#if BS_DEBUG_MODE
			if(dataSize > std::numeric_limits<UINT32>::max())
			{
				BS_EXCEPT(InternalErrorException, "Data overflow! Size doesn't fit into 32 bits.");
			}
#endif

			return (UINT32)dataSize;
		}	
	}; 

	class BS_CORE_EXPORT ShaderRTTI : public RTTIType<Shader, Resource, ShaderRTTI>
	{
	private:
		SPtr<Technique> getTechnique(Shader* obj, UINT32 idx) { return obj->mTechniques[idx]; }
		void setTechnique(Shader* obj, UINT32 idx, SPtr<Technique> val) { obj->mTechniques[idx] = val; }
		UINT32 getTechniqueArraySize(Shader* obj) { return (UINT32)obj->mTechniques.size(); }
		void setTechniqueArraySize(Shader* obj, UINT32 size) { obj->mTechniques.resize(size); }

		String& getName(Shader* obj) { return obj->mName; }
		void setName(Shader* obj, String& name) { obj->mName = name; }

		SHADER_DATA_PARAM_DESC& getDataParam(Shader* obj, UINT32 idx) 
		{ 
			auto iter = obj->mDesc.dataParams.begin();
			for(UINT32 i = 0; i < idx; i++) ++iter;

			return iter->second; 
		}

		void setDataParam(Shader* obj, UINT32 idx, SHADER_DATA_PARAM_DESC& val) { obj->mDesc.dataParams[val.name] = val; }
		UINT32 getDataParamsArraySize(Shader* obj) { return (UINT32)obj->mDesc.dataParams.size(); }
		void setDataParamsArraySize(Shader* obj, UINT32 size) {  } // Do nothing

		SHADER_OBJECT_PARAM_DESC& getTextureParam(Shader* obj, UINT32 idx) 
		{ 
			auto iter = obj->mDesc.textureParams.begin();
			for(UINT32 i = 0; i < idx; i++) ++iter;

			return iter->second; 
		}

		void setTextureParam(Shader* obj, UINT32 idx, SHADER_OBJECT_PARAM_DESC& val) { obj->mDesc.textureParams[val.name] = val; }
		UINT32 getTextureParamsArraySize(Shader* obj) { return (UINT32)obj->mDesc.textureParams.size(); }
		void setTextureParamsArraySize(Shader* obj, UINT32 size) {  } // Do nothing

		SHADER_OBJECT_PARAM_DESC& getSamplerParam(Shader* obj, UINT32 idx)
		{
			auto iter = obj->mDesc.samplerParams.begin();
			for (UINT32 i = 0; i < idx; i++) ++iter;

			return iter->second;
		}

		void setSamplerParam(Shader* obj, UINT32 idx, SHADER_OBJECT_PARAM_DESC& val) { obj->mDesc.samplerParams[val.name] = val; }
		UINT32 getSamplerParamsArraySize(Shader* obj) { return (UINT32)obj->mDesc.samplerParams.size(); }
		void setSamplerParamsArraySize(Shader* obj, UINT32 size) {  } // Do nothing

		SHADER_OBJECT_PARAM_DESC& getBufferParam(Shader* obj, UINT32 idx)
		{
			auto iter = obj->mDesc.bufferParams.begin();
			for (UINT32 i = 0; i < idx; i++) ++iter;

			return iter->second;
		}

		void setBufferParam(Shader* obj, UINT32 idx, SHADER_OBJECT_PARAM_DESC& val) { obj->mDesc.bufferParams[val.name] = val; }
		UINT32 getBufferParamsArraySize(Shader* obj) { return (UINT32)obj->mDesc.bufferParams.size(); }
		void setBufferParamsArraySize(Shader* obj, UINT32 size) {  } // Do nothing

		SHADER_PARAM_BLOCK_DESC& getParamBlock(Shader* obj, UINT32 idx)
		{
			auto iter = obj->mDesc.paramBlocks.begin();
			for (UINT32 i = 0; i < idx; i++) ++iter;

			return iter->second;
		}

		void setParamBlock(Shader* obj, UINT32 idx, SHADER_PARAM_BLOCK_DESC& val) { obj->mDesc.paramBlocks[val.name] = val; }
		UINT32 getParamBlocksArraySize(Shader* obj) { return (UINT32)obj->mDesc.paramBlocks.size(); }
		void setParamBlocksArraySize(Shader* obj, UINT32 size) {  } // Do nothing

		UINT32& getQueueSortType(Shader* obj) { return (UINT32&)obj->mDesc.queueSortType; }
		void setQueueSortType(Shader* obj, UINT32& value) { obj->mDesc.queueSortType = (QueueSortType)value; }

		INT32& getQueuePriority(Shader* obj) { return obj->mDesc.queuePriority; }
		void setQueuePriority(Shader* obj, INT32& value) { obj->mDesc.queuePriority = value; }

		bool& getAllowSeparablePasses(Shader* obj) { return obj->mDesc.separablePasses; }
		void setAllowSeparablePasses(Shader* obj, bool& value) { obj->mDesc.separablePasses = value; }

		UINT32& getFlags(Shader* obj) { return obj->mDesc.flags; }
		void setFlags(Shader* obj, UINT32& value) { obj->mDesc.flags = value; }

		Vector<UINT8>& getDataDefaultValues(Shader* obj) { return obj->mDesc.dataDefaultValues; }
		void setDataDefaultValues(Shader* obj, Vector<UINT8>& value) { obj->mDesc.dataDefaultValues = value; }

		HTexture& getDefaultTexture(Shader* obj, UINT32 idx) { return obj->mDesc.textureDefaultValues[idx]; }
		void setDefaultTexture(Shader* obj, UINT32 idx, HTexture& val) { obj->mDesc.textureDefaultValues[idx] = val; }
		UINT32 getDefaultTextureArraySize(Shader* obj) { return (UINT32)obj->mDesc.textureDefaultValues.size(); }
		void setDefaultTextureArraySize(Shader* obj, UINT32 size) { obj->mDesc.textureDefaultValues.resize(size); }

		SPtr<SamplerState> getDefaultSampler(Shader* obj, UINT32 idx) { return obj->mDesc.samplerDefaultValues[idx]; }
		void setDefaultSampler(Shader* obj, UINT32 idx, SPtr<SamplerState> val) { obj->mDesc.samplerDefaultValues[idx] = val; }
		UINT32 getDefaultSamplerArraySize(Shader* obj) { return (UINT32)obj->mDesc.samplerDefaultValues.size(); }
		void setDefaultSamplerArraySize(Shader* obj, UINT32 size) { obj->mDesc.samplerDefaultValues.resize(size); }

	public:
		ShaderRTTI()
		{
			addReflectablePtrArrayField("mTechniques", 0, &ShaderRTTI::getTechnique, &ShaderRTTI::getTechniqueArraySize, 
				&ShaderRTTI::setTechnique, &ShaderRTTI::setTechniqueArraySize);
			addPlainField("mName", 1, &ShaderRTTI::getName, &ShaderRTTI::setName);

			addPlainArrayField("mDataParams", 2, &ShaderRTTI::getDataParam, &ShaderRTTI::getDataParamsArraySize, 
				&ShaderRTTI::setDataParam, &ShaderRTTI::setDataParamsArraySize);
			addPlainArrayField("mTextureParams", 3, &ShaderRTTI::getTextureParam, &ShaderRTTI::getTextureParamsArraySize,
				&ShaderRTTI::setTextureParam, &ShaderRTTI::setTextureParamsArraySize);
			addPlainArrayField("mSamplerParams", 4, &ShaderRTTI::getSamplerParam, &ShaderRTTI::getSamplerParamsArraySize,
				&ShaderRTTI::setSamplerParam, &ShaderRTTI::setSamplerParamsArraySize);
			addPlainArrayField("mBufferParams", 5, &ShaderRTTI::getBufferParam, &ShaderRTTI::getBufferParamsArraySize,
				&ShaderRTTI::setBufferParam, &ShaderRTTI::setBufferParamsArraySize);
			addPlainArrayField("mParamBlocks", 6, &ShaderRTTI::getParamBlock, &ShaderRTTI::getParamBlocksArraySize, 
				&ShaderRTTI::setParamBlock, &ShaderRTTI::setParamBlocksArraySize);

			addPlainField("mQueueSortType", 7, &ShaderRTTI::getQueueSortType, &ShaderRTTI::setQueueSortType);
			addPlainField("mQueuePriority", 8, &ShaderRTTI::getQueuePriority, &ShaderRTTI::setQueuePriority);
			addPlainField("mSeparablePasses", 9, &ShaderRTTI::getAllowSeparablePasses, &ShaderRTTI::setAllowSeparablePasses);

			addPlainField("mDataDefaultValues", 10, &ShaderRTTI::getDataDefaultValues, &ShaderRTTI::setDataDefaultValues);
			addReflectableArrayField("mTextureDefaultValues", 11, &ShaderRTTI::getDefaultTexture, &ShaderRTTI::getDefaultTextureArraySize,
				&ShaderRTTI::setDefaultTexture, &ShaderRTTI::setDefaultTextureArraySize);
			addReflectablePtrArrayField("mSamplerDefaultValues", 12, &ShaderRTTI::getDefaultSampler, &ShaderRTTI::getDefaultSamplerArraySize,
				&ShaderRTTI::setDefaultSampler, &ShaderRTTI::setDefaultSamplerArraySize);

			addPlainField("mFlags", 13, &ShaderRTTI::getFlags, &ShaderRTTI::setFlags);
		}

		void onDeserializationEnded(IReflectable* obj, const UnorderedMap<String, UINT64>& params) override
		{
			Shader* shader = static_cast<Shader*>(obj);
			shader->initialize();
		}

		const String& getRTTIName() override
		{
			static String name = "Shader";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_Shader;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return Shader::createEmpty();
		}
	};

	class BS_CORE_EXPORT ShaderMetaDataRTTI : public RTTIType < ShaderMetaData, ResourceMetaData, ShaderMetaDataRTTI >
	{
	private:
		Vector<String>& getIncludes(ShaderMetaData* obj) { return obj->includes; }
		void setIncludes(ShaderMetaData* obj, Vector<String>& includes) { obj->includes = includes; }

	public:
		ShaderMetaDataRTTI()
		{
			addPlainField("includes", 0, &ShaderMetaDataRTTI::getIncludes, &ShaderMetaDataRTTI::setIncludes);
		}

		const String& getRTTIName() override
		{
			static String name = "ShaderMetaData";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_ShaderMetaData;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<ShaderMetaData>();
		}
	};

	/** @} */
	/** @endcond */
}