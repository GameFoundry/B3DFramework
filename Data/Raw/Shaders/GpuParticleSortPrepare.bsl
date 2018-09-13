shader GpuParticleSortPrepare
{
	featureset = HighEnd;
	
	code
	{
		Texture2D gPosAndTimeTex;
		Buffer<uint2> gInputIndices;
		
		RWBuffer<uint> gOutputKeys;
		RWBuffer<uint2> gOutputIndices;
		
		cbuffer Input
		{
			uint gIterationsPerGroup;
			uint gNumExtraIterations;
			uint gNumParticles;
			uint gOutputOffset;
			uint gSystemKey;
			float3 gLocalViewOrigin;
		};	
		
		[numthreads(NUM_THREADS, 1, 1)]
		void csmain(uint3 groupThreadId : SV_GroupThreadID, uint3 groupId : SV_GroupID)
		{
			uint threadId = groupThreadId.x;
		
			uint inputIdx;
			uint numIterations;
			
			if(groupId.x < gNumExtraIterations)
			{
				inputIdx = groupId.x * (gIterationsPerGroup + 1);
				numIterations = gIterationsPerGroup + 1;
			}
			else
			{
				inputIdx = groupId.x * gIterationsPerGroup + gNumExtraIterations;
				numIterations = gIterationsPerGroup;
			}
					
			inputIdx = inputIdx * NUM_THREADS + threadId;
			uint outputIdx = gOutputOffset + inputIdx;
		
			for(uint i = 0; i < numIterations; ++i)
			{
				if(inputIdx >= gNumParticles)
					break;
					
				uint3 pixelPos;
				pixelPos.xy = gInputIndices[inputIdx];
				pixelPos.z = 0;
				
				float3 position = gPosAndTimeTex.Load(pixelPos).xyz;
				float dist = length(position - gLocalViewOrigin);
				
				uint key = f32tof16(dist);
				
				// Flip the sign to sort from furthest to nearest. This works because positive float bits
				// will still compare properly. Negative ones won't be those are invisible behind the camera anyway.
				key = ((~key) & 0xFFFF) | gSystemKey;

				gOutputKeys[outputIdx] = key;
				gOutputIndices[outputIdx] = pixelPos.xy;
					
				inputIdx += NUM_THREADS;
				outputIdx += NUM_THREADS;
			}	
		}	
	};
};