//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"

namespace b3d
{
	/** @addtogroup GpuBackend
	 *  @{
	 */

	/** Push-constant block size guaranteed by every backend that advertises push-constant support. */
	inline constexpr u32 kMaxPushConstantSizeInBytes = 16;

	/** Number of 32-bit values in the guaranteed push-constant block. */
	inline constexpr u32 kMaxPushConstantValueCount = kMaxPushConstantSizeInBytes / sizeof(u32);

	/** Reserved HLSL register used by shader compiler for marking the push-constant buffer. */
	inline constexpr u32 kPushConstantHlslRegister = 0;

	/** Reserved HLSL register space used by shader compiler for marking the push-constant buffer. */
	inline constexpr u32 kPushConstantHlslRegisterSpace = 0xFFFF;

	/** CPU-side storage used by backends to retain the complete value across partial push-constant writes. */
	struct B3D_EXPORT GpuPushConstantPayload
	{
		GpuPushConstantPayload() = default;

		Array<u32, kMaxPushConstantValueCount> Values{};

		/** Clears all values to zero. */
		void Clear();

		/** Copies a validated byte range into the payload. */
		void Write(u32 offsetInBytes, u32 sizeInBytes, const void* data);

		const void* GetData() const { return Values.data(); }
	};

	/** @} */
} // namespace b3d
