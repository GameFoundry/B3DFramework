//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DNullPrerequisites.h"
#include "RenderAPI/B3DGpuProgram.h"

namespace b3d
{
	namespace render
	{
		class NullGpuDevice;

		/** @addtogroup NullGpuBackend 
		 *  @{
		 */

		/**	Abstraction of a null shader object. */
		class NullGpuProgram : public GpuProgram
		{
		public:
			NullGpuProgram(NullGpuDevice& gpuDevice, const GpuProgramCreateInformation& createInformation);
			virtual ~NullGpuProgram() = default;

			void Initialize() override {}
		};

		/** Identifier of the compiler used for compiling Null GPU programs. */
		static constexpr const char* NULL_COMPILER_ID = "Null";

		/**
		 * Version of the compiler used for compiling Null GPU programs.
		 */
		static constexpr u32 NULL_COMPILER_VERSION = 1;

		/** @} */
	} // namespace render
} // namespace b3d
