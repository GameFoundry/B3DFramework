shader ClearLoadStore
{
	featureset = HighEnd;

	variations
	{
	    // 0 - Texture, 1 - Texture array, 2 - Buffer, 3 - StructuredBuffer
		OBJ_TYPE = { 0, 1, 2, 3 };

		// 0 - Float, 1 - Integer
		DATA_TYPE = { 0, 1 };

		NUM_COMPONENTS = { 0, 1, 2, 3 };
	};
	
	code
	{
	    #if DATA_TYPE == 0
	        #if NUM_COMPONENTS == 0
                #define TYPE float
                #define CLEAR_VAL gFloatClearVal.x
            #elif NUM_COMPONENTS == 1
                #define TYPE float2
                #define CLEAR_VAL gFloatClearVal.xy
            #elif NUM_COMPONENTS == 2
                #define TYPE float3
                #define CLEAR_VAL gFloatClearVal.xyz
            #elif NUM_COMPONENTS == 3
                #define TYPE float4
                #define CLEAR_VAL gFloatClearVal
            #endif
	    #else
	        #if NUM_COMPONENTS == 0
                #define TYPE uint
                #define CLEAR_VAL gIntClearVal.x
            #elif NUM_COMPONENTS == 1
                #define TYPE uint2
                #define CLEAR_VAL gIntClearVal.xy
            #elif NUM_COMPONENTS == 2
                #define TYPE uint3
                #define CLEAR_VAL gIntClearVal.xyz
            #elif NUM_COMPONENTS == 3
                #define TYPE uint4
                #define CLEAR_VAL gIntClearVal
            #endif
	    #endif

	    #if OBJ_TYPE == 0
			RWTexture2D<TYPE> gOutput;
	    #elif OBJ_TYPE == 1
			RWTexture2DArray<TYPE> gOutput;
	    #elif OBJ_TYPE == 2
		    RWBuffer<TYPE> gOutput;
		#else
			RWStructuredBuffer<TYPE> gOutput;
	    #endif

		[internal]
		cbuffer Params
		{
			uint2 gSize;
			float4 gFloatClearVal;
			uint4 gIntClearVal;
		}

		[numthreads(NUM_THREADS, 1, 1)]
		void csmain(uint3 groupThreadId : SV_GroupThreadID, uint3 groupId : SV_GroupID)
		{
			#if OBJ_TYPE == 0 || OBJ_TYPE == 1
                uint2 tilePos;
                tilePos.x = groupId.x * NUM_THREADS + groupThreadId.x;
                tilePos.y = groupId.y * NUM_THREADS + groupThreadId.y;

                if(tilePos.x >= gSize.x || tilePos.y >= gSize.y)
                    return;

                for (uint y = 0; y < TILE_SIZE; ++y)
                {
                    for (uint x = 0; x < TILE_SIZE; ++x)
                    {
                        uint2 pixelPos = tilePos + uint2(x, y);
                        if(pixelPos.x >= gSize.x || pixelPos.y >= gSize.y)
                            continue;

                        #if OBJ_TYPE == 1
                            gOutput[uint3(pixelPos, 0)] = CLEAR_VAL;
                        #else
                            gOutput[pixelPos] = CLEAR_VAL;
                        #endif
                    }
                }
			#else
                uint threadIdx = groupId.x * NUM_THREADS + groupThreadId.x;
                uint pos = threadIdx * (TILE_SIZE * TILE_SIZE);

                if(pos >= gSize.x)
                    return;

                for (uint x = 0; x < (TILE_SIZE * TILE_SIZE); ++x)
                {
                    uint curPos = pos + x;
                    if(curPos.x >= gSize.x)
                        continue;

                    gOutput[curPos] = CLEAR_VAL;
                }
			#endif
		}
	};
};
