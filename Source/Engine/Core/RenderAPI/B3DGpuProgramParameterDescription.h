//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Utility/B3DResult.h"

namespace b3d
{
	/** @addtogroup RenderAPI
	 *  @{
	 */

	/** Describes a single GPU program data (for example int, float, Vector2) parameter. */
	struct GpuUniformBufferMemberInformation
	{
		String Name;
		u32 ElementSize; /**< In multiples of 4 bytes. */
		u32 ArraySize;
		u32 ArrayElementStride; /**< In multiples of 4 bytes. */
		GpuDataParameterType Type;

		u32 ParentUniformBufferSlot;
		u32 ParentUniformBufferSet;
		u32 GpuOffset; /**< In multiples of 4 bytes, or index for parameters not in a buffer. */
		u32 CpuOffset; /**< In multiples of 4 bytes. */

		bool operator==(const GpuUniformBufferMemberInformation& other) const;
		bool operator!=(const GpuUniformBufferMemberInformation& other) const { return !operator==(other); }
	};

	inline bool GpuUniformBufferMemberInformation::operator==(const GpuUniformBufferMemberInformation& other) const
	{
		return Name == other.Name &&
			ElementSize == other.ElementSize &&
			ArraySize == other.ArraySize &&
			ArrayElementStride == other.ArrayElementStride &&
			Type == other.Type &&
			ParentUniformBufferSlot == other.ParentUniformBufferSlot &&
			ParentUniformBufferSet == other.ParentUniformBufferSet &&
			GpuOffset == other.GpuOffset &&
			CpuOffset == other.CpuOffset;
	}

	/**	Describes a single GPU program object (for example texture, sampler state) parameter. */
	struct GpuObjectParameterInformation
	{
		String Name;
		GpuParameterObjectType Type;

		u32 Slot; /**< Slot within a set. Uniquely identifies bind location in the GPU pipeline, together with the set. */
		u32 Set; /**< Uniquely identifies the bind location in the GPU pipeline, together with the slot. */
		GpuBufferFormat ElementType = BF_UNKNOWN; /**< Underlying type of individual elements in the buffer or texture. */
		u32 ArraySize = 1; /**< Number of elements in the array, if the parameter is an array. */
		GpuProgramStageBits Stages = GpuProgramStageBit::None; /**< Stages in which the parameter is used in. */
	};

	/**	Describes a GPU program parameter block (collection of GPU program data parameters). */
	struct GpuUniformBufferInformation
	{
		String Name;
		u32 Slot; /** Slot within a set. Uniquely identifies bind location in the GPU pipeline, together with the set. */
		u32 Set; /** Uniquely identifies the bind location in the GPU pipeline, together with the slot. */
		u32 Size; /**< In multiples of 4 bytes. */
		GpuProgramStageBits Stages = GpuProgramStageBit::None; /**< Stages in which the parameter is used in. */
		bool IsShareable; /** True for blocks that can be shared between different GPU pipeline stages. */
	};

	/** Contains information about all parameters (i.e. uniforms) for a single GPU program, including data/object parameters and parameter blocks. */
	struct B3D_EXPORT GpuProgramParameterDescription : IReflectable
	{
		Map<String, GpuUniformBufferInformation> UniformBuffers;
		Map<String, GpuUniformBufferMemberInformation> UniformBufferMembers;

		Map<String, GpuObjectParameterInformation> Samplers;
		Map<String, GpuObjectParameterInformation> SampledTextures;
		Map<String, GpuObjectParameterInformation> StorageTextures;
		Map<String, GpuObjectParameterInformation> Buffers;

		/**
		 * Attempts to combine another parameter description into this one. Parameters with the same name
		 * have their stage fields combined. Parameters with the same name but different type, slot, or array size
		 * will cause the method to fail.
		 *
		 * @param other		The parameter description to combine into this one.
		 * @param stage		The GPU program stage bit to apply to all parameters from @p other.
		 * @return			Result indicating success or failure with an error message.
		 */
		Result TryCombine(const GpuProgramParameterDescription& other, GpuProgramStageBit stage);

		/**
		 * Splits this parameter description into multiple descriptions, one per set. Each output description
		 * contains only the parameters belonging to that set index.
		 *
		 * @param output	Array to receive the per-set parameter descriptions. Will be resized as needed.
		 */
		void SplitBySet(TInlineArray<GpuProgramParameterDescription, 4>& output) const;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class GpuProgramParameterDescriptionRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const;
	};

	/** @} */
} // namespace b3d
