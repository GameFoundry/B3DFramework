//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"

namespace bs
{
	/** @addtogroup RenderAPI
	 *  @{
	 */

	/** Describes a single GPU program data (for example int, float, Vector2) parameter. */
	struct GpuParamDataDesc
	{
		String name;
		UINT32 elementSize; /**< In multiples of 4 bytes. */
		UINT32 arraySize;
		UINT32 arrayElementStride; /**< In multiples of 4 bytes. */
		GpuParamDataType type;

		UINT32 paramBlockSlot;
		UINT32 paramBlockSet;
		UINT32 gpuMemOffset; /**< In multiples of 4 bytes, or index for parameters not in a buffer. */
		UINT32 cpuMemOffset; /**< In multiples of 4 bytes. */
	};

	/**	Describes a single GPU program object (for example texture, sampler state) parameter. */
	struct GpuParamObjectDesc
	{
		String name;
		GpuParamObjectType type;

		UINT32 slot; /** Slot within a set. Uniquely identifies bind location in the GPU pipeline, together with the set. */
		UINT32 set; /** Uniquely identifies the bind location in the GPU pipeline, together with the slot. */
	};

	/**	Describes a GPU program parameter block (collection of GPU program data parameters). */
	struct GpuParamBlockDesc
	{
		String name;
		UINT32 slot; /** Slot within a set. Uniquely identifies bind location in the GPU pipeline, together with the set. */
		UINT32 set; /** Uniquely identifies the bind location in the GPU pipeline, together with the slot. */
		UINT32 blockSize; /**< In multiples of 4 bytes. */
		bool isShareable; /** True for blocks that can be shared between different GPU pipeline stages. */
	};

	/** Contains all parameter information for a GPU program, including data and object parameters, plus parameter blocks. */
	struct BS_CORE_EXPORT GpuParamDesc : IReflectable
	{
		Map<String, GpuParamBlockDesc> paramBlocks;
		Map<String, GpuParamDataDesc> params;

		Map<String, GpuParamObjectDesc> samplers;
		Map<String, GpuParamObjectDesc> textures;
		Map<String, GpuParamObjectDesc> loadStoreTextures;
		Map<String, GpuParamObjectDesc> buffers;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class GpuParamDescRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** @} */

	/** @cond SPECIALIZATIONS */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	template<> struct RTTIPlainType<GpuParamDataDesc>
	{
		enum { id = TID_GpuParamDataDesc }; enum { hasDynamicSize = 1 };
		static constexpr UINT32 VERSION = 1;

		static void toMemory(const GpuParamDataDesc& data, char* memory)
		{
			UINT32 size = getDynamicSize(data);

			UINT32 curSize = sizeof(UINT32);
			memcpy(memory, &size, curSize);
			memory += curSize;

			memory = rttiWriteElem(VERSION, memory);

			memory = rttiWriteElem(data.name, memory);
			memory = rttiWriteElem(data.elementSize, memory);
			memory = rttiWriteElem(data.arraySize, memory);
			memory = rttiWriteElem(data.arrayElementStride, memory);
			memory = rttiWriteElem(data.type, memory);

			memory = rttiWriteElem(data.paramBlockSlot, memory);
			memory = rttiWriteElem(data.paramBlockSet, memory);
			memory = rttiWriteElem(data.gpuMemOffset, memory);
			rttiWriteElem(data.cpuMemOffset, memory);
		}

		static UINT32 fromMemory(GpuParamDataDesc& data, char* memory)
		{
			UINT32 size;
			memcpy(&size, memory, sizeof(UINT32));
			memory += sizeof(UINT32);

			UINT32 version = 0;
			memory = rttiReadElem(version, memory);
			assert(version == VERSION);

			memory = rttiReadElem(data.name, memory);
			memory = rttiReadElem(data.elementSize, memory);
			memory = rttiReadElem(data.arraySize, memory);
			memory = rttiReadElem(data.arrayElementStride, memory);
			memory = rttiReadElem(data.type, memory);

			memory = rttiReadElem(data.paramBlockSlot, memory);
			memory = rttiReadElem(data.paramBlockSet, memory);
			memory = rttiReadElem(data.gpuMemOffset, memory);
			rttiReadElem(data.cpuMemOffset, memory);

			return size;
		}

		static UINT32 getDynamicSize(const GpuParamDataDesc& data)
		{
			UINT32 dataSize = rttiGetElemSize(VERSION) + rttiGetElemSize(data.name) + rttiGetElemSize(data.elementSize) +
				rttiGetElemSize(data.arraySize) + rttiGetElemSize(data.arrayElementStride) + rttiGetElemSize(data.type) +
				rttiGetElemSize(data.paramBlockSlot) + rttiGetElemSize(data.paramBlockSet) +
				rttiGetElemSize(data.gpuMemOffset) + rttiGetElemSize(data.cpuMemOffset) + sizeof(UINT32);

			return dataSize;
		}
	};

