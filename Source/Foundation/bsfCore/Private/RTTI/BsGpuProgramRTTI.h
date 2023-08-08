//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "RenderAPI/BsGpuDevice.h"
#include "RTTI/BsStringRTTI.h"
#include "RTTI/BsStdRTTI.h"
#include "RTTI/BsDataBlobRTTI.h"
#include "RenderAPI/BsGpuProgram.h"
#include "RenderAPI/BsGpuProgramParameterDescription.h"
#include "BsCoreApplication.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class B3D_CORE_EXPORT GpuProgramBytecodeRTTI : public RTTIType<GpuProgramBytecode, IReflectable, GpuProgramBytecodeRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_PLAIN(Instructions, 0)
			B3D_RTTI_MEMBER_REFLPTR(ParameterDescription, 1)
			B3D_RTTI_MEMBER_PLAIN(VertexInput, 2)
			B3D_RTTI_MEMBER_PLAIN(Messages, 3)
			B3D_RTTI_MEMBER_PLAIN(CompilerId, 4)
			B3D_RTTI_MEMBER_PLAIN(CompilerVersion, 5)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName()
		{
			static String name = "GpuProgramBytecode";
			return name;
		}

		u32 GetRttiId()
		{
			return TID_GpuProgramBytecode;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return B3DMakeShared<GpuProgramBytecode>();
		}
	};

	class B3D_CORE_EXPORT GpuProgramParameterDescriptionRTTI : public RTTIType<GpuProgramParameterDescription, IReflectable, GpuProgramParameterDescriptionRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_PLAIN(UniformBuffers, 0)
			B3D_RTTI_MEMBER_PLAIN(UniformBufferMembers, 1)
			B3D_RTTI_MEMBER_PLAIN(Samplers, 2)
			B3D_RTTI_MEMBER_PLAIN(SampledTextures, 3)
			B3D_RTTI_MEMBER_PLAIN(StorageTextures, 4)
			B3D_RTTI_MEMBER_PLAIN(Buffers, 5)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "GpuParameterDescription";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_GpuParameterDescription;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return B3DMakeShared<GpuProgramParameterDescription>();
		}
	};

	class B3D_CORE_EXPORT GpuProgramRTTI : public RTTIType<GpuProgram, IReflectable, GpuProgramRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_PLAIN(mType, 2)
			B3D_RTTI_MEMBER_PLAIN(mNeedsAdjacencyInfo, 3)
			B3D_RTTI_MEMBER_PLAIN(mEntryPoint, 4)
			B3D_RTTI_MEMBER_PLAIN(mSource, 6)
			B3D_RTTI_MEMBER_PLAIN(mLanguage, 7)
			B3D_RTTI_MEMBER_PLAIN(mName, 8)
			B3D_RTTI_MEMBER_REFLPTR(mBytecode, 9)
		B3D_RTTI_END_MEMBERS

	public:
		void OnDeserializationEnded(IReflectable* obj, SerializationContext* context) override
		{
			GpuProgram* gpuProgram = static_cast<GpuProgram*>(obj);
			gpuProgram->Initialize();
		}

		const String& GetRttiName() override
		{
			static String name = "GpuProgram";
			return name;
		}

		u32 GetRttiId() override
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

	class B3D_CORE_EXPORT GpuProgramCreateInformationRTTI : public RTTIType<GpuProgramCreateInformation, IReflectable, GpuProgramCreateInformationRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_PLAIN(Name, 0)
			B3D_RTTI_MEMBER_PLAIN(Source, 1)
			B3D_RTTI_MEMBER_PLAIN(EntryPoint, 2)
			B3D_RTTI_MEMBER_PLAIN(Language, 3)
			B3D_RTTI_MEMBER_PLAIN(Type, 4)
			B3D_RTTI_MEMBER_PLAIN(RequiresAdjacency, 5)
			B3D_RTTI_MEMBER_REFLPTR(Bytecode, 6)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "GpuProgramCreateInformation";
			return name;
		}

		u32 GetRttiId() override
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
} // namespace bs
