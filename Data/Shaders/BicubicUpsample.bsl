shader BicubicUpsample
{
	depth
	{	
		read = false;
		write = false;	
	};
	
	blend
	{
		target	
		{
			enabled = true;
			color = { one, one, add };
		};
	};
	
	variations
	{
		HERMITE = { true, false };
	};
	

	code
	{
		#if HERMITE
			#define CUBIC_FUNC cubicHermite
		#else
			#define CUBIC_FUNC cubicLagrange
		#endif
	
		[internal]
		cbuffer Input
		{
			[color]
			float4 gTint;
			uint2 gTextureSize;
			float2 gInvPixel;
			float2 gInvTwoPixels;
		}		

		struct VStoFS
		{
			noperspective float4 position : SV_POSITION;
			noperspective float2 uv0 : TEXCOORD0;
		};

		struct VertexInput
		{
			float2 screenPos : POSITION;
			float2 uv0 : TEXCOORD0;
		};
		
		VStoFS vsmain(VertexInput input)
		{
			VStoFS output;
		
			output.position = float4(input.screenPos, 0, 1);
			output.uv0 = input.uv0;

			return output;
		}			

		Texture2D<float4> gSource;
		
		[alias(gSource)]
		SamplerState gSampler
		{
			Filter = MIN_MAG_MIP_POINT;
		};
		
		static float4 OFFSET = float4(-1.0f, 0.0f, 1.0f, 2.0f);
		
		float3 cubicLagrange(float3 A, float3 B, float3 C, float3 D, float t)
		{
			return
				A * 
				(
					(t - OFFSET.y) / (OFFSET.x - OFFSET.y) * 
					(t - OFFSET.z) / (OFFSET.x - OFFSET.z) *
					(t - OFFSET.w) / (OFFSET.x - OFFSET.w)
				) +
				B * 
				(
					(t - OFFSET.x) / (OFFSET.y - OFFSET.x) * 
					(t - OFFSET.z) / (OFFSET.y - OFFSET.z) *
					(t - OFFSET.w) / (OFFSET.y - OFFSET.w)
				) +
				C * 
				(
					(t - OFFSET.x) / (OFFSET.z - OFFSET.x) * 
					(t - OFFSET.y) / (OFFSET.z - OFFSET.y) *
					(t - OFFSET.w) / (OFFSET.z - OFFSET.w)
				) +       
				D * 
				(
					(t - OFFSET.x) / (OFFSET.w - OFFSET.x) * 
					(t - OFFSET.y) / (OFFSET.w - OFFSET.y) *
					(t - OFFSET.z) / (OFFSET.w - OFFSET.z)
				);
		}
		
		float3 cubicHermite(float3 A, float3 B, float3 C, float3 D, float t)
		{
			float t2 = t*t;
			float t3 = t*t*t;
			float3 a = -A/2.0f + (3.0f*B)/2.0f - (3.0f*C)/2.0f + D/2.0f;
			float3 b = A - (5.0f*B)/2.0f + 2.0f*C - D / 2.0f;
			float3 c = -A/2.0f + C/2.0f;
			float3 d = B;
			
			return a*t3 + b*t2 + c*t + d;
		}
	
		float4 fsmain(VStoFS input) : SV_Target0
		{
			float2 pixel = input.uv0 * gTextureSize + 0.5f;
    
			float2 fractional = frac(pixel);
			pixel = floor(pixel) / gTextureSize - gInvPixel/2.0f;
	
			// Note: Investigate a version requiring less taps: http://vec3.ca/bicubic-filtering-in-fewer-taps/
			float3 C00 = gSource.Sample(gSampler, pixel + float2(-gInvPixel.x    	, -gInvPixel.y)).rgb;
			float3 C10 = gSource.Sample(gSampler, pixel + float2( 0.0f         		, -gInvPixel.y)).rgb;
			float3 C20 = gSource.Sample(gSampler, pixel + float2( gInvPixel.x    	, -gInvPixel.y)).rgb;
			float3 C30 = gSource.Sample(gSampler, pixel + float2( gInvTwoPixels.x	, -gInvPixel.y)).rgb;
			
			float3 C01 = gSource.Sample(gSampler, pixel + float2(-gInvPixel.x 		, 0.0f)).rgb;
			float3 C11 = gSource.Sample(gSampler, pixel + float2( 0.0f    			, 0.0f)).rgb;
			float3 C21 = gSource.Sample(gSampler, pixel + float2( gInvPixel.x 		, 0.0f)).rgb;
			float3 C31 = gSource.Sample(gSampler, pixel + float2( gInvTwoPixels.x	, 0.0f)).rgb;    
			
			float3 C02 = gSource.Sample(gSampler, pixel + float2(-gInvPixel.x 		, gInvPixel.y)).rgb;
			float3 C12 = gSource.Sample(gSampler, pixel + float2( 0.0f        		, gInvPixel.y)).rgb;
			float3 C22 = gSource.Sample(gSampler, pixel + float2( gInvPixel.x 		, gInvPixel.y)).rgb;
			float3 C32 = gSource.Sample(gSampler, pixel + float2( gInvTwoPixels.x	, gInvPixel.y)).rgb;    
			
			float3 C03 = gSource.Sample(gSampler, pixel + float2(-gInvPixel.x 		, gInvTwoPixels.y)).rgb;
			float3 C13 = gSource.Sample(gSampler, pixel + float2( 0.0f        		, gInvTwoPixels.y)).rgb;
			float3 C23 = gSource.Sample(gSampler, pixel + float2( gInvPixel.x 		, gInvTwoPixels.y)).rgb;
			float3 C33 = gSource.Sample(gSampler, pixel + float2( gInvTwoPixels.x	, gInvTwoPixels.y)).rgb;    
			
			float3 CP0X = CUBIC_FUNC(C00, C10, C20, C30, fractional.x);
			float3 CP1X = CUBIC_FUNC(C01, C11, C21, C31, fractional.x);
			float3 CP2X = CUBIC_FUNC(C02, C12, C22, C32, fractional.x);
			float3 CP3X = CUBIC_FUNC(C03, C13, C23, C33, fractional.x);
		
			return float4(CUBIC_FUNC(CP0X, CP1X, CP2X, CP3X, fractional.y), 1.0f) * gTint;
		}
	};
};
