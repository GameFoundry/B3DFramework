//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "RTTI/B3DStringIDRTTI.h"
#include "RTTI/B3DStringRTTI.h"
#include "RTTI/B3DStdRTTI.h"
#include "RTTI/B3DTArrayRTTI.h"
#include "RTTI/B3DFlagsRTTI.h"
#include "Material/B3DShader.h"
#include "Material/B3DMaterial.h"
#include "RTTI/B3DShaderCompilerRTTI.h"
#include "RTTI/B3DSamplerStateRTTI.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	template <>
	struct RTTIPlainType<ShaderDataParameterInformation>
	{
		enum
		{
			id = TID_ShaderDataParameterInformation
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
					B3D_LOG(Error, LogRTTI, "Unknown version. Unable to deserialize.");
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
			id = TID_ShaderObjectParameterInformation
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
			id = TID_ShaderParameterBlockInformation
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
			id = TID_ShaderParameterAttribute
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
				B3D_LOG(Error, LogRTTI, "Unknown version. Unable to deserialize.");
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
			id = TID_ShaderVariationParameterValue
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
			id = TID_ShaderVariationParameterInfo
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

	class B3D_EXPORT ShaderInformationBaseRTTI : public TRTTIType<ShaderInformationBase, IReflectable, ShaderInformationBaseRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(QueueSortType, 0)
			B3D_RTTI_MEMBER(QueuePriority, 1)
			B3D_RTTI_MEMBER(SeparablePasses, 2)
			B3D_RTTI_MEMBER(Flags, 3)

			B3D_RTTI_MEMBER_CONTAINER(DataParameters, 4)
			B3D_RTTI_MEMBER_CONTAINER(TextureParameters, 5)
			B3D_RTTI_MEMBER_CONTAINER(SamplerParameters, 6)
			B3D_RTTI_MEMBER_CONTAINER(BufferParameters, 7)
			B3D_RTTI_MEMBER_CONTAINER(DataParameterBlocks, 8)

			B3D_RTTI_MEMBER_CONTAINER(DataDefaultValues, 9)
			B3D_RTTI_MEMBER_CONTAINER(TextureDefaultValues, 10)
			B3D_RTTI_MEMBER_CONTAINER(SamplerDefaultValues, 11)

			B3D_RTTI_MEMBER_CONTAINER(ParamAttributes, 12)
			B3D_RTTI_MEMBER_CONTAINER(VariationParams, 13)

			B3D_RTTI_MEMBER(CompilerMetaData, 14)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "ShaderInformationBase";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_ShaderInformationBase;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<ShaderInformationBase>();
		}
	};

	class B3D_EXPORT ShaderInformationRTTI : public TRTTIType<ShaderInformation, ShaderInformationBase, ShaderInformationRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_CONTAINER(Techniques, 0)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "ShaderInformation";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_ShaderInformation;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<ShaderInformation>();
		}
	};

	class B3D_EXPORT ShaderInformationRenderProxyRTTI : public TRTTIType<render::ShaderInformation, ShaderInformationBase, ShaderInformationRenderProxyRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_CONTAINER(Techniques, 0)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "ShaderInformationRenderProxy";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_ShaderInformationRenderProxy;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<render::ShaderInformation>();
		}
	};

	class B3D_EXPORT ShaderRTTI : public TRTTIType<Shader, Resource, ShaderRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			//B3D_RTTI_MEMBER(mName, 0)
			B3D_RTTI_MEMBER(mInformation, 1)
		B3D_RTTI_END_MEMBERS

	public:
		void OnOperationEnded(Shader& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(operationType.IsSet(RTTIOperationType::WriteBit) && !operationType.IsSet(RTTIOperationType::PreExistingObjectBit))
			{
				object.Initialize();

				// Note: Important to call Initialize before the call below, because it will trigger a sync to render thread, and shaders render thread representation only gets created once Initialize() is called.
				for(const auto& technique : object.mInformation.Techniques)
					technique->SetOwner(std::static_pointer_cast<Shader>(object.GetShared()));
			}
		}


		const String& GetRttiName() override
		{
			static String name = "Shader";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_Shader;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return Shader::CreateEmpty();
		}
	};

	class B3D_EXPORT ShaderRenderProxyRTTI : public TRTTIType<render::Shader, IReflectable, ShaderRenderProxyRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(mName, 0)
			B3D_RTTI_MEMBER(mInformation, 1)
		B3D_RTTI_END_MEMBERS

	public:
		void OnOperationEnded(render::Shader& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(operationType.IsSet(RTTIOperationType::WriteBit) && !operationType.IsSet(RTTIOperationType::PreExistingObjectBit))
			{
				for(const auto& technique : object.mInformation.Techniques)
					technique->SetOwner(std::static_pointer_cast<render::Shader>(object.GetShared()));

				object.Initialize();
			}
		}

		const String& GetRttiName() override
		{
			static String name = "ShaderRenderProxy";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_ShaderRenderProxy;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return render::Shader::CreateEmpty();
		}
	};

	class B3D_EXPORT ShaderMetaDataRTTI : public TRTTIType<ShaderMetaData, ResourceMetaData, ShaderMetaDataRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(Includes, 0)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "ShaderMetaData";
			return name;
		}

		u32 GetRttiId() const override
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
} // namespace b3d
