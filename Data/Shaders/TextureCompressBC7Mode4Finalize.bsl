#include "$ENGINE$\TextureCompression\TextureCompressBC7Mode4.bslinc"

shader TextureCompressBC7Mode4Finalize
{
	featureset = HighEnd;

	mixin TextureCompressBC7Mode4;

	code
	{
		Texture2D gInput;
		RWTexture2D<uint4> gOutput;
		RWTexture2D<float> gBestErr;

		[internal]
		cbuffer Parameters
		{
			int2 gTextureSize;
			int2 gBlockCount;
			int2 gBlockOffset;
			int2 gScratchBlockCount;
		}

		groupshared float4 gBC7BlockTexels[16];
		groupshared uint4 gBC7Mode4Blocks[8];
		groupshared float gBC7Mode4Errors[8];

		[numthreads(8, 1, 1)]
		void csmain(uint3 groupId : SV_GroupID, uint3 groupThreadId : SV_GroupThreadID)
		{
			const uint2 localBlockId = groupId.xy;
			const uint2 blockId = localBlockId + (uint2)gBlockOffset;
			const uint threadIndex = groupThreadId.x;
			const uint2 maxCoordinate = (uint2)gTextureSize - 1u;

			[unroll]
			for (uint loadIndex = 0u; loadIndex < 2u; ++loadIndex)
			{
				const uint texelIndex = threadIndex + loadIndex * 8u;
				const uint2 texelInBlock = uint2(texelIndex & 3u, texelIndex >> 2u);
				const uint2 coordinate = min(blockId * 4u + texelInBlock, maxCoordinate);
				gBC7BlockTexels[texelIndex] = gInput.Load(int3((int2)coordinate, 0));
			}
			GroupMemoryBarrierWithGroupSync();

			float4 texels[16];
			[unroll]
			for (uint texelIndex = 0u; texelIndex < 16u; ++texelIndex)
				texels[texelIndex] = gBC7BlockTexels[texelIndex];

			const uint rotation = threadIndex >> 1u;
			const uint indexMode = threadIndex & 1u;
			float candidateError;
			gBC7Mode4Blocks[threadIndex] = CompressBC7Mode4Candidate(texels, rotation, indexMode, candidateError);
			gBC7Mode4Errors[threadIndex] = candidateError;
			GroupMemoryBarrierWithGroupSync();

			if (threadIndex == 0u)
			{
				uint bestCandidate = 0u;
				float bestError = gBC7Mode4Errors[0];
				[unroll]
				for (uint candidate = 1u; candidate < 8u; ++candidate)
				{
					if (gBC7Mode4Errors[candidate] < bestError)
					{
						bestCandidate = candidate;
						bestError = gBC7Mode4Errors[candidate];
					}
				}

				const float previousError = gBestErr[localBlockId];
				if (bestError < previousError)
				{
					gOutput[blockId] = gBC7Mode4Blocks[bestCandidate];
					gBestErr[localBlockId] = bestError;
				}
			}
		}
	};
};
