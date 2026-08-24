#include "$ENGINE$\TextureCompression\TextureCompressBC7Mode5.bslinc"

shader TextureCompressBC7Mode5Finalize
{
	featureset = HighEnd;

	mixin TextureCompressBC7Mode5;

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
		groupshared uint4 gBC7Mode5Blocks[4];
		groupshared float gBC7Mode5Errors[4];

		[numthreads(4, 1, 1)]
		void csmain(uint3 groupId : SV_GroupID, uint3 groupThreadId : SV_GroupThreadID)
		{
			const uint2 localBlockId = groupId.xy;
			const uint2 blockId = localBlockId + (uint2)gBlockOffset;
			const uint threadIndex = groupThreadId.x;
			const uint2 maxCoordinate = (uint2)gTextureSize - 1u;

			[unroll]
			for (uint loadIndex = 0u; loadIndex < 4u; ++loadIndex)
			{
				const uint texelIndex = threadIndex + loadIndex * 4u;
				const uint2 texelInBlock = uint2(texelIndex & 3u, texelIndex >> 2u);
				const uint2 coordinate = min(blockId * 4u + texelInBlock, maxCoordinate);
				gBC7BlockTexels[texelIndex] = gInput.Load(int3((int2)coordinate, 0));
			}
			GroupMemoryBarrierWithGroupSync();

			float4 texels[16];
			[unroll]
			for (uint texelIndex = 0u; texelIndex < 16u; ++texelIndex)
				texels[texelIndex] = gBC7BlockTexels[texelIndex];

			float candidateError;
			gBC7Mode5Blocks[threadIndex] = CompressBC7Mode5Candidate(texels, threadIndex, candidateError);
			gBC7Mode5Errors[threadIndex] = candidateError;
			GroupMemoryBarrierWithGroupSync();

			if (threadIndex == 0u)
			{
				uint bestCandidate = 0u;
				float bestError = gBC7Mode5Errors[0];
				[unroll]
				for (uint candidate = 1u; candidate < 4u; ++candidate)
				{
					if (gBC7Mode5Errors[candidate] < bestError)
					{
						bestCandidate = candidate;
						bestError = gBC7Mode5Errors[candidate];
					}
				}

				const float previousError = gBestErr[localBlockId];
				if (bestError < previousError)
				{
					gOutput[blockId] = gBC7Mode5Blocks[bestCandidate];
					gBestErr[localBlockId] = bestError;
				}
			}
		}
	};
};
