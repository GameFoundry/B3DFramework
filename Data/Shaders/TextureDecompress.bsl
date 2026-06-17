//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//

// Decodes a block-compressed texture (any BCn format) back to a full-resolution float4 surface using the GPU's own
// texture-unit decoder: each thread samples one texel at its exact centre - with point filtering, so the sample
// returns that texel's decoded value with no interpolation - and writes it to an uncompressed RGBA32F UAV. This exists
// for verification tooling that needs to know what the *hardware* decoder reconstructs (the value actually seen when
// the texture is sampled at runtime), independent of any CPU reference decoder.
shader TextureDecompress
{
	featureset = HighEnd;

	code
	{
		// Compressed source. Sampling it makes the texture unit run the real hardware decode.
		Texture2D gInput;

		// Point sampler so a texel-centre sample returns exactly that texel's decoded value (no filtering leak from
		// neighbouring texels - important so a single firefly texel is measured in isolation).
		SamplerState gSampler
		{
			Filter = MIN_MAG_MIP_POINT;
			AddressU = CLAMP;
			AddressV = CLAMP;
		};

		// Decoded output, one float4 per pixel, at the source resolution.
		RWTexture2D<float4> gOutput;

		[internal]
		cbuffer Parameters
		{
			int2 gSize; // output size in pixels
		}

		[numthreads(8, 8, 1)]
		void csmain(uint3 dispatchId : SV_DispatchThreadID)
		{
			if((int)dispatchId.x >= gSize.x || (int)dispatchId.y >= gSize.y)
				return;

			// Sample the texel centre so point filtering reproduces the block's decoded texel exactly.
			float2 uv = (float2(dispatchId.xy) + 0.5f) / float2(gSize);
			gOutput[dispatchId.xy] = gInput.SampleLevel(gSampler, uv, 0);
		}
	};
};
