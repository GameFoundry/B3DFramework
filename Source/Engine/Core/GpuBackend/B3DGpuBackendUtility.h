//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"
#include "GpuBackend/B3DGpuParameterSet.h"
#include "GpuBackend/B3DGpuProgramParameterDescription.h"

namespace b3d
{
	/** @addtogroup GpuBackend
	 *  @{
	 */

	/** Collection of static utility methods shared between GPU backend implementations. */
	class GpuBackendUtility
	{
	public:
		/**
		 * Calculates the size and alignment of a single element within a shader interface block using the std140 layout.
		 *
		 * @param[in]		type		Type of the element. Structs are not supported.
		 * @param[in]		arraySize	Number of array elements of the element (1 if it's not an array).
		 * @param[in, out]	offset		Current location in some parent buffer at which the element should be placed at. If the
		 *								location doesn't match the element's alignment, the value will be modified to a valid
		 *								alignment. In multiples of 4 bytes.
		 * @return						Size of the element, in multiples of 4 bytes.
		 */	
		static u32 CalcStd140MemberSizeAndOffset(GpuDataParameterType type, u32 arraySize, u32& offset)
		{
			const GpuDataParameterTypeInformation& typeInfo = GpuParameterSet::kParamSizes.Lookup[type];
			u32 size = (typeInfo.BaseTypeSize * typeInfo.NumColumns * typeInfo.NumRows) / 4;
			const u32 alignment = typeInfo.Alignment / 4;

			if (alignment > 0)
			{
				const u32 alignOffset = offset % alignment;
				if (alignOffset != 0)
					offset += (alignment - alignOffset);
			}

			if (arraySize > 1)
			{
				// Array elements are always padded and aligned to vec4.
				const u32 sizeRemainder = size % 4;
				if (sizeRemainder != 0)
					size += (4 - sizeRemainder);

				const u32 offsetRemainder = offset % 4;
				if (offsetRemainder != 0)
					offset += (4 - offsetRemainder);
			}

			return size;
		}

		/** Maps resource usage flags to the stages describing on which pipeline stage and how the resource is accessed. */
		static render::GpuStageFlags GetStageFlags(render::GpuResourceUseFlags usage);

		/** Converts a single GpuStageFlag into a readable string representing the access and stage. */
		static const char* GetAccessStageName(render::GpuStageFlag flag);

		/** Appends every bit set in @p flags to @p output as readable stage names, using "|" as separator. */
		static void GetAccessStageNames(render::GpuStageFlags flags, StringStream& output);
	};

	/** @} */
} // namespace b3d
