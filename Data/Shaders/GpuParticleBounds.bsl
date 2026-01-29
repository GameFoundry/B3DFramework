shader GpuParticleBounds
{
	featureset = HighEnd;

	code
	{
		#define NUM_REDUCE_THREADS 32
		#define NUM_SERIAL_REDUCTIONS NUM_THREADS / NUM_REDUCE_THREADS
		
		#define FLT_MAX 3.402823466e+38f
		#define FLT_MIN -3.402823466e+38f

		Buffer<uint2> gParticleIndices;
		Texture2D gPosAndTimeTex;
		
		RWBuffer<float3> gOutput;
		
		cbuffer Input
		{
			uint gIterationsPerGroup;
			uint gNumExtraIterations;
			uint gNumParticles;
		};	
		
		groupshared float3 sGroupMin[NUM_THREADS];
		groupshared float3 sGroupMax[NUM_THREADS];

		[numthreads(NUM_THREADS, 1, 1)]
		void csmain(uint3 groupThreadId : SV_GroupThreadID, uint3 groupId : SV_GroupID)
		{
			uint threadId = groupThreadId.x;
		
			uint particleIdx;
			uint numIterations;
			
			if(groupId.x < gNumExtraIterations)
			{
				particleIdx = groupId.x * (gIterationsPerGroup + 1);
				numIterations = gIterationsPerGroup + 1;
			}
			else
			{
				particleIdx = groupId.x * gIterationsPerGroup + gNumExtraIterations;
				numIterations = gIterationsPerGroup;
			}
					
			particleIdx = particleIdx * NUM_THREADS + threadId;
		
			float3 localMin = FLT_MAX;
			float3 localMax = FLT_MIN;
		
			for(uint i = 0; i < numIterations; ++i)
			{
				if(particleIdx >= gNumParticles)
					break;
					
				uint3 pixelPos;
				pixelPos.xy = gParticleIndices[particleIdx];
				pixelPos.z = 0;
				
				float4 positionAndTime = gPosAndTimeTex.Load(pixelPos);
				
				// Check if particle is dead
				if(positionAndTime.w > 1.0f)
					continue;
					
				localMin = min(localMin, positionAndTime.xyz);
				localMax = max(localMax, positionAndTime.xyz);
				
				particleIdx += NUM_THREADS;
			}
		
			sGroupMin[threadId] = localMin;
			sGroupMax[threadId] = localMax;
		
			GroupMemoryBarrierWithGroupSync();
			
			// Reduce serially first
			uint reduceThreadId = threadId & (NUM_REDUCE_THREADS - 1);
			if(threadId < NUM_REDUCE_THREADS)
			{
				// Reduce minimum
				localMin = sGroupMin[reduceThreadId];
				
				[unroll]
				for(int i = 0; i < NUM_SERIAL_REDUCTIONS; ++i)
					localMin = min(localMin, sGroupMin[i * NUM_REDUCE_THREADS + reduceThreadId]);
				
			}
			else if(threadId < (NUM_REDUCE_THREADS * 2))
			{
				// Reduce maximum
				localMax = sGroupMax[reduceThreadId];
				
				[unroll]
				for(int i = 0; i < NUM_SERIAL_REDUCTIONS; ++i)
					localMax = max(localMax, sGroupMax[i * NUM_REDUCE_THREADS + reduceThreadId]);
			}
			
			GroupMemoryBarrierWithGroupSync();
			
			// Store serial reduction results
			if(threadId < NUM_REDUCE_THREADS)
				sGroupMin[reduceThreadId] = localMin;
			else if(threadId < (NUM_REDUCE_THREADS * 2))
				sGroupMax[reduceThreadId] = localMax;
		
			GroupMemoryBarrierWithGroupSync();
			
			// Do parallel reduction within a warp
			if(threadId < NUM_REDUCE_THREADS)
			{
				// Reduce minimum
				localMin = sGroupMin[reduceThreadId];

				[unroll]
				for(uint i = 1; i < NUM_REDUCE_THREADS; i <<= 1)
				{
					// Note: Bank conflicts with i = 2, 4, etc.
					localMin = min(localMin, sGroupMin[reduceThreadId + i]);
					sGroupMin[reduceThreadId] = localMin;
				}
			}
			else if(threadId < (NUM_REDUCE_THREADS * 2))
			{
				// Reduce maximum
				localMax = sGroupMax[reduceThreadId];

				[unroll]
				for(uint i = 1; i < NUM_REDUCE_THREADS; i <<= 1)
				{
					// Note: Bank conflicts with i = 2, 4, etc.
					localMax = min(localMax, sGroupMax[reduceThreadId + i]);
					sGroupMax[reduceThreadId] = localMax;
				}
			}
			
			GroupMemoryBarrierWithGroupSync();
			
			if(threadId == 0) gOutput[groupId.x * 2 + 0] = sGroupMin[0];
			if(threadId == 1) gOutput[groupId.x * 2 + 1] = sGroupMax[0];
		}
	};
};
