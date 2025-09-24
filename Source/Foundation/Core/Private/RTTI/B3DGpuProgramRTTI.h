//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DCorePrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "RenderAPI/B3DGpuDevice.h"
#include "RTTI/B3DStringRTTI.h"
#include "RTTI/B3DStdRTTI.h"
#include "RTTI/B3DDataBlobRTTI.h"
#include "RenderAPI/B3DGpuProgram.h"
#include "RenderAPI/B3DGpuProgramParameterDescription.h"
#include "B3DCoreApplication.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class B3D_CORE_EXPORT GpuProgramBytecodeRTTI : public TRTTIType<GpuProgramBytecode, IReflectable, GpuProgramBytecodeRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(Instructions, 0)
			B3D_RTTI_MEMBER(ParameterDescription, 1)
			B3D_RTTI_MEMBER(VertexInput, 2)
			B3D_RTTI_MEMBER(Messages, 3)
			B3D_RTTI_MEMBER(CompilerId, 4)
			B3D_RTTI_MEMBER(CompilerVersion, 5)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName()
		{
			static String name = "GpuProgramBytecode";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_GpuProgramBytecode;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return B3DMakeShared<GpuProgramBytecode>();
		}
	};

	class B3D_CORE_EXPORT GpuProgramParameterDescriptionRTTI : public TRTTIType<GpuProgramParameterDescription, IReflectable, GpuProgramParameterDescriptionRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(UniformBuffers, 0)
			B3D_RTTI_MEMBER(UniformBufferMembers, 1)
			B3D_RTTI_MEMBER(Samplers, 2)
			B3D_RTTI_MEMBER(SampledTextures, 3)
			B3D_RTTI_MEMBER(StorageTextures, 4)
			B3D_RTTI_MEMBER(Buffers, 5)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "GpuParameterDescription";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_GpuParameterDescription;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return B3DMakeShared<GpuProgramParameterDescription>();
		}
	};

	class B3D_CORE_EXPORT GpuProgramRTTI : public TRTTIType<GpuProgram, IReflectable, GpuProgramRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(mType, 2)
			B3D_RTTI_MEMBER(mNeedsAdjacencyInfo, 3)
			B3D_RTTI_MEMBER(mEntryPoint, 4)
			B3D_RTTI_MEMBER(mSource, 6)
			B3D_RTTI_MEMBER(mLanguage, 7)
			B3D_RTTI_MEMBER(mName, 8)
			B3D_RTTI_MEMBER(mBytecode, 9)
		B3D_RTTI_END_MEMBERS

	public:
		void OnOperationEnded(GpuProgram& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(operationType.IsSet(RTTIOperationType::WriteBit) && !operationType.IsSet(RTTIOperationType::PreExistingObjectBit))
				object.Initialize();
		}

		const String& GetRttiName() override
		{
			static String name = "GpuProgram";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_GpuProgram;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			const SPtr<GpuDevice>& gpuDevice = GetCoreApplication().GetPrimaryGpuDevice();
			if(!gpuDevice)
				return nullptr;

			return gpuDevice->CreateGpuProgram(GpuProgramCreateInformation(), true);
		}
	};

	class B3D_CORE_EXPORT GpuProgramCreateInformationRTTI : public TRTTIType<GpuProgramCreateInformation, IReflectable, GpuProgramCreateInformationRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(Name, 0)
			B3D_RTTI_MEMBER(Source, 1)
			B3D_RTTI_MEMBER(EntryPoint, 2)
			B3D_RTTI_MEMBER(Language, 3)
			B3D_RTTI_MEMBER(Type, 4)
			B3D_RTTI_MEMBER(RequiresAdjacency, 5)
			B3D_RTTI_MEMBER(Bytecode, 6)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "GpuProgramCreateInformation";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_GpuProgramCreateInformation;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<GpuProgramCreateInformation>();
		}
	};

	template <>
	struct RTTIPlainType<GpuDataParameterInformation>
	{
		enum
		{
			id = TID_GpuParamDataDesc
		};

		enum
		{
			hasDynamicSize = 1
		};

		static constexpr uint32_t kVersion = 1;

		static BitLength ToMemory(const GpuDataParameterInformation& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			return B3DRTTIWriteWithSizeHeader(stream, data, compress, [&data, &stream]()
											   {
				BitLength size = 0;
				size += B3DRTTIWrite(kVersion, stream);

				size += B3DRTTIWrite(data.Name, stream);
				size += B3DRTTIWrite(data.ElementSize, stream);
				size += B3DRTTIWrite(data.ArraySize, stream);
				size += B3DRTTIWrite(data.ArrayElementStride, stream);
				size += B3DRTTIWrite(data.Type, stream);

				size += B3DRTTIWrite(data.ParentUniformBufferSlot, stream);
				size += B3DRTTIWrite(data.ParentUniformBufferSet, stream);
				size += B3DRTTIWrite(data.GpuOffset, stream);
				size += B3DRTTIWrite(data.CpuOffset, stream);

				return size; });
		}

		static BitLength FromMemory(GpuDataParameterInformation& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			B3DRTTIReadSizeHeader(stream, compress, size);

			uint32_t version = 0;
			B3DRTTIRead(version, stream);
			B3D_ASSERT(version == kVersion);

			B3DRTTIRead(data.Name, stream);
			B3DRTTIRead(data.ElementSize, stream);
			B3DRTTIRead(data.ArraySize, stream);
			B3DRTTIRead(data.ArrayElementStride, stream);
			B3DRTTIRead(data.Type, stream);

			B3DRTTIRead(data.ParentUniformBufferSlot, stream);
			B3DRTTIRead(data.ParentUniformBufferSet, stream);
			B3DRTTIRead(data.GpuOffset, stream);
			B3DRTTIRead(data.CpuOffset, stream);

			return size;
		}

		static BitLength GetSize(const GpuDataParameterInformation& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize = B3DRTTISize(kVersion) + B3DRTTISize(data.Name) + B3DRTTISize(data.ElementSize) +
				B3DRTTISize(data.ArraySize) + B3DRTTISize(data.ArrayElementStride) + B3DRTTISize(data.Type) +
				B3DRTTISize(data.ParentUniformBufferSlot) + B3DRTTISize(data.ParentUniformBufferSet) +
				B3DRTTISize(data.GpuOffset) + B3DRTTISize(data.CpuOffset);

			B3DRTTIAddHeaderSize(dataSize, compress);
			return dataSize;
		}
	};

	template <>
	struct RTTIPlainType<GpuObjectParameterInformation>
	{
		enum
		{
			id = TID_GpuParamObjectDesc
		};

		enum
		{
			hasDynamicSize = 1
		};

		static constexpr uint32_t kVersion = 3;

		static BitLength ToMemory(const GpuObjectParameterInformation& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			return B3DRTTIWriteWithSizeHeader(stream, data, compress, [&data, &stream]
											   {
				BitLength size = 0;
				size += B3DRTTIWrite(kVersion, stream);
				size += B3DRTTIWrite(data.Name, stream);
				size += B3DRTTIWrite(data.Type, stream);
				size += B3DRTTIWrite(data.Slot, stream);
				size += B3DRTTIWrite(data.Set, stream);
				size += B3DRTTIWrite(data.ElementType, stream);
				size += B3DRTTIWrite(data.ArraySize, stream);

				return size; });
		}

		static BitLength FromMemory(GpuObjectParameterInformation& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			B3DRTTIReadSizeHeader(stream, compress, size);

			uint32_t version = 0;
			B3DRTTIRead(version, stream);

			B3DRTTIRead(data.Name, stream);
			B3DRTTIRead(data.Type, stream);
			B3DRTTIRead(data.Slot, stream);
			B3DRTTIRead(data.Set, stream);

			if(version > 1)
				B3DRTTIRead(data.ElementType, stream);

			if(version > 2)
				B3DRTTIRead(data.ArraySize, stream);

			return size;
		}

		static BitLength GetSize(const GpuObjectParameterInformation& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize = B3DRTTISize(kVersion) + B3DRTTISize(data.Name) + B3DRTTISize(data.Type) +
				B3DRTTISize(data.Slot) + B3DRTTISize(data.Set) + B3DRTTISize(data.ElementType) + B3DRTTISize(data.ArraySize);

			B3DRTTIAddHeaderSize(dataSize, compress);
			return dataSize;
		}
	};

	template <>
	struct RTTIPlainType<GpuDataParameterBlockInformation>
	{
		enum
		{
			id = TID_GpuParamBlockDesc
		};

		enum
		{
			hasDynamicSize = 1
		};

		static constexpr uint32_t kVersion = 1;

		static BitLength ToMemory(const GpuDataParameterBlockInformation& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			return B3DRTTIWriteWithSizeHeader(stream, data, compress, [&data, &stream]
											   {
				BitLength size = 0;
				size += B3DRTTIWrite(kVersion, stream);
				size += B3DRTTIWrite(data.Name, stream);
				size += B3DRTTIWrite(data.Set, stream);
				size += B3DRTTIWrite(data.Slot, stream);
				size += B3DRTTIWrite(data.BlockSize, stream);
				size += B3DRTTIWrite(data.IsShareable, stream);

				return size; });
		}

		static BitLength FromMemory(GpuDataParameterBlockInformation& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			B3DRTTIReadSizeHeader(stream, compress, size);

			uint32_t version = 0;
			B3DRTTIRead(version, stream);
			B3D_ASSERT(version == kVersion);

			B3DRTTIRead(data.Name, stream);
			B3DRTTIRead(data.Set, stream);
			B3DRTTIRead(data.Slot, stream);
			B3DRTTIRead(data.BlockSize, stream);
			B3DRTTIRead(data.IsShareable, stream);

			return size;
		}

		static BitLength GetSize(const GpuDataParameterBlockInformation& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize = B3DRTTISize(kVersion) + B3DRTTISize(data.Name) + B3DRTTISize(data.Set) +
				B3DRTTISize(data.Slot) + B3DRTTISize(data.BlockSize) + B3DRTTISize(data.IsShareable);

			B3DRTTIAddHeaderSize(dataSize, compress);
			return dataSize;
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