	template<> struct RTTIPlainType<GpuParamObjectDesc>
	{
		enum { id = TID_GpuParamObjectDesc }; enum { hasDynamicSize = 1 };
		static constexpr UINT32 VERSION = 1;

		static void toMemory(const GpuParamObjectDesc& data, char* memory)
		{
			UINT32 size = getDynamicSize(data);

			UINT32 curSize = sizeof(UINT32);
			memcpy(memory, &size, curSize);
			memory += curSize;

			memory = rttiWriteElem(VERSION, memory);

			memory = rttiWriteElem(data.name, memory);
			memory = rttiWriteElem(data.type, memory);
			memory = rttiWriteElem(data.slot, memory);
			rttiWriteElem(data.set, memory);
		}

		static UINT32 fromMemory(GpuParamObjectDesc& data, char* memory)
		{
			UINT32 size;
			memcpy(&size, memory, sizeof(UINT32));
			memory += sizeof(UINT32);

			UINT32 version = 0;
			memory = rttiReadElem(version, memory);
			assert(version == VERSION);

			memory = rttiReadElem(data.name, memory);
			memory = rttiReadElem(data.type, memory);
			memory = rttiReadElem(data.slot, memory);
			rttiReadElem(data.set, memory);

			return size;
		}

		static UINT32 getDynamicSize(const GpuParamObjectDesc& data)
		{
			UINT32 dataSize = rttiGetElemSize(VERSION) + rttiGetElemSize(data.name) + rttiGetElemSize(data.type) +
				rttiGetElemSize(data.slot) + rttiGetElemSize(data.set) + sizeof(UINT32);

			return dataSize;
		}
	};

	template<> struct RTTIPlainType<GpuParamBlockDesc>
	{
		enum { id = TID_GpuParamBlockDesc }; enum { hasDynamicSize = 1 };
		static constexpr UINT32 VERSION = 1;

		static void toMemory(const GpuParamBlockDesc& data, char* memory)
		{
			UINT32 size = getDynamicSize(data);

			UINT32 curSize = sizeof(UINT32);
			memcpy(memory, &size, curSize);
			memory += curSize;

			memory = rttiWriteElem(VERSION, memory);

			memory = rttiWriteElem(data.name, memory);
			memory = rttiWriteElem(data.set, memory);
			memory = rttiWriteElem(data.slot, memory);
			memory = rttiWriteElem(data.blockSize, memory);
			rttiWriteElem(data.isShareable, memory);
		}

		static UINT32 fromMemory(GpuParamBlockDesc& data, char* memory)
		{
			UINT32 size;
			memcpy(&size, memory, sizeof(UINT32));
			memory += sizeof(UINT32);

			UINT32 version = 0;
			memory = rttiReadElem(version, memory);
			assert(version == VERSION);

			memory = rttiReadElem(data.name, memory);
			memory = rttiReadElem(data.set, memory);
			memory = rttiReadElem(data.slot, memory);
			memory = rttiReadElem(data.blockSize, memory);
			rttiReadElem(data.isShareable, memory);

			return size;
		}

		static UINT32 getDynamicSize(const GpuParamBlockDesc& data)
		{
			UINT32 dataSize = rttiGetElemSize(VERSION) + rttiGetElemSize(data.name) + rttiGetElemSize(data.set) +
				rttiGetElemSize(data.slot) + rttiGetElemSize(data.blockSize) + rttiGetElemSize(data.isShareable) +
				sizeof(UINT32);

			return dataSize;
		}
	};

	/** @} */
	/** @endcond */
}
