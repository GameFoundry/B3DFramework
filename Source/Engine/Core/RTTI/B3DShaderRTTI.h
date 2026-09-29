//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
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
	struct RTTIPlainType<ShaderDataParameterInformation> : RTTIPlainTypeHelper<ShaderDataParameterInformation, TID_ShaderDataParameterInformation, 0>
	{
		template<class Processor>
		static void RTTIEnumerateFields(ShaderDataParameterInformation& object, Processor& processor, u8 version)
		{
			processor(object.IsInternal);
			processor(object.Name);
			processor(object.GpuVariableName);
			processor(object.RendererSemantic);
			processor(object.DefaultValueIndex);
			processor(object.AttributeIndex);
			processor(object.Set);
			processor(object.Slot);
			processor(object.Type);
			processor(object.ArraySize);
			processor(object.ElementSize);
		}
	};

	template <>
	struct RTTIPlainType<ShaderObjectParameterInformation> : RTTIPlainTypeHelper<ShaderObjectParameterInformation, TID_ShaderObjectParameterInformation, 0>
	{
		template<class Processor>
		static void RTTIEnumerateFields(ShaderObjectParameterInformation& object, Processor& processor, u8 version)
		{
			processor(object.IsInternal);
			processor(object.Name);
			processor(object.GpuVariableName);
			processor(object.RendererSemantic);
			processor(object.DefaultValueIndex);
			processor(object.AttributeIndex);
			processor(object.Set);
			processor(object.Slot);
			processor(object.Type);
			processor(object.ArraySize);
			processor(object.GpuVariableNames);
		}
	};

	template <>
	struct RTTIPlainType<ShaderUniformBufferInformation> : RTTIPlainTypeHelper<ShaderUniformBufferInformation, TID_ShaderUniformBufferInformation, 0>
	{
		template<class Processor>
		static void RTTIEnumerateFields(ShaderUniformBufferInformation& object, Processor& processor, u8 version)
		{
			processor(object.IsInternal);
			processor(object.Name);
			processor(object.Set);
			processor(object.Slot);
			processor(object.UsesDynamicOffset);
			processor(object.Shared);
			processor(object.RendererSemantic);
			processor(object.Flags);
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

	class B3D_EXPORT ShaderParameterDescriptionRTTI : public TRTTIType<ShaderParameterDescription, IReflectable, ShaderParameterDescriptionRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(mDataParameters, 0)
			B3D_RTTI_MEMBER(mTextureParameters, 1)
			B3D_RTTI_MEMBER(mBufferParameters, 2)
			B3D_RTTI_MEMBER(mSamplerParameters, 3)
			B3D_RTTI_MEMBER(mUniformBuffers, 4)
			B3D_RTTI_MEMBER(mDataDefaultValues, 5)
			B3D_RTTI_MEMBER(mSamplerDefaultValues, 6)
			B3D_RTTI_MEMBER(mTextureDefaultValues, 7)
			B3D_RTTI_MEMBER(mParameterAttributes, 8)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "ShaderParameterDescription";
			return name;
		}

		u32 GetRttiId() const override { return TID_ShaderParameterDescription; }
		TShared<IReflectable> NewRttiObject() override { return B3DMakeShared<ShaderParameterDescription>(); }
	};

	class B3D_EXPORT ShaderDescriptionRTTI : public TRTTIType<ShaderDescription, IReflectable, ShaderDescriptionRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(Parameters, 0)
			B3D_RTTI_MEMBER(QueueSortType, 1)
			B3D_RTTI_MEMBER(QueuePriority, 2)
			B3D_RTTI_MEMBER(SeparablePasses, 3)
			B3D_RTTI_MEMBER(Flags, 4)
			B3D_RTTI_MEMBER(VariationParameters, 5)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "ShaderDescription";
			return name;
		}

		u32 GetRttiId() const override { return TID_ShaderDescription; }
		TShared<IReflectable> NewRttiObject() override { return B3DMakeShared<ShaderDescription>(); }
	};

	class B3D_EXPORT ShaderCreateInformationRTTI : public TRTTIType<ShaderCreateInformation, IReflectable, ShaderCreateInformationRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(Description, 0)
			B3D_RTTI_MEMBER(CompilerMetaData, 1)
			B3D_RTTI_MEMBER_CONTAINER(Variations, 2)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "ShaderCreateInformation";
			return name;
		}

		u32 GetRttiId() const override { return TID_ShaderCreateInformation; }
		TShared<IReflectable> NewRttiObject() override { return B3DMakeShared<ShaderCreateInformation>(); }
	};

	class B3D_EXPORT ShaderCreateInformationRenderProxyRTTI : public TRTTIType<render::ShaderCreateInformation, IReflectable, ShaderCreateInformationRenderProxyRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(Description, 0)
			B3D_RTTI_MEMBER(CompilerMetaData, 1)
			B3D_RTTI_MEMBER_CONTAINER(Variations, 2)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "ShaderCreateInformationRenderProxy";
			return name;
		}

		u32 GetRttiId() const override { return TID_ShaderCreateInformationRenderProxy; }
		TShared<IReflectable> NewRttiObject() override { return B3DMakeShared<render::ShaderCreateInformation>(); }
	};

	class B3D_EXPORT PrecompiledShaderDataRTTI : public TRTTIType<PrecompiledShaderData, IReflectable, PrecompiledShaderDataRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(Name, 0)
			B3D_RTTI_MEMBER(Description, 1)
			B3D_RTTI_MEMBER(CompilerMetaData, 2)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "PrecompiledShaderData";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_PrecompiledShaderData;
		}

		TShared<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<PrecompiledShaderData>();
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
				for(const auto& variation : object.mInformation.Variations)
					variation->SetOwner(std::static_pointer_cast<Shader>(object.GetShared()));
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

		TShared<IReflectable> NewRttiObject() override
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
				for(const auto& variation : object.mInformation.Variations)
					variation->SetOwner(std::static_pointer_cast<render::Shader>(object.GetShared()));

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

		TShared<IReflectable> NewRttiObject() override
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

		TShared<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<ShaderMetaData>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
