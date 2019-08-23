//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "RTTI/BsStringIDRTTI.h"
#include "RTTI/BsStringRTTI.h"
#include "RTTI/BsStdRTTI.h"
#include "RTTI/BsSmallVectorRTTI.h"
#include "RTTI/BsFlagsRTTI.h"
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

		static uint32_t toMemory(const SHADER_DATA_PARAM_DESC& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo)
		{
			static constexpr UINT32 VERSION = 1;

			return rtti_write_with_size_header(stream, [&data, &stream]()
			{
				uint32_t size = 0;

				size += rttiWriteElem(data.arraySize, stream);
				size += rttiWriteElem(data.rendererSemantic, stream);
				size += rttiWriteElem(data.type, stream);
				size += rttiWriteElem(data.name, stream);
				size += rttiWriteElem(data.gpuVariableName, stream);
				size += rttiWriteElem(data.elementSize, stream);
				size += rttiWriteElem(data.defaultValueIdx, stream);
				size += rttiWriteElem(VERSION, stream);
				size += rttiWriteElem(data.attribIdx, stream);

				return size;
			});
		}

		static uint32_t fromMemory(SHADER_DATA_PARAM_DESC& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo)
		{
			uint32_t sizeRead = 0;
			uint32_t size;
			sizeRead += rttiReadElem(size, stream);
			sizeRead += rttiReadElem(data.arraySize, stream);
			sizeRead += rttiReadElem(data.rendererSemantic, stream);
			sizeRead += rttiReadElem(data.type, stream);
			sizeRead += rttiReadElem(data.name, stream);
			sizeRead += rttiReadElem(data.gpuVariableName, stream);
			sizeRead += rttiReadElem(data.elementSize, stream);
			sizeRead += rttiReadElem(data.defaultValueIdx, stream);

			// There's more to read, meaning we're reading a newer version of the format
			// (In the first version, version field is missing, so we check this way).
			if(sizeRead < size)
			{
				uint32_t version = 0;
				rttiReadElem(version, stream);
				switch(version)
				{
				case 1:
					rttiReadElem(data.attribIdx, stream);
					break;
				default:
					BS_LOG(Error, RTTI, "Unknown version. Unable to deserialize.");
					break;
				}
			}

			return size;
		}

		static uint32_t getDynamicSize(const SHADER_DATA_PARAM_DESC& data)
		{
			uint64_t dataSize = rttiGetElemSize(data.arraySize) + rttiGetElemSize(data.rendererSemantic) + rttiGetElemSize(data.type) +
				rttiGetElemSize(data.name) + rttiGetElemSize(data.gpuVariableName) + rttiGetElemSize(data.elementSize) +
				rttiGetElemSize(data.defaultValueIdx) + rttiGetElemSize(data.attribIdx) + sizeof(uint32_t) * 2;

#if BS_DEBUG_MODE
			if(dataSize > std::numeric_limits<uint32_t>::max())
			{
				BS_EXCEPT(InternalErrorException, "Data overflow! Size doesn't fit into 32 bits.");
			}
#endif

			return (uint32_t)dataSize;
		}
	};

	template<> struct RTTIPlainType<SHADER_OBJECT_PARAM_DESC>
	{
		enum { id = TID_SHADER_OBJECT_PARAM_DESC }; enum { hasDynamicSize = 1 };

		static uint32_t toMemory(const SHADER_OBJECT_PARAM_DESC& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo)
		{
			static constexpr uint32_t VERSION = 1;

			uint32_t size = 0;
			size += rttiWriteElem(data.rendererSemantic, stream);
			size += rttiWriteElem(data.type, stream);
			size += rttiWriteElem(data.name, stream);
			size += rttiWriteElem(data.gpuVariableNames, stream);
			size += rttiWriteElem(data.defaultValueIdx, stream);
			size += rttiWriteElem(VERSION, stream);
			size += rttiWriteElem(data.attribIdx, stream);

			return size;
		}

		static uint32_t fromMemory(SHADER_OBJECT_PARAM_DESC& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo)
		{
			uint32_t size;
			uint32_t sizeRead = rttiReadElem(size, stream);

			sizeRead += rttiReadElem(data.rendererSemantic, stream);
			sizeRead += rttiReadElem(data.type, stream);
			sizeRead += rttiReadElem(data.name, stream);
			sizeRead += rttiReadElem(data.gpuVariableNames, stream);
			sizeRead += rttiReadElem(data.defaultValueIdx, stream);

			// There's more to read, meaning we're reading a newer version of the format
			// (In the first version, version field is missing, so we check this way).
			if(sizeRead < size)
			{
				uint32_t version = 0;
				rttiReadElem(version, stream);
				switch(version)
				{
				case 1:
					rttiReadElem(data.attribIdx, stream);
					break;
				default:
					BS_LOG(Error, RTTI, "Unknown version. Unable to deserialize.");
					break;
				}
			}

			return size;
		}

		static uint32_t getDynamicSize(const SHADER_OBJECT_PARAM_DESC& data)
		{
			uint64_t dataSize = rttiGetElemSize(data.rendererSemantic) + rttiGetElemSize(data.type) +
				rttiGetElemSize(data.name) + rttiGetElemSize(data.gpuVariableNames) +
				rttiGetElemSize(data.defaultValueIdx) + rttiGetElemSize(data.attribIdx) + sizeof(uint32_t) * 2;

#if BS_DEBUG_MODE
			if(dataSize > std::numeric_limits<uint32_t>::max())
			{
				BS_EXCEPT(InternalErrorException, "Data overflow! Size doesn't fit into 32 bits.");
			}
#endif

			return (uint32_t)dataSize;
		}
	};

	template<> struct RTTIPlainType<SHADER_PARAM_BLOCK_DESC>
	{
		enum { id = TID_SHADER_PARAM_BLOCK_DESC }; enum { hasDynamicSize = 1 };

		static uint32_t toMemory(const SHADER_PARAM_BLOCK_DESC& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo)
		{
			return rtti_write_with_size_header(stream, [&data, &stream]()
			{
				uint32_t size = 0;
				size += rttiWriteElem(data.shared, stream);
				size += rttiWriteElem(data.usage, stream);
				size += rttiWriteElem(data.name, stream);
				size += rttiWriteElem(data.rendererSemantic, stream);

				return size;
			});
		}

		static uint32_t fromMemory(SHADER_PARAM_BLOCK_DESC& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo)
		{
			uint32_t size;
			rttiReadElem(size, stream);
			rttiReadElem(data.shared, stream);
			rttiReadElem(data.usage, stream);
			rttiReadElem(data.name, stream);
			rttiReadElem(data.rendererSemantic, stream);

			return size;
		}

		static uint32_t getDynamicSize(const SHADER_PARAM_BLOCK_DESC& data)
		{
			uint64_t dataSize = rttiGetElemSize(data.shared) + rttiGetElemSize(data.usage) +
				rttiGetElemSize(data.name) + rttiGetElemSize(data.rendererSemantic) + sizeof(uint32_t);

#if BS_DEBUG_MODE
			if(dataSize > std::numeric_limits<uint32_t>::max())
			{
				BS_EXCEPT(InternalErrorException, "Data overflow! Size doesn't fit into 32 bits.");
			}
#endif

			return (uint32_t)dataSize;
		}
	};

	template<> struct RTTIPlainType<SHADER_PARAM_ATTRIBUTE>
	{
		enum { id = TID_SHADER_PARAM_ATTRIBUTE }; enum { hasDynamicSize = 1 };

		static uint32_t toMemory(const SHADER_PARAM_ATTRIBUTE& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo)
		{
			static constexpr UINT32 VERSION = 0;

			return rtti_write_with_size_header(stream, [&data, &stream]()
			{
				uint32_t size = 0;
				size += rttiWriteElem(VERSION, stream);
				size += rttiWriteElem(data.type, stream);
				size += rttiWriteElem(data.value, stream);
				size += rttiWriteElem(data.nextParamIdx, stream);

				return size;
			});
		}

		static uint32_t fromMemory(SHADER_PARAM_ATTRIBUTE& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo)
		{
			uint32_t size;
			rttiReadElem(size, stream);

			uint32_t version = 0;
			rttiReadElem(version, stream);

			switch(version)
			{
			case 0:
				rttiReadElem(data.type, stream);
				rttiReadElem(data.value, stream);
				rttiReadElem(data.nextParamIdx, stream);
				break;
			default:
				BS_LOG(Error, RTTI, "Unknown version. Unable to deserialize.");
				break;
			}

			return size;
		}

		static uint32_t getDynamicSize(const SHADER_PARAM_ATTRIBUTE& data)
		{
			uint64_t dataSize = rttiGetElemSize(data.type) + rttiGetElemSize(data.value) +
				rttiGetElemSize(data.nextParamIdx) + sizeof(uint32_t) * 2;

#if BS_DEBUG_MODE
			if(dataSize > std::numeric_limits<uint32_t>::max())
			{
				BS_EXCEPT(InternalErrorException, "Data overflow! Size doesn't fit into 32 bits.");
			}
#endif

			return (uint32_t)dataSize;
		}
	};

	template<>	struct RTTIPlainType<ShaderVariationParamValue>
	{
		enum { id = TID_ShaderVariationParamValue }; enum { hasDynamicSize = 1 };

		/** @copydoc RTTIPlainType::toMemory */
		static uint32_t toMemory(const ShaderVariationParamValue& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo)
		{
			static constexpr uint8_t VERSION = 0;

			return rtti_write_with_size_header(stream, [&data, &stream]()
			{
				uint32_t size = 0;
				size += rttiWriteElem(VERSION, stream);
				size += rttiWriteElem(data.name, stream);
				size += rttiWriteElem(data.value, stream);

				return size;
			});
		}

		/** @copydoc RTTIPlainType::fromMemory */
		static uint32_t fromMemory(ShaderVariationParamValue& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo)
		{
			uint32_t size = 0;
			rttiReadElem(size, stream);

			uint8_t version;
			rttiReadElem(version, stream);
			assert(version == 0);

			rttiReadElem(data.name, stream);
			rttiReadElem(data.value, stream);

			return size;
		}

		/** @copydoc RTTIPlainType::getDynamicSize */
		static uint32_t getDynamicSize(const ShaderVariationParamValue& data)
		{
			uint64_t dataSize = sizeof(uint8_t) + sizeof(uint32_t);
			dataSize += rttiGetElemSize(data.name);
			dataSize += rttiGetElemSize(data.value);

			assert(dataSize <= std::numeric_limits<uint32_t>::max());

			return (uint32_t)dataSize;
		}
	};

	template<>	struct RTTIPlainType<ShaderVariationParamInfo>
	{
		enum { id = TID_ShaderVariationParamInfo }; enum { hasDynamicSize = 1 };

		/** @copydoc RTTIPlainType::toMemory */
		static uint32_t toMemory(const ShaderVariationParamInfo& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo)
		{
			static constexpr uint8_t VERSION = 0;

			return rtti_write_with_size_header(stream, [&data, &stream]()
			{
				uint32_t size = sizeof(uint32_t);
				size += rttiWriteElem(VERSION, stream);
				size += rttiWriteElem(data.name, stream);
				size += rttiWriteElem(data.identifier, stream);
				size += rttiWriteElem(data.isInternal, stream);
				size += rttiWriteElem(data.values, stream);

				return size;
			});
		}

		/** @copydoc RTTIPlainType::fromMemory */
		static uint32_t fromMemory(ShaderVariationParamInfo& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo)
		{
			uint32_t size = 0;
			rttiReadElem(size, stream);

			uint8_t version;
			rttiReadElem(version, stream);
			assert(version == 0);

			rttiReadElem(data.name, stream);
			rttiReadElem(data.identifier, stream);
			rttiReadElem(data.isInternal, stream);
			rttiReadElem(data.values, stream);

			return size;
		}

		/** @copydoc RTTIPlainType::getDynamicSize */
		static uint32_t getDynamicSize(const ShaderVariationParamInfo& data)
		{
			uint64_t dataSize = sizeof(uint8_t) + sizeof(uint32_t);
			dataSize += rttiGetElemSize(data.name);
			dataSize += rttiGetElemSize(data.identifier);
			dataSize += rttiGetElemSize(data.isInternal);
			dataSize += rttiGetElemSize(data.values);

			assert(dataSize <= std::numeric_limits<uint32_t>::max());

			return (uint32_t)dataSize;
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
			BS_RTTI_MEMBER_PLAIN_ARRAY_NAMED(mVariationParams, mDesc.variationParams, 16)
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

		void onDeserializationEnded(IReflectable* obj, SerializationContext* context) override
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
