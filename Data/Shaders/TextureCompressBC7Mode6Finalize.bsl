#include "$ENGINE$\TextureCompression\TextureCompressBC7Mode6.bslinc"

shader TextureCompressBC7Mode6Finalize
{
	featureset = HighEnd;

	mixin TextureCompressBC7Mode6;

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

		[numthreads(8, 8, 1)]
		void csmain(uint3 dispatchId : SV_DispatchThreadID)
		{
			const uint2 localBlockId = dispatchId.xy;
			const uint2 blockId = localBlockId + (uint2)gBlockOffset;
			const uint2 blockCount = (uint2)gBlockCount;
			if (blockId.x >= blockCount.x || blockId.y >= blockCount.y)
				return;

			const uint2 baseCoordinate = blockId * 4u;
			const uint2 maxCoordinate = (uint2)gTextureSize - 1u;
			float4 rgba[16];
			[unroll]
			for (uint y = 0u; y < 4u; ++y)
			{
				[unroll]
				for (uint x = 0u; x < 4u; ++x)
				{
					const uint texelIndex = y * 4u + x;
					const uint2 coordinate = min(baseCoordinate + uint2(x, y), maxCoordinate);
					rgba[texelIndex] = gInput.Load(int3((int2)coordinate, 0));
				}
			}

			float modeError;
			const uint4 modeBlock = CompressBC7Mode6(rgba, modeError);
			const float previousError = gBestErr[localBlockId];
			if (modeError < previousError)
			{
				gOutput[blockId] = modeBlock;
				gBestErr[localBlockId] = modeError;
			}
		}
	};
};
