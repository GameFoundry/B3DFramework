#include "$ENGINE$\ReflectionCubemapCommon.bslinc"
#include "$ENGINE$\SHCommon.bslinc"

shader IrradianceReduceSH
{
	mixin ReflectionCubemapCommon;
	mixin SHCommon;

	featureset = HighEnd;
	
	variations
	{
		SH_ORDER = { 3, 5 };
	};	
	
	code
	{
		#define PI 3.1415926
	
		struct SHCoeffsAndWeight
		{
			SHVectorRGB coeffs;
			float weight;
		};

		StructuredBuffer<SHCoeffsAndWeight> gInput;
		RWTexture2D<float4> gOutput;
		
		[internal]
		cbuffer Params
		{
			uint2 gOutputIdx;
			uint gNumEntries;
		}			
		
		[numthreads(1, 1, 1)]
		void csmain(
			uint groupIdx : SV_GroupIndex,
			uint groupId : SV_GroupID,
			uint3 dispatchThreadId : SV_DispatchThreadID)
		{
			SHVectorRGB coeffs;
			float weight = 0;
			
			SHZero(coeffs.R);
			SHZero(coeffs.G);
			SHZero(coeffs.B);
			
			// Note: There shouldn't be many entries, so we add them all in one thread. Otherwise we should do parallel reduction.
			for(uint i = 0; i < gNumEntries; i++)
			{
				SHCoeffsAndWeight current = gInput[i];
			
				SHAdd(coeffs.R, current.coeffs.R);
				SHAdd(coeffs.G, current.coeffs.G);
				SHAdd(coeffs.B, current.coeffs.B);

				weight += current.weight;
			}
			
			// Normalize
			float normFactor = (4 * PI) / weight;
			SHMultiply(coeffs.R, normFactor);
			SHMultiply(coeffs.G, normFactor);
			SHMultiply(coeffs.B, normFactor);
			
			uint2 writeIdx = gOutputIdx;
			[unroll]
			for(int i = 0; i < SH_NUM_COEFFS; ++i)
			{			
				gOutput[writeIdx] = float4(coeffs.R.v[i], coeffs.G.v[i], coeffs.B.v[i], 0.0f);
				writeIdx.x += 1;
			}
		}
	};
};
