//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DIReflectable.h"

namespace b3d
{
	class ShaderParameterDescription;

	/** Source metadata for one entry point, before native bytecode reflection. */
	struct ShaderEntryPointReflection
	{
		ShaderEntryPointReflection() = default;

		GpuProgramType Type = GPT_VERTEX_PROGRAM;
		Array<u32, 3> ThreadGroupSize = { 1, 1, 1 }; /**< Compute threads per workgroup. */
		u32 PushConstantBufferSize = 0; /**< Declared push-constant size in bytes. */
	};

	/** Reflection for one source compilation. Binding coordinates belong to its target, pass and variation. */
	struct B3D_EXPORT ShaderReflection : IReflectable
	{
		ShaderReflection() = default;

		/** Declarations and target-specific bindings shared by stages of a compiled pass. */
		TShared<ShaderParameterDescription> Parameters;

		/** Fingerprint of all source parameter declarations and bindings, including internal resources and struct members. */
		Array<u64, 2> ParameterLayoutHash = { 0, 0 };

		/** Entry points keyed by their generated names; discovery-only reflection uses the source names. */
		Map<String, ShaderEntryPointReflection> EntryPoints;

		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};
}
