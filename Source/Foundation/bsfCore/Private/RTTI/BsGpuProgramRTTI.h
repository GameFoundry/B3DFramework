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

	class BS_CORE_EXPORT GpuProgramBytecodeRTTI : public RTTIType<GpuProgramBytecode, IReflectable, GpuProgramBytecodeRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN(Instructions, 0)
			BS_RTTI_MEMBER_REFLPTR(ParamDesc, 1)
			BS_RTTI_MEMBER_PLAIN(VertexInput, 2)
			BS_RTTI_MEMBER_PLAIN(Messages, 3)
			BS_RTTI_MEMBER_PLAIN(CompilerId, 4)
			BS_RTTI_MEMBER_PLAIN(CompilerVersion, 5)
		BS_END_RTTI_MEMBERS

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

	class BS_CORE_EXPORT GpuParamDescRTTI : public RTTIType<GpuParamDesc, IReflectable, GpuParamDescRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN(ParamBlocks, 0)
			BS_RTTI_MEMBER_PLAIN(Params, 1)
			BS_RTTI_MEMBER_PLAIN(Samplers, 2)
			BS_RTTI_MEMBER_PLAIN(Textures, 3)
			BS_RTTI_MEMBER_PLAIN(LoadStoreTextures, 4)
			BS_RTTI_MEMBER_PLAIN(Buffers, 5)
		BS_END_RTTI_MEMBERS

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

	class BS_CORE_EXPORT GpuProgramRTTI : public RTTIType<GpuProgram, IReflectable, GpuProgramRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN(mType, 2)
			BS_RTTI_MEMBER_PLAIN(mNeedsAdjacencyInfo, 3)
			BS_RTTI_MEMBER_PLAIN(mEntryPoint, 4)
			BS_RTTI_MEMBER_PLAIN(mSource, 6)
			BS_RTTI_MEMBER_PLAIN(mLanguage, 7)
		BS_END_RTTI_MEMBERS

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
			return rtti_write_with_size_header(stream, data, compress, [&data, &stream]()
											   {
				BitLength size = 0;
				size += rtti_write(kVersion, stream);

				size += rtti_write(data.Name, stream);
				size += rtti_write(data.ElementSize, stream);
				size += rtti_write(data.ArraySize, stream);
				size += rtti_write(data.ArrayElementStride, stream);
				size += rtti_write(data.Type, stream);

				size += rtti_write(data.ParamBlockSlot, stream);
				size += rtti_write(data.ParamBlockSet, stream);
				size += rtti_write(data.GpuMemOffset, stream);
				size += rtti_write(data.CpuMemOffset, stream);

				return size; });
		}

		static BitLength FromMemory(GpuParamDataDesc& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			rtti_read_size_header(stream, compress, size);

			uint32_t version = 0;
			rtti_read(version, stream);
			assert(version == kVersion);

			rtti_read(data.Name, stream);
			rtti_read(data.ElementSize, stream);
			rtti_read(data.ArraySize, stream);
			rtti_read(data.ArrayElementStride, stream);
			rtti_read(data.Type, stream);

			rtti_read(data.ParamBlockSlot, stream);
			rtti_read(data.ParamBlockSet, stream);
			rtti_read(data.GpuMemOffset, stream);
			rtti_read(data.CpuMemOffset, stream);

			return size;
		}

		static BitLength GetSize(const GpuParamDataDesc& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize = rtti_size(kVersion) + rtti_size(data.Name) + rtti_size(data.ElementSize) +
				rtti_size(data.ArraySize) + rtti_size(data.ArrayElementStride) + rtti_size(data.Type) +
				rtti_size(data.ParamBlockSlot) + rtti_size(data.ParamBlockSet) +
				rtti_size(data.GpuMemOffset) + rtti_size(data.CpuMemOffset);

			rtti_add_header_size(dataSize, compress);
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
			return rtti_write_with_size_header(stream, data, compress, [&data, &stream]
											   {
				BitLength size = 0;
				size += rtti_write(kVersion, stream);
				size += rtti_write(data.Name, stream);
				size += rtti_write(data.Type, stream);
				size += rtti_write(data.Slot, stream);
				size += rtti_write(data.Set, stream);
				size += rtti_write(data.ElementType, stream);

				return size; });
		}

		static BitLength FromMemory(GpuParamObjectDesc& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			rtti_read_size_header(stream, compress, size);

			uint32_t version = 0;
			rtti_read(version, stream);

			rtti_read(data.Name, stream);
			rtti_read(data.Type, stream);
			rtti_read(data.Slot, stream);
			rtti_read(data.Set, stream);

			if(version > 1)
				rtti_read(data.ElementType, stream);

			return size;
		}

		static BitLength GetSize(const GpuParamObjectDesc& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize = rtti_size(kVersion) + rtti_size(data.Name) + rtti_size(data.Type) +
				rtti_size(data.Slot) + rtti_size(data.Set) + rtti_size(data.ElementType);

			rtti_add_header_size(dataSize, compress);
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
			return rtti_write_with_size_header(stream, data, compress, [&data, &stream]
											   {
				BitLength size = 0;
				size += rtti_write(kVersion, stream);
				size += rtti_write(data.Name, stream);
				size += rtti_write(data.Set, stream);
				size += rtti_write(data.Slot, stream);
				size += rtti_write(data.BlockSize, stream);
				size += rtti_write(data.IsShareable, stream);

				return size; });
		}

		static BitLength FromMemory(GpuParamBlockDesc& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			rtti_read_size_header(stream, compress, size);

			uint32_t version = 0;
			rtti_read(version, stream);
			assert(version == kVersion);

			rtti_read(data.Name, stream);
			rtti_read(data.Set, stream);
			rtti_read(data.Slot, stream);
			rtti_read(data.BlockSize, stream);
			rtti_read(data.IsShareable, stream);

			return size;
		}

		static BitLength GetSize(const GpuParamBlockDesc& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize = rtti_size(kVersion) + rtti_size(data.Name) + rtti_size(data.Set) +
				rtti_size(data.Slot) + rtti_size(data.BlockSize) + rtti_size(data.IsShareable);

			rtti_add_header_size(dataSize, compress);
			return dataSize;
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
