//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "RTTI/BsStringIDRTTI.h"
#include "RTTI/BsStringRTTI.h"
#include "RTTI/BsStdRTTI.h"
#include "RTTI/BsTArrayRTTI.h"
#include "RTTI/BsFlagsRTTI.h"
#include "Material/BsShader.h"
#include "Material/BsMaterial.h"
#include "Private/RTTI/BsShaderCompilerRTTI.h"
#include "Private/RTTI/BsSamplerStateRTTI.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	template <>
	struct RTTIPlainType<ShaderDataParameterInformation>
	{
		enum
		{
			id = TID_SHADER_DATA_PARAM_DESC
		};

		enum
		{
			hasDynamicSize = 1
		};

		static BitLength ToMemory(const ShaderDataParameterInformation& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			static constexpr u32 kVersion = 1;

			return B3DRTTIWriteWithSizeHeader(stream, data, compress, [&data, &stream]()
											   {
				BitLength size = 0;

				size += B3DRTTIWrite(data.ArraySize, stream);
				size += B3DRTTIWrite(data.RendererSemantic, stream);
				size += B3DRTTIWrite(data.Type, stream);
				size += B3DRTTIWrite(data.Name, stream);
				size += B3DRTTIWrite(data.GpuVariableName, stream);
				size += B3DRTTIWrite(data.ElementSize, stream);
				size += B3DRTTIWrite(data.DefaultValueIndex, stream);
				size += B3DRTTIWrite(kVersion, stream);
				size += B3DRTTIWrite(data.AttributeIndex, stream);

				return size; });
		}

		static BitLength FromMemory(ShaderDataParameterInformation& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			BitLength sizeRead = B3DRTTIReadSizeHeader(stream, compress, size);

			sizeRead += B3DRTTIRead(data.ArraySize, stream);
			sizeRead += B3DRTTIRead(data.RendererSemantic, stream);
			sizeRead += B3DRTTIRead(data.Type, stream);
			sizeRead += B3DRTTIRead(data.Name, stream);
			sizeRead += B3DRTTIRead(data.GpuVariableName, stream);
			sizeRead += B3DRTTIRead(data.ElementSize, stream);
			sizeRead += B3DRTTIRead(data.DefaultValueIndex, stream);

			// There's more to read, meaning we're reading a newer version of the format
			// (In the first version, version field is missing, so we check this way).
			if(sizeRead < size)
			{
				uint32_t version = 0;
				B3DRTTIRead(version, stream);
				switch(version)
				{
				case 1:
					B3DRTTIRead(data.AttributeIndex, stream);
					break;
				default:
					B3D_LOG(Error, RTTI, "Unknown version. Unable to deserialize.");
					break;
				}
			}

			return size;
		}

		static BitLength GetSize(const ShaderDataParameterInformation& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize = B3DRTTISize(data.ArraySize) + B3DRTTISize(data.RendererSemantic) + B3DRTTISize(data.Type) +
				B3DRTTISize(data.Name) + B3DRTTISize(data.GpuVariableName) + B3DRTTISize(data.ElementSize) +
				B3DRTTISize(data.DefaultValueIndex) + B3DRTTISize(data.AttributeIndex) + sizeof(uint32_t);

			B3DRTTIAddHeaderSize(dataSize, compress);
			return dataSize;
		}
	};

	template <>
	struct RTTIPlainType<ShaderObjectParameterInformation>
	{
		enum
		{
			id = TID_SHADER_OBJECT_PARAM_DESC
		};

		enum
		{
			hasDynamicSize = 1
		};

		static BitLength ToMemory(const ShaderObjectParameterInformation& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			static constexpr uint32_t kVersion = 2;

			return B3DRTTIWriteWithSizeHeader(stream, data, compress, [&data, &stream]()
											   {
				BitLength size = 0;
				size += B3DRTTIWrite(data.RendererSemantic, stream);
				size += B3DRTTIWrite(data.Type, stream);
				size += B3DRTTIWrite(data.Name, stream);
				size += B3DRTTIWrite(data.GpuVariableNames, stream);
				size += B3DRTTIWrite(data.DefaultValueIndex, stream);
				size += B3DRTTIWrite(kVersion, stream);
				size += B3DRTTIWrite(data.AttributeIndex, stream);
				size += B3DRTTIWrite(data.ArraySize, stream);

				return size; });
		}

		static BitLength FromMemory(ShaderObjectParameterInformation& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			BitLength sizeRead = B3DRTTIReadSizeHeader(stream, compress, size);

			sizeRead += B3DRTTIRead(data.RendererSemantic, stream);
			sizeRead += B3DRTTIRead(data.Type, stream);
			sizeRead += B3DRTTIRead(data.Name, stream);
			sizeRead += B3DRTTIRead(data.GpuVariableNames, stream);
			sizeRead += B3DRTTIRead(data.DefaultValueIndex, stream);

			// There's more to read, meaning we're reading a newer version of the format
			// (In the first version, version field is missing, so we check this way).
			if(sizeRead < size)
			{
				uint32_t version = 0;
				B3DRTTIRead(version, stream);

				if(version >= 1)
					B3DRTTIRead(data.AttributeIndex, stream);

				if(version >= 2)
					B3DRTTIRead(data.ArraySize, stream);
			}

			return size;
		}

		static BitLength GetSize(const ShaderObjectParameterInformation& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize = B3DRTTISize(data.RendererSemantic) + B3DRTTISize(data.Type) +
				B3DRTTISize(data.Name) + B3DRTTISize(data.GpuVariableNames) +
				B3DRTTISize(data.DefaultValueIndex) + B3DRTTISize(data.AttributeIndex) + B3DRTTISize(data.ArraySize) + sizeof(uint32_t);

			B3DRTTIAddHeaderSize(dataSize, compress);
			return dataSize;
		}
	};

	template <>
	struct RTTIPlainType<ShaderParameterBlockInformation>
	{
		enum
		{
			id = TID_SHADER_PARAM_BLOCK_DESC
		};

		enum
		{
			hasDynamicSize = 1
		};

		static BitLength ToMemory(const ShaderParameterBlockInformation& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			return B3DRTTIWriteWithSizeHeader(stream, data, compress, [&data, &stream]()
											   {
				BitLength size = 0;
				size += B3DRTTIWrite(data.Shared, stream);
				size += B3DRTTIWrite(data.Flags, stream);
				size += B3DRTTIWrite(data.Name, stream);
				size += B3DRTTIWrite(data.RendererSemantic, stream);

				return size; });
		}

		static BitLength FromMemory(ShaderParameterBlockInformation& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			B3DRTTIReadSizeHeader(stream, compress, size);

			B3DRTTIRead(data.Shared, stream);
			B3DRTTIRead(data.Flags, stream);
			B3DRTTIRead(data.Name, stream);
			B3DRTTIRead(data.RendererSemantic, stream);

			return size;
		}

		static BitLength GetSize(const ShaderParameterBlockInformation& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize = B3DRTTISize(data.Shared) + B3DRTTISize(data.Flags) +
				B3DRTTISize(data.Name) + B3DRTTISize(data.RendererSemantic);

			B3DRTTIAddHeaderSize(dataSize, compress);
			return dataSize;
		}
	};

	template <>
	struct RTTIPlainType<ShaderParameterAttribute>
	{
		enum
		{
			id = TID_SHADER_PARAM_ATTRIBUTE
		};

		enum
		{
			hasDynamicSize = 1
		};

		static BitLength ToMemory(const ShaderParameterAttribute& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			static constexpr u32 kVersion = 0;

			return B3DRTTIWriteWithSizeHeader(stream, data, compress, [&data, &stream]()
											   {
				BitLength size = 0;
				size += B3DRTTIWrite(kVersion, stream);
				size += B3DRTTIWrite(data.Type, stream);
				size += B3DRTTIWrite(data.Value, stream);
				size += B3DRTTIWrite(data.NextParameterIndex, stream);

				return size; });
		}

		static BitLength FromMemory(ShaderParameterAttribute& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			B3DRTTIReadSizeHeader(stream, compress, size);

			uint32_t version = 0;
			B3DRTTIRead(version, stream);

			switch(version)
			{
			case 0:
				B3DRTTIRead(data.Type, stream);
				B3DRTTIRead(data.Value, stream);
				B3DRTTIRead(data.NextParameterIndex, stream);
				break;
			default:
				B3D_LOG(Error, RTTI, "Unknown version. Unable to deserialize.");
				break;
			}

			return size;
		}

		static BitLength GetSize(const ShaderParameterAttribute& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize = B3DRTTISize(data.Type) + B3DRTTISize(data.Value) +
				B3DRTTISize(data.NextParameterIndex) + sizeof(uint32_t);

			B3DRTTIAddHeaderSize(dataSize, compress);
			return dataSize;
		}
	};

	template <>
	struct RTTIPlainType<ShaderVariationParameterValue>
	{
		enum
		{
			id = TID_ShaderVariationParamValue
		};

		enum
		{
			hasDynamicSize = 1
		};

		static BitLength ToMemory(const ShaderVariationParameterValue& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			static constexpr uint8_t kVersion = 0;

			return B3DRTTIWriteWithSizeHeader(stream, data, compress, [&data, &stream]()
											   {
				BitLength size = 0;
				size += B3DRTTIWrite(kVersion, stream);
				size += B3DRTTIWrite(data.Name, stream);
				size += B3DRTTIWrite(data.Value, stream);

				return size; });
		}

		static BitLength FromMemory(ShaderVariationParameterValue& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			B3DRTTIReadSizeHeader(stream, compress, size);

			uint8_t version;
			B3DRTTIRead(version, stream);
			B3D_ASSERT(version == 0);

			B3DRTTIRead(data.Name, stream);
			B3DRTTIRead(data.Value, stream);

			return size;
		}

		static BitLength GetSize(const ShaderVariationParameterValue& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize = sizeof(uint8_t);
			dataSize += B3DRTTISize(data.Name);
			dataSize += B3DRTTISize(data.Value);

			B3DRTTIAddHeaderSize(dataSize, compress);
			return dataSize;
		}
	};

	template <>
	struct RTTIPlainType<ShaderVariationParameterInformation>
	{
		enum
		{
			id = TID_ShaderVariationParamInfo
		};

		enum
		{
			hasDynamicSize = 1
		};

		static BitLength ToMemory(const ShaderVariationParameterInformation& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			static constexpr uint8_t kVersion = 0;

			return B3DRTTIWriteWithSizeHeader(stream, data, compress, [&data, &stream]()
											   {
				BitLength size = 0;
				size += B3DRTTIWrite(kVersion, stream);
				size += B3DRTTIWrite(data.Name, stream);
				size += B3DRTTIWrite(data.Identifier, stream);
				size += B3DRTTIWrite(data.IsInternal, stream);
				size += B3DRTTIWrite(data.Values, stream);

				return size; });
		}

		static BitLength FromMemory(ShaderVariationParameterInformation& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			B3DRTTIReadSizeHeader(stream, compress, size);

			uint8_t version;
			B3DRTTIRead(version, stream);
			B3D_ASSERT(version == 0);

			B3DRTTIRead(data.Name, stream);
			B3DRTTIRead(data.Identifier, stream);
			B3DRTTIRead(data.IsInternal, stream);
			B3DRTTIRead(data.Values, stream);

			return size;
		}

		static BitLength GetSize(const ShaderVariationParameterInformation& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize = sizeof(uint8_t);
			dataSize += B3DRTTISize(data.Name);
			dataSize += B3DRTTISize(data.Identifier);
			dataSize += B3DRTTISize(data.IsInternal);
			dataSize += B3DRTTISize(data.Values);

			B3DRTTIAddHeaderSize(dataSize, compress);
			return dataSize;
		}
	};

	class B3D_CORE_EXPORT ShaderInformationBaseRTTI : public RTTIType<ShaderInformationBase, IReflectable, ShaderInformationBaseRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_PLAIN(QueueSortType, 0)
			B3D_RTTI_MEMBER_PLAIN(QueuePriority, 1)
			B3D_RTTI_MEMBER_PLAIN(SeparablePasses, 2)
			B3D_RTTI_MEMBER_PLAIN(Flags, 3)

			B3D_RTTI_MEMBER_PLAIN_MAP(DataParameters, 4, Name)
			B3D_RTTI_MEMBER_PLAIN_MAP(TextureParameters, 5, Name)
			B3D_RTTI_MEMBER_PLAIN_MAP(SamplerParameters, 6, Name)
			B3D_RTTI_MEMBER_PLAIN_MAP(BufferParameters, 7, Name)
			B3D_RTTI_MEMBER_PLAIN_MAP(DataParameterBlocks, 8, Name)

			B3D_RTTI_MEMBER_PLAIN_ARRAY(DataDefaultValues, 9)
			B3D_RTTI_MEMBER_PLAIN_ARRAY(TextureDefaultValues, 10)
			B3D_RTTI_MEMBER_PLAIN_ARRAY(SamplerDefaultValues, 11)

			B3D_RTTI_MEMBER_PLAIN_ARRAY(ParamAttributes, 12)
			B3D_RTTI_MEMBER_PLAIN_ARRAY(VariationParams, 13)

			B3D_RTTI_MEMBER_REFLPTR(CompilerMetaData, 14)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "ShaderInformationBase";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_ShaderInformationBase;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<ShaderInformationBase>();
		}
	};

	class B3D_CORE_EXPORT ShaderInformationRTTI : public RTTIType<ShaderInformation, ShaderInformationBase, ShaderInformationRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_REFLPTR_ARRAY(Techniques, 0)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "ShaderInformation";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_ShaderInformation;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<ShaderInformation>();
		}
	};

	class B3D_CORE_EXPORT CoreShaderInformationRTTI : public RTTIType<ct::ShaderInformation, ShaderInformationBase, CoreShaderInformationRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_REFLPTR_ARRAY(Techniques, 0)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "CoreShaderInformation";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_CoreShaderInformation;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<ct::ShaderInformation>();
		}
	};

	class B3D_CORE_EXPORT ShaderRTTI : public RTTIType<Shader, Resource, ShaderRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_PLAIN(mName, 0)
			B3D_RTTI_MEMBER_REFL(mInformation, 1)
		B3D_RTTI_END_MEMBERS

	public:
		void OnDeserializationEnded(IReflectable* obj, SerializationContext* context) override
		{
			Shader* shader = static_cast<Shader*>(obj);
			shader->Initialize();

			// Note: Important to call Initialize before the call below, because it will trigger a sync to render thread, and shaders render thread representation only gets created once Initialize() is called.
			for(const auto& technique : shader->mInformation.Techniques)
				technique->SetOwner(std::static_pointer_cast<Shader>(shader->GetShared()));
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

	class B3D_CORE_EXPORT CoreShaderRTTI : public RTTIType<ct::Shader, IReflectable, CoreShaderRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_PLAIN(mName, 0)
			B3D_RTTI_MEMBER_REFL(mInformation, 1)
		B3D_RTTI_END_MEMBERS

	public:
		void OnDeserializationEnded(IReflectable* obj, SerializationContext* context) override
		{
			ct::Shader* const shader = static_cast<ct::Shader*>(obj);

			for(const auto& technique : shader->mInformation.Techniques)
				technique->SetOwner(std::static_pointer_cast<ct::Shader>(shader->GetShared()));

			shader->Initialize();
		}

		const String& GetRttiName() override
		{
			static String name = "CoreShader";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_CoreShader;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return ct::Shader::CreateEmpty();
		}
	};

	class B3D_CORE_EXPORT ShaderMetaDataRTTI : public RTTIType<ShaderMetaData, ResourceMetaData, ShaderMetaDataRTTI>
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
			return B3DMakeShared<ShaderMetaData>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
