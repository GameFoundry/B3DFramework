//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
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
			static constexpr UINT32 VERSION = 1;

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
			memory = rttiWriteElem(VERSION, memory);
			memory = rttiWriteElem(data.attribIdx, memory);
		}

		static UINT32 fromMemory(SHADER_DATA_PARAM_DESC& data, char* memory)
		{
			UINT32 size;
			memcpy(&size, memory, sizeof(UINT32));
			memory += sizeof(UINT32);

			UINT32 sizeRead = sizeof(UINT32);
			memory = rttiReadElem(data.arraySize, memory, sizeRead);
			memory = rttiReadElem(data.rendererSemantic, memory, sizeRead);
			memory = rttiReadElem(data.type, memory, sizeRead);
			memory = rttiReadElem(data.name, memory, sizeRead);
			memory = rttiReadElem(data.gpuVariableName, memory, sizeRead);
			memory = rttiReadElem(data.elementSize, memory, sizeRead);
			memory = rttiReadElem(data.defaultValueIdx, memory, sizeRead);

			// There's more to read, meaning we're reading a newer version of the format
			// (In the first version, version field is missing, so we check this way).
			if(sizeRead < size)
			{
				UINT32 version = 0;
				memory = rttiReadElem(version, memory);
				switch(version)
				{
				case 1:
					memory = rttiReadElem(data.attribIdx, memory);
					break;
				default:
					LOGERR("Unknown version. Unable to deserialize.");
					break;
				}
			}

			return size;
		}

		static UINT32 getDynamicSize(const SHADER_DATA_PARAM_DESC& data)
		{
			UINT64 dataSize = rttiGetElemSize(data.arraySize) + rttiGetElemSize(data.rendererSemantic) + rttiGetElemSize(data.type) +
				rttiGetElemSize(data.name) + rttiGetElemSize(data.gpuVariableName) + rttiGetElemSize(data.elementSize) +
				rttiGetElemSize(data.defaultValueIdx) + rttiGetElemSize(data.attribIdx) + sizeof(UINT32) * 2;

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
			static constexpr UINT32 VERSION = 1;

			UINT32 size = getDynamicSize(data);

			UINT32 curSize = sizeof(UINT32);
			memcpy(memory, &size, curSize);
			memory += curSize;

			memory = rttiWriteElem(data.rendererSemantic, memory);
			memory = rttiWriteElem(data.type, memory);
			memory = rttiWriteElem(data.name, memory);
			memory = rttiWriteElem(data.gpuVariableNames, memory);
			memory = rttiWriteElem(data.defaultValueIdx, memory);
			memory = rttiWriteElem(VERSION, memory);
			memory = rttiWriteElem(data.attribIdx, memory);
		}

		static UINT32 fromMemory(SHADER_OBJECT_PARAM_DESC& data, char* memory)
		{
			UINT32 size;
			memcpy(&size, memory, sizeof(UINT32));
			memory += sizeof(UINT32);

			UINT32 sizeRead = sizeof(UINT32);
			memory = rttiReadElem(data.rendererSemantic, memory, sizeRead);
			memory = rttiReadElem(data.type, memory, sizeRead);
			memory = rttiReadElem(data.name, memory, sizeRead);
			memory = rttiReadElem(data.gpuVariableNames, memory, sizeRead);
			memory = rttiReadElem(data.defaultValueIdx, memory, sizeRead);

			// There's more to read, meaning we're reading a newer version of the format
			// (In the first version, version field is missing, so we check this way).
			if(sizeRead < size)
			{
				UINT32 version = 0;
				memory = rttiReadElem(version, memory);
				switch(version)
				{
				case 1:
					memory = rttiReadElem(data.attribIdx, memory);
					break;
				default:
					LOGERR("Unknown version. Unable to deserialize.");
					break;
				}
			}

			return size;
		}

		static UINT32 getDynamicSize(const SHADER_OBJECT_PARAM_DESC& data)
		{
			UINT64 dataSize = rttiGetElemSize(data.rendererSemantic) + rttiGetElemSize(data.type) +
				rttiGetElemSize(data.name) + rttiGetElemSize(data.gpuVariableNames) +
				rttiGetElemSize(data.defaultValueIdx) + rttiGetElemSize(data.attribIdx) + sizeof(UINT32) * 2;

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
			rttiWriteElem(data.rendererSemantic, memory);
		}

		static UINT32 fromMemory(SHADER_PARAM_BLOCK_DESC& data, char* memory)
		{
			UINT32 size;
			memcpy(&size, memory, sizeof(UINT32));
			memory += sizeof(UINT32);

			memory = rttiReadElem(data.shared, memory);
			memory = rttiReadElem(data.usage, memory);
			memory = rttiReadElem(data.name, memory);
			rttiReadElem(data.rendererSemantic, memory);

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

	template<> struct RTTIPlainType<SHADER_PARAM_ATTRIBUTE>
	{
		enum { id = TID_SHADER_PARAM_ATTRIBUTE }; enum { hasDynamicSize = 1 };

		static void toMemory(const SHADER_PARAM_ATTRIBUTE& data, char* memory)
		{
			static constexpr UINT32 VERSION = 0;

			UINT32 size = getDynamicSize(data);

			UINT32 curSize = sizeof(UINT32);
			memcpy(memory, &size, curSize);
			memory += curSize;

			memory = rttiWriteElem(VERSION, memory);
			memory = rttiWriteElem(data.type, memory);
			memory = rttiWriteElem(data.value, memory);
			memory = rttiWriteElem(data.nextParamIdx, memory);
		}

		static UINT32 fromMemory(SHADER_PARAM_ATTRIBUTE& data, char* memory)
		{
			UINT32 size;
			memcpy(&size, memory, sizeof(UINT32));
			memory += sizeof(UINT32);

			UINT32 version = 0;
			memory = rttiReadElem(version, memory);

			switch(version)
			{
			case 0:
				memory = rttiReadElem(data.type, memory);
				memory = rttiReadElem(data.value, memory);
				memory = rttiReadElem(data.nextParamIdx, memory);
				break;
			default:
				LOGERR("Unknown version. Unable to deserialize.");
				break;
			}

			return size;
		}

		static UINT32 getDynamicSize(const SHADER_PARAM_ATTRIBUTE& data)
		{
			UINT64 dataSize = rttiGetElemSize(data.type) + rttiGetElemSize(data.value) +
				rttiGetElemSize(data.nextParamIdx) + sizeof(UINT32) * 2;

#if BS_DEBUG_MODE
			if(dataSize > std::numeric_limits<UINT32>::max())
			{
				BS_EXCEPT(InternalErrorException, "Data overflow! Size doesn't fit into 32 bits.");
			}
#endif

			return (UINT32)dataSize;
		}
	};

	class BS_CORE_EXPORT SubShaderRTTI : public RTTIType<SubShader, IReflectable, SubShaderRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN(name, 0)
			BS_RTTI_MEMBER_REFLPTR(shader, 1)
		BS_END_RTTI_MEMBERS

	public:
		const String& getRTTIName() override
		{
			static String name = "SubShader";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_SubShader;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<SubShader>();
		}
	};

	class BS_CORE_EXPORT ShaderRTTI : public RTTIType<Shader, Resource, ShaderRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_REFLPTR_ARRAY_NAMED(mTechniques, mDesc.techniques, 0)
			BS_RTTI_MEMBER_PLAIN(mName, 1)

			BS_RTTI_MEMBER_PLAIN_NAMED(mQueueSortType, mDesc.queueSortType, 7)
			BS_RTTI_MEMBER_PLAIN_NAMED(mQueuePriority, mDesc.queuePriority, 8)
			BS_RTTI_MEMBER_PLAIN_NAMED(mSeparablePasses, mDesc.separablePasses, 9)

			BS_RTTI_MEMBER_PLAIN_NAMED(mDataDefaultValues, mDesc.dataDefaultValues, 10)
			BS_RTTI_MEMBER_REFL_ARRAY_NAMED(mTextureDefaultValues, mDesc.textureDefaultValues, 11)
			BS_RTTI_MEMBER_REFLPTR_ARRAY_NAMED(mSamplerDefaultValues, mDesc.samplerDefaultValues, 12)

			BS_RTTI_MEMBER_PLAIN_NAMED(mFlags, mDesc.flags, 13)
			BS_RTTI_MEMBER_REFL_ARRAY_NAMED(mSubShaders, mDesc.subShaders, 14)

			BS_RTTI_MEMBER_PLAIN_ARRAY_NAMED(mParamAttributes, mDesc.paramAttributes, 15)
		BS_END_RTTI_MEMBERS

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

	public:
		ShaderRTTI()
		{
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
