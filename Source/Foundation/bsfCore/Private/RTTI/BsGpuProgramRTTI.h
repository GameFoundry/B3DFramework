//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "RTTI/BsStringRTTI.h"
#include "RTTI/BsStdRTTI.h"
#include "RTTI/BsDataBlobRTTI.h"
#include "RenderAPI/BsGpuProgram.h"
#include "RenderAPI/BsGpuParamDesc.h"
#include "Managers/BsGpuProgramManager.h"

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
			B3D_RTTI_MEMBER_REFLPTR(ParamDesc, 1)
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

	class B3D_CORE_EXPORT GpuParamDescRTTI : public RTTIType<GpuParamDesc, IReflectable, GpuParamDescRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_PLAIN(ParamBlocks, 0)
			B3D_RTTI_MEMBER_PLAIN(Params, 1)
			B3D_RTTI_MEMBER_PLAIN(Samplers, 2)
			B3D_RTTI_MEMBER_PLAIN(Textures, 3)
			B3D_RTTI_MEMBER_PLAIN(LoadStoreTextures, 4)
			B3D_RTTI_MEMBER_PLAIN(Buffers, 5)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "GpuParamDesc";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_GpuParamDesc;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return B3DMakeShared<GpuParamDesc>();
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
		B3D_RTTI_END_MEMBERS

	public:
		void OnSerializationStarted(IReflectable* obj, SerializationContext* context) override
		{
			// Need to ensure the core thread object is initialized
			GpuProgram* gpuProgram = static_cast<GpuProgram*>(obj);
			gpuProgram->BlockUntilCoreInitialized();
		}

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

		SPtr<IReflectable> NewRttiObject()
		{
			return GpuProgramManager::Instance().CreateEmpty("", GPT_VERTEX_PROGRAM); // Params don't matter, they'll get overwritten
		}
	};

	template <>
	struct RTTIPlainType<GpuParamDataDesc>
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

		static BitLength ToMemory(const GpuParamDataDesc& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
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

				size += B3DRTTIWrite(data.ParamBlockSlot, stream);
				size += B3DRTTIWrite(data.ParamBlockSet, stream);
				size += B3DRTTIWrite(data.GpuMemOffset, stream);
				size += B3DRTTIWrite(data.CpuMemOffset, stream);

				return size; });
		}

		static BitLength FromMemory(GpuParamDataDesc& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
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

			B3DRTTIRead(data.ParamBlockSlot, stream);
			B3DRTTIRead(data.ParamBlockSet, stream);
			B3DRTTIRead(data.GpuMemOffset, stream);
			B3DRTTIRead(data.CpuMemOffset, stream);

			return size;
		}

		static BitLength GetSize(const GpuParamDataDesc& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize = B3DRTTISize(kVersion) + B3DRTTISize(data.Name) + B3DRTTISize(data.ElementSize) +
				B3DRTTISize(data.ArraySize) + B3DRTTISize(data.ArrayElementStride) + B3DRTTISize(data.Type) +
				B3DRTTISize(data.ParamBlockSlot) + B3DRTTISize(data.ParamBlockSet) +
				B3DRTTISize(data.GpuMemOffset) + B3DRTTISize(data.CpuMemOffset);

			B3DRTTIAddHeaderSize(dataSize, compress);
			return dataSize;
		}
	};

	template <>
	struct RTTIPlainType<GpuParamObjectDesc>
	{
		enum
		{
			id = TID_GpuParamObjectDesc
		};

		enum
		{
			hasDynamicSize = 1
		};

		static constexpr uint32_t kVersion = 2;

		static BitLength ToMemory(const GpuParamObjectDesc& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
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

				return size; });
		}

		static BitLength FromMemory(GpuParamObjectDesc& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
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

			return size;
		}

		static BitLength GetSize(const GpuParamObjectDesc& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize = B3DRTTISize(kVersion) + B3DRTTISize(data.Name) + B3DRTTISize(data.Type) +
				B3DRTTISize(data.Slot) + B3DRTTISize(data.Set) + B3DRTTISize(data.ElementType);

			B3DRTTIAddHeaderSize(dataSize, compress);
			return dataSize;
		}
	};

	template <>
	struct RTTIPlainType<GpuParamBlockDesc>
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

		static BitLength ToMemory(const GpuParamBlockDesc& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
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

		static BitLength FromMemory(GpuParamBlockDesc& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
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

		static BitLength GetSize(const GpuParamBlockDesc& data, const RTTIFieldInfo& fieldInfo, bool compress)
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
