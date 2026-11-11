//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Material/B3DShaderReflection.h"
#include "RTTI/B3DShaderRTTI.h"
#include "RTTI/B3DFlagsRTTI.h"

namespace b3d
{
	template <>
	struct RTTIPlainType<GpuProgramEntryPointReflection> : RTTIPlainTypeHelper<GpuProgramEntryPointReflection, TID_GpuProgramEntryPointReflection, 2>
	{
		template<class Processor>
		static void RTTIEnumerateFields(GpuProgramEntryPointReflection& object, Processor& processor, u8 version)
		{
			processor(object.Type);
			processor(object.ThreadGroupSize);
			processor(object.PushConstantBufferSize);

			if(version > 0)
				processor(object.Features);

			if(version > 1)
			{
				processor(object.RayPayloadSize);
				processor(object.HitAttributeSize);
			}
		}
	};

	class ShaderReflectionRTTI : public TRTTIType<ShaderReflection, IReflectable, ShaderReflectionRTTI>
	{
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(Parameters, 0)
			B3D_RTTI_MEMBER(EntryPoints, 1)
			B3D_RTTI_MEMBER(ParameterLayoutHash, 2)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "ShaderReflection";
			return name;
		}

		u32 GetRttiId() const override { return TID_ShaderReflection; }
		TShared<IReflectable> NewRttiObject() override { return B3DMakeShared<ShaderReflection>(); }
	};
}
