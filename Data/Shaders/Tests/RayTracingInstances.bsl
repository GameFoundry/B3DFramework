//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "$ENGINE$\RayTracingInstances.bslinc"

shader RayTracingInstances
{
	mixin RayTracingInstances;

	code
	{
		// Marks a source that is written with WriteEmptyRayTracingInstance(), in the flags
		#define WRITE_EMPTY_INSTANCE 0x80000000

		/** Instance to write: the transform by rows, then the id, hit group offset, mask and flags. */
		struct InstanceSource
		{
			float4 TransformRows[3];
			uint InstanceId;
			uint HitGroupIndexOffset;
			uint Mask;
			uint Flags;
		};

		[pushConstant] cbuffer Parameters
		{
			uint InstanceCount;
		};

		StructuredBuffer<InstanceSource> Sources;
		StructuredBuffer<uint2> References;
		RWByteAddressBuffer Instances;

		// Thread N writes the instance described by Sources[N], with the structure References[N]
		[numthreads(64, 1, 1)]
		void csmain(uint3 threadId : SV_DispatchThreadID)
		{
			const uint instanceIndex = threadId.x;
			if(instanceIndex >= InstanceCount)
				return;

			const InstanceSource source = Sources[instanceIndex];
			if((source.Flags & WRITE_EMPTY_INSTANCE) != 0)
			{
				WriteEmptyRayTracingInstance(Instances, instanceIndex);
				return;
			}

			RayTracingInstance instance;
			instance.Transform = float3x4(source.TransformRows[0], source.TransformRows[1], source.TransformRows[2]);
			instance.InstanceId = source.InstanceId;
			instance.HitGroupIndexOffset = source.HitGroupIndexOffset;
			instance.Mask = source.Mask;
			instance.Flags = source.Flags;
			instance.BottomLevel = References[instanceIndex];

			WriteRayTracingInstance(Instances, instanceIndex, instance);
		}
	};
};
