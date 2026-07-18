//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup GpuBackend-Internal
		 *  @{
		 */

		/** HLSL shader register classes, used for encoding engine parameter slots. See MapRegisterToSlot(). */
		enum class HLSLRegisterClass
		{
			ConstantBuffer = 0,	 // b registers
			ShaderResource = 1,	 // t registers
			UnorderedAccess = 2, // u registers
			Sampler = 3,		 // s registers

			Count = 4
		};

		/**
		 * Maps an HLSL shader register to an engine parameter slot. HLSL registers are only unique within their register
		 * class (b/t/u/s), while engine slots must be unique across all parameters of a set, so the class is encoded
		 * into the slot. Inverted by MapSlotToRegister().
		 *
		 * This encoding is the ABI between the hlsl bytecode compiler (which stamps the slots into the reflected
		 * parameter descriptions) and the Direct3D GPU backends (which recover the registers when binding), so both
		 * sides must use these functions rather than their own arithmetic.
		 */
		constexpr u32 MapRegisterToSlot(u32 registerIndex, HLSLRegisterClass registerClass)
		{
			return registerIndex * (u32)HLSLRegisterClass::Count + (u32)registerClass;
		}

		/** Recovers the HLSL shader register from an engine parameter slot encoded by MapRegisterToSlot(). */
		constexpr u32 MapSlotToRegister(u32 slot)
		{
			return slot / (u32)HLSLRegisterClass::Count;
		}

		/** @} */
	} // namespace render
} // namespace b3d
