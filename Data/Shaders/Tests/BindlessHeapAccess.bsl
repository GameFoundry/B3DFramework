//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "$ENGINE$\BindlessSamplers.bslinc"

shader BindlessHeapAccess
{
	mixin BindlessSamplers;

	code
	{
		// The texture index is in the low and the sampler index in the high 16 bits of TextureAndSamplerIndices
		[pushConstant] cbuffer Indices
		{
			uint StructuredIndex;
			uint TypedIndex;
			uint TextureAndSamplerIndices;
		};

		RWStructuredBuffer<uint> Output;

		[numthreads(1, 1, 1)]
		void csmain()
		{
			StructuredBuffer<uint> structuredBuffer = ResourceDescriptorHeap[StructuredIndex];
			Buffer<uint> typedBuffer = ResourceDescriptorHeap[TypedIndex];
			Texture2D<float4> sourceTexture = ResourceDescriptorHeap[TextureAndSamplerIndices & 0xFFFF];
			SamplerState cachedSampler = SamplerDescriptorHeap[TextureAndSamplerIndices >> 16];
			SamplerState globalSampler = SamplerDescriptorHeap[GetGlobalSamplerIndex(GLOBAL_SAMPLER_FILTER_POINT, GLOBAL_SAMPLER_ADDRESS_CLAMP)];

			Output[0] = structuredBuffer[1];
			Output[1] = typedBuffer[2];
			Output[2] = (uint)(sourceTexture.Load(int3(1, 0, 0)).r * 255.0f + 0.5f);
			Output[3] = (uint)(sourceTexture.SampleLevel(cachedSampler, float2(0.75f, 0.5f), 0).g * 255.0f + 0.5f);
			Output[4] = (uint)(sourceTexture.SampleLevel(globalSampler, float2(0.25f, 0.5f), 0).b * 255.0f + 0.5f);
		}
	};
};
