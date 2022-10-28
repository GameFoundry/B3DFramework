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

	template <>
	struct RTTIPlainType<SHADER_DATA_PARAM_DESC>
	{
		enum
		{
			id = TID_SHADER_DATA_PARAM_DESC
		};

		enum
		{
			hasDynamicSize = 1
		};

		static BitLength ToMemory(const SHADER_DATA_PARAM_DESC& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			static constexpr u32 kVersion = 1;

			return rtti_write_with_size_header(stream, data, compress, [&data, &stream]()
											   {
				BitLength size = 0;

				size += rtti_write(data.ArraySize, stream);
				size += rtti_write(data.RendererSemantic, stream);
				size += rtti_write(data.Type, stream);
				size += rtti_write(data.Name, stream);
				size += rtti_write(data.GpuVariableName, stream);
				size += rtti_write(data.ElementSize, stream);
				size += rtti_write(data.DefaultValueIdx, stream);
				size += rtti_write(kVersion, stream);
				size += rtti_write(data.AttribIdx, stream);

				return size; });
		}

		static BitLength FromMemory(SHADER_DATA_PARAM_DESC& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			BitLength sizeRead = rtti_read_size_header(stream, compress, size);

			sizeRead += rtti_read(data.ArraySize, stream);
			sizeRead += rtti_read(data.RendererSemantic, stream);
			sizeRead += rtti_read(data.Type, stream);
			sizeRead += rtti_read(data.Name, stream);
			sizeRead += rtti_read(data.GpuVariableName, stream);
			sizeRead += rtti_read(data.ElementSize, stream);
			sizeRead += rtti_read(data.DefaultValueIdx, stream);

			// There's more to read, meaning we're reading a newer version of the format
			// (In the first version, version field is missing, so we check this way).
			if(sizeRead < size)
			{
				uint32_t version = 0;
				rtti_read(version, stream);
				switch(version)
				{
				case 1:
					rtti_read(data.AttribIdx, stream);
					break;
				default:
					BS_LOG(Error, RTTI, "Unknown version. Unable to deserialize.");
					break;
				}
			}

			return size;
		}

		static BitLength GetSize(const SHADER_DATA_PARAM_DESC& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize = rtti_size(data.ArraySize) + rtti_size(data.RendererSemantic) + rtti_size(data.Type) +
				rtti_size(data.Name) + rtti_size(data.GpuVariableName) + rtti_size(data.ElementSize) +
				rtti_size(data.DefaultValueIdx) + rtti_size(data.AttribIdx) + sizeof(uint32_t);

			rtti_add_header_size(dataSize, compress);
			return dataSize;
		}
	};

	template <>
	struct RTTIPlainType<SHADER_OBJECT_PARAM_DESC>
	{
		enum
		{
			id = TID_SHADER_OBJECT_PARAM_DESC
		};

		enum
		{
			hasDynamicSize = 1
		};

		static BitLength ToMemory(const SHADER_OBJECT_PARAM_DESC& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			static constexpr uint32_t kVersion = 1;

			return rtti_write_with_size_header(stream, data, compress, [&data, &stream]()
											   {
				BitLength size = 0;
				size += rtti_write(data.RendererSemantic, stream);
				size += rtti_write(data.Type, stream);
				size += rtti_write(data.Name, stream);
				size += rtti_write(data.GpuVariableNames, stream);
				size += rtti_write(data.DefaultValueIdx, stream);
				size += rtti_write(kVersion, stream);
				size += rtti_write(data.AttribIdx, stream);

				return size; });
		}

		static BitLength FromMemory(SHADER_OBJECT_PARAM_DESC& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			BitLength sizeRead = rtti_read_size_header(stream, compress, size);

			sizeRead += rtti_read(data.RendererSemantic, stream);
			sizeRead += rtti_read(data.Type, stream);
			sizeRead += rtti_read(data.Name, stream);
			sizeRead += rtti_read(data.GpuVariableNames, stream);
			sizeRead += rtti_read(data.DefaultValueIdx, stream);

			// There's more to read, meaning we're reading a newer version of the format
			// (In the first version, version field is missing, so we check this way).
			if(sizeRead < size)
			{
				uint32_t version = 0;
				rtti_read(version, stream);
				switch(version)
				{
				case 1:
					rtti_read(data.AttribIdx, stream);
					break;
				default:
					BS_LOG(Error, RTTI, "Unknown version. Unable to deserialize.");
					break;
				}
			}

			return size;
		}

		static BitLength GetSize(const SHADER_OBJECT_PARAM_DESC& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize = rtti_size(data.RendererSemantic) + rtti_size(data.Type) +
				rtti_size(data.Name) + rtti_size(data.GpuVariableNames) +
				rtti_size(data.DefaultValueIdx) + rtti_size(data.AttribIdx) + sizeof(uint32_t);

			rtti_add_header_size(dataSize, compress);
			return dataSize;
		}
	};

	template <>
	struct RTTIPlainType<SHADER_PARAM_BLOCK_DESC>
	{
		enum
		{
			id = TID_SHADER_PARAM_BLOCK_DESC
		};

		enum
		{
			hasDynamicSize = 1
		};

		static BitLength ToMemory(const SHADER_PARAM_BLOCK_DESC& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			return rtti_write_with_size_header(stream, data, compress, [&data, &stream]()
											   {
				BitLength size = 0;
				size += rtti_write(data.Shared, stream);
				size += rtti_write(data.Usage, stream);
				size += rtti_write(data.Name, stream);
				size += rtti_write(data.RendererSemantic, stream);

				return size; });
		}

		static BitLength FromMemory(SHADER_PARAM_BLOCK_DESC& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			rtti_read_size_header(stream, compress, size);

			rtti_read(data.Shared, stream);
			rtti_read(data.Usage, stream);
			rtti_read(data.Name, stream);
			rtti_read(data.RendererSemantic, stream);

			return size;
		}

		static BitLength GetSize(const SHADER_PARAM_BLOCK_DESC& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize = rtti_size(data.Shared) + rtti_size(data.Usage) +
				rtti_size(data.Name) + rtti_size(data.RendererSemantic);

			rtti_add_header_size(dataSize, compress);
			return dataSize;
		}
	};

	template <>
	struct RTTIPlainType<SHADER_PARAM_ATTRIBUTE>
	{
		enum
		{
			id = TID_SHADER_PARAM_ATTRIBUTE
		};

		enum
		{
			hasDynamicSize = 1
		};

		static BitLength ToMemory(const SHADER_PARAM_ATTRIBUTE& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			static constexpr u32 kVersion = 0;

			return rtti_write_with_size_header(stream, data, compress, [&data, &stream]()
											   {
				BitLength size = 0;
				size += rtti_write(kVersion, stream);
				size += rtti_write(data.Type, stream);
				size += rtti_write(data.Value, stream);
				size += rtti_write(data.NextParamIdx, stream);

				return size; });
		}

		static BitLength FromMemory(SHADER_PARAM_ATTRIBUTE& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			rtti_read_size_header(stream, compress, size);

			uint32_t version = 0;
			rtti_read(version, stream);

			switch(version)
			{
			case 0:
				rtti_read(data.Type, stream);
				rtti_read(data.Value, stream);
				rtti_read(data.NextParamIdx, stream);
				break;
			default:
				BS_LOG(Error, RTTI, "Unknown version. Unable to deserialize.");
				break;
			}

			return size;
		}

		static BitLength GetSize(const SHADER_PARAM_ATTRIBUTE& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize = rtti_size(data.Type) + rtti_size(data.Value) +
				rtti_size(data.NextParamIdx) + sizeof(uint32_t);

			rtti_add_header_size(dataSize, compress);
			return dataSize;
		}
	};

	template <>
	struct RTTIPlainType<ShaderVariationParamValue>
	{
		enum
		{
			id = TID_ShaderVariationParamValue
		};

		enum
		{
			hasDynamicSize = 1
		};

		static BitLength ToMemory(const ShaderVariationParamValue& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			static constexpr uint8_t kVersion = 0;

			return rtti_write_with_size_header(stream, data, compress, [&data, &stream]()
											   {
				BitLength size = 0;
				size += rtti_write(kVersion, stream);
				size += rtti_write(data.Name, stream);
				size += rtti_write(data.Value, stream);

				return size; });
		}

		static BitLength FromMemory(ShaderVariationParamValue& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			rtti_read_size_header(stream, compress, size);

			uint8_t version;
			rtti_read(version, stream);
			assert(version == 0);

			rtti_read(data.Name, stream);
			rtti_read(data.Value, stream);

			return size;
		}

		static BitLength GetSize(const ShaderVariationParamValue& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize = sizeof(uint8_t);
			dataSize += rtti_size(data.Name);
			dataSize += rtti_size(data.Value);

			rtti_add_header_size(dataSize, compress);
			return dataSize;
		}
	};

	template <>
	struct RTTIPlainType<ShaderVariationParamInfo>
	{
		enum
		{
			id = TID_ShaderVariationParamInfo
		};

		enum
		{
			hasDynamicSize = 1
		};

		static BitLength ToMemory(const ShaderVariationParamInfo& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			static constexpr uint8_t kVersion = 0;

			return rtti_write_with_size_header(stream, data, compress, [&data, &stream]()
											   {
				BitLength size = 0;
				size += rtti_write(kVersion, stream);
				size += rtti_write(data.Name, stream);
				size += rtti_write(data.Identifier, stream);
				size += rtti_write(data.IsInternal, stream);
				size += rtti_write(data.Values, stream);

				return size; });
		}

		static BitLength FromMemory(ShaderVariationParamInfo& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			rtti_read_size_header(stream, compress, size);

			uint8_t version;
			rtti_read(version, stream);
			assert(version == 0);

			rtti_read(data.Name, stream);
			rtti_read(data.Identifier, stream);
			rtti_read(data.IsInternal, stream);
			rtti_read(data.Values, stream);

			return size;
		}

		static BitLength GetSize(const ShaderVariationParamInfo& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize = sizeof(uint8_t);
			dataSize += rtti_size(data.Name);
			dataSize += rtti_size(data.Identifier);
			dataSize += rtti_size(data.IsInternal);
			dataSize += rtti_size(data.Values);

			rtti_add_header_size(dataSize, compress);
			return dataSize;
		}
	};

	class BS_CORE_EXPORT SubShaderRTTI : public RTTIType<SubShader, IReflectable, SubShaderRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN(Name, 0)
			BS_RTTI_MEMBER_REFLPTR(Shader, 1)
		BS_END_RTTI_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "SubShader";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_SubShader;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return bs_shared_ptr_new<SubShader>();
		}
	};

	class BS_CORE_EXPORT ShaderRTTI : public RTTIType<Shader, Resource, ShaderRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_REFLPTR_ARRAY_NAMED(mTechniques, mDesc.Techniques, 0)
			BS_RTTI_MEMBER_PLAIN(mName, 1)

			BS_RTTI_MEMBER_PLAIN_NAMED(mQueueSortType, mDesc.QueueSortType, 7)
			BS_RTTI_MEMBER_PLAIN_NAMED(mQueuePriority, mDesc.QueuePriority, 8)
			BS_RTTI_MEMBER_PLAIN_NAMED(mSeparablePasses, mDesc.SeparablePasses, 9)

			BS_RTTI_MEMBER_PLAIN_NAMED(mDataDefaultValues, mDesc.DataDefaultValues, 10)
			BS_RTTI_MEMBER_REFL_ARRAY_NAMED(mTextureDefaultValues, mDesc.TextureDefaultValues, 11)
			BS_RTTI_MEMBER_REFLPTR_ARRAY_NAMED(mSamplerDefaultValues, mDesc.SamplerDefaultValues, 12)

			BS_RTTI_MEMBER_PLAIN_NAMED(mFlags, mDesc.Flags, 13)
			BS_RTTI_MEMBER_REFL_ARRAY_NAMED(mSubShaders, mDesc.SubShaders, 14)

			BS_RTTI_MEMBER_PLAIN_ARRAY_NAMED(mParamAttributes, mDesc.ParamAttributes, 15)
			BS_RTTI_MEMBER_PLAIN_ARRAY_NAMED(mVariationParams, mDesc.VariationParams, 16)
		BS_END_RTTI_MEMBERS

		SHADER_DATA_PARAM_DESC& GetDataParam(Shader* obj, u32 idx)
		{
			auto iter = obj->mDesc.DataParams.begin();
			for(u32 i = 0; i < idx; i++) ++iter;

			return iter->second;
		}

		void SetDataParam(Shader* obj, u32 idx, SHADER_DATA_PARAM_DESC& val) { obj->mDesc.DataParams[val.Name] = val; }

		u32 GetDataParamsArraySize(Shader* obj) { return (u32)obj->mDesc.DataParams.size(); }

		void SetDataParamsArraySize(Shader* obj, u32 size) {} // Do nothing

		SHADER_OBJECT_PARAM_DESC& GetTextureParam(Shader* obj, u32 idx)
		{
			auto iter = obj->mDesc.TextureParams.begin();
			for(u32 i = 0; i < idx; i++) ++iter;

			return iter->second;
		}

		void SetTextureParam(Shader* obj, u32 idx, SHADER_OBJECT_PARAM_DESC& val) { obj->mDesc.TextureParams[val.Name] = val; }

		u32 GetTextureParamsArraySize(Shader* obj) { return (u32)obj->mDesc.TextureParams.size(); }

		void SetTextureParamsArraySize(Shader* obj, u32 size) {} // Do nothing

		SHADER_OBJECT_PARAM_DESC& GetSamplerParam(Shader* obj, u32 idx)
		{
			auto iter = obj->mDesc.SamplerParams.begin();
			for(u32 i = 0; i < idx; i++) ++iter;

			return iter->second;
		}

		void SetSamplerParam(Shader* obj, u32 idx, SHADER_OBJECT_PARAM_DESC& val) { obj->mDesc.SamplerParams[val.Name] = val; }

		u32 GetSamplerParamsArraySize(Shader* obj) { return (u32)obj->mDesc.SamplerParams.size(); }

		void SetSamplerParamsArraySize(Shader* obj, u32 size) {} // Do nothing

		SHADER_OBJECT_PARAM_DESC& GetBufferParam(Shader* obj, u32 idx)
		{
			auto iter = obj->mDesc.BufferParams.begin();
			for(u32 i = 0; i < idx; i++) ++iter;

			return iter->second;
		}

		void SetBufferParam(Shader* obj, u32 idx, SHADER_OBJECT_PARAM_DESC& val) { obj->mDesc.BufferParams[val.Name] = val; }

		u32 GetBufferParamsArraySize(Shader* obj) { return (u32)obj->mDesc.BufferParams.size(); }

		void SetBufferParamsArraySize(Shader* obj, u32 size) {} // Do nothing

		SHADER_PARAM_BLOCK_DESC& GetParamBlock(Shader* obj, u32 idx)
		{
			auto iter = obj->mDesc.ParamBlocks.begin();
			for(u32 i = 0; i < idx; i++) ++iter;

			return iter->second;
		}

		void SetParamBlock(Shader* obj, u32 idx, SHADER_PARAM_BLOCK_DESC& val) { obj->mDesc.ParamBlocks[val.Name] = val; }

		u32 GetParamBlocksArraySize(Shader* obj) { return (u32)obj->mDesc.ParamBlocks.size(); }

		void SetParamBlocksArraySize(Shader* obj, u32 size) {} // Do nothing

	public:
		ShaderRTTI()
		{
			AddPlainArrayField("mDataParams", 2, &ShaderRTTI::GetDataParam, &ShaderRTTI::GetDataParamsArraySize, &ShaderRTTI::SetDataParam, &ShaderRTTI::SetDataParamsArraySize);
			AddPlainArrayField("mTextureParams", 3, &ShaderRTTI::GetTextureParam, &ShaderRTTI::GetTextureParamsArraySize, &ShaderRTTI::SetTextureParam, &ShaderRTTI::SetTextureParamsArraySize);
			AddPlainArrayField("mSamplerParams", 4, &ShaderRTTI::GetSamplerParam, &ShaderRTTI::GetSamplerParamsArraySize, &ShaderRTTI::SetSamplerParam, &ShaderRTTI::SetSamplerParamsArraySize);
			AddPlainArrayField("mBufferParams", 5, &ShaderRTTI::GetBufferParam, &ShaderRTTI::GetBufferParamsArraySize, &ShaderRTTI::SetBufferParam, &ShaderRTTI::SetBufferParamsArraySize);
			AddPlainArrayField("mParamBlocks", 6, &ShaderRTTI::GetParamBlock, &ShaderRTTI::GetParamBlocksArraySize, &ShaderRTTI::SetParamBlock, &ShaderRTTI::SetParamBlocksArraySize);
		}

		void OnDeserializationEnded(IReflectable* obj, SerializationContext* context) override
		{
			Shader* shader = static_cast<Shader*>(obj);
			shader->Initialize();
		}

		const String& GetRttiName() override
		{
			static String name = "Shader";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_Shader;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return Shader::CreateEmpty();
		}
	};

	class BS_CORE_EXPORT ShaderMetaDataRTTI : public RTTIType<ShaderMetaData, ResourceMetaData, ShaderMetaDataRTTI>
	{
	private:
		Vector<String>& GetIncludes(ShaderMetaData* obj) { return obj->Includes; }

		void SetIncludes(ShaderMetaData* obj, Vector<String>& includes) { obj->Includes = includes; }

	public:
		ShaderMetaDataRTTI()
		{
			AddPlainField("includes", 0, &ShaderMetaDataRTTI::GetIncludes, &ShaderMetaDataRTTI::SetIncludes);
		}

		const String& GetRttiName() override
		{
			static String name = "ShaderMetaData";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_ShaderMetaData;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return bs_shared_ptr_new<ShaderMetaData>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
