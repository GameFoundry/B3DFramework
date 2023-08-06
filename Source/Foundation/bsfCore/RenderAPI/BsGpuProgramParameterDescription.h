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
	struct GpuDataParameterInformation
	{
		String Name;
		u32 ElementSize; /**< In multiples of 4 bytes. */
		u32 ArraySize;
		u32 ArrayElementStride; /**< In multiples of 4 bytes. */
		GpuDataParameterType Type;

		u32 ParamBlockSlot;
		u32 ParamBlockSet;
		u32 GpuOffset; /**< In multiples of 4 bytes, or index for parameters not in a buffer. */
		u32 CpuOffset; /**< In multiples of 4 bytes. */
	};

	/**	Describes a single GPU program object (for example texture, sampler state) parameter. */
	struct GpuObjectParameterInformation
	{
		String Name;
		GpuParameterObjectType Type;

		u32 Slot; /**< Slot within a set. Uniquely identifies bind location in the GPU pipeline, together with the set. */
		u32 Set; /**< Uniquely identifies the bind location in the GPU pipeline, together with the slot. */
		GpuBufferFormat ElementType = BF_UNKNOWN; /**< Underlying type of individual elements in the buffer or texture. */
		u32 ArraySize = 1; /**< Number of elements in the array, if the parameter is an array. */
	};

	/**	Describes a GPU program parameter block (collection of GPU program data parameters). */
	struct GpuDataParameterBlockInformation
	{
		String Name;
		u32 Slot; /** Slot within a set. Uniquely identifies bind location in the GPU pipeline, together with the set. */
		u32 Set; /** Uniquely identifies the bind location in the GPU pipeline, together with the slot. */
		u32 BlockSize; /**< In multiples of 4 bytes. */
		bool IsShareable; /** True for blocks that can be shared between different GPU pipeline stages. */
	};

	/** Contains information about all parameters (i.e. uniforms) for a single GPU program, including data/object parameters and parameter blocks. */
	struct B3D_CORE_EXPORT GpuProgramParameterDescription : IReflectable
	{
		Map<String, GpuDataParameterBlockInformation> UniformBuffers;
		Map<String, GpuDataParameterInformation> UniformBufferMembers;

		Map<String, GpuObjectParameterInformation> Samplers;
		Map<String, GpuObjectParameterInformation> SampledTextures;
		Map<String, GpuObjectParameterInformation> StorageTextures;
		Map<String, GpuObjectParameterInformation> Buffers;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class GpuProgramParameterDescriptionRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const;
	};

	/** @} */
} // namespace bs
