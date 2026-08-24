#include "$ENGINE$\TextureCompression\TextureCompressBC7Partition.bslinc"

shader TextureCompressBC7PartitionSearch
{
	featureset = HighEnd;

	mixin TextureCompressBC7Partition;

	variations
	{
		BC7_MODE = { 0, 1, 2, 3, 7 };
	};

	code
	{
		Texture2D gInput;
		RWBuffer<uint4> gBC7Candidate;

		[internal]
		cbuffer Parameters
		{
			int2 gTextureSize;
			int2 gBlockCount;
			int2 gBlockOffset;
			int2 gScratchBlockCount;
			int gBC7PartitionOffset;
		}

		groupshared int4 gBC7BlockTexels[16];
		groupshared float gBC7PartitionErrors[16];

		[numthreads(16, 1, 1)]
		void csmain(uint3 groupId : SV_GroupID, uint3 groupThreadId : SV_GroupThreadID)
		{
			const uint2 localBlockId = groupId.xy;
			const uint2 blockId = localBlockId + (uint2)gBlockOffset;
			const uint threadIndex = groupThreadId.x;

			if (threadIndex < 16u)
			{
				const uint2 texelInBlock = uint2(threadIndex & 3u, threadIndex >> 2u);
				const uint2 maxCoordinate = (uint2)gTextureSize - 1u;
				const uint2 coordinate = min(blockId * 4u + texelInBlock, maxCoordinate);
				gBC7BlockTexels[threadIndex] = (int4)round(saturate(gInput.Load(int3((int2)coordinate, 0))) * 255.0f);
			}

			GroupMemoryBarrierWithGroupSync();

			const uint partition = (uint)gBC7PartitionOffset + threadIndex;
			int4 texels[16];
			[unroll]
			for (uint texelIndex = 0; texelIndex < 16u; ++texelIndex)
				texels[texelIndex] = gBC7BlockTexels[texelIndex];

			int4 endpoint0[3];
			int4 endpoint1[3];
			uint pbit0[3];
			uint pbit1[3];
			uint indices[16];
			float partitionError;

			#if BC7_MODE == 0
				EvaluateBC7Partition(texels, 3, 3, 4, 1, 3, (int)partition, endpoint0, endpoint1, pbit0,
					pbit1, indices, partitionError);
			#elif BC7_MODE == 1
				EvaluateBC7Partition(texels, 2, 3, 6, 2, 3, (int)partition, endpoint0, endpoint1, pbit0,
					pbit1, indices, partitionError);
			#elif BC7_MODE == 2
				EvaluateBC7Partition(texels, 3, 3, 5, 0, 2, (int)partition, endpoint0, endpoint1, pbit0,
					pbit1, indices, partitionError);
			#elif BC7_MODE == 3
				EvaluateBC7Partition(texels, 2, 3, 7, 1, 2, (int)partition, endpoint0, endpoint1, pbit0,
					pbit1, indices, partitionError);
			#else
				EvaluateBC7Partition(texels, 2, 4, 5, 1, 2, (int)partition, endpoint0, endpoint1, pbit0,
					pbit1, indices, partitionError);
			#endif

			gBC7PartitionErrors[threadIndex] = partitionError;

			GroupMemoryBarrierWithGroupSync();

			if (threadIndex == 0u)
			{
				float batchBestError = gBC7PartitionErrors[0];
				uint batchBestPartition = (uint)gBC7PartitionOffset;
				for (uint partitionInBatch = 1u; partitionInBatch < 16u; ++partitionInBatch)
				{
					const float error = gBC7PartitionErrors[partitionInBatch];
					if (error < batchBestError)
					{
						batchBestError = error;
						batchBestPartition = (uint)gBC7PartitionOffset + partitionInBatch;
					}
				}

				const uint candidateIndex = localBlockId.y * (uint)gScratchBlockCount.x + localBlockId.x;
				const uint4 previousCandidate = gBC7Candidate[candidateIndex];
				if (gBC7PartitionOffset == 0 || batchBestError < asfloat(previousCandidate.y))
					gBC7Candidate[candidateIndex] = uint4(batchBestPartition, asuint(batchBestError), 0u, 0u);
			}
		}
	};
};
