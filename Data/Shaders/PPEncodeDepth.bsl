#include "$ENGINE$\PPBase.bslinc"
#include "$ENGINE$\PerCameraData.bslinc"

shader PPEncodeDepth
{
	mixin PPBase;
	mixin PerCameraData;

	blend
	{
		target
		{
			writemask = A;
		};
	};

	code
	{
		[internal]
		cbuffer Params
		{
			float gNear;
			float gFar;
		}	
	
		SamplerState gInputSamp;
		Texture2D gInputTex;
		
		float4 fsmain(VStoFS input) : SV_Target0
		{
			float deviceZ = gInputTex.Sample(gInputSamp, input.uv0);
			float viewZ = convertFromDeviceZ(deviceZ);
		
			float alpha = 1.0f - saturate((viewZ - gNear) / (gFar - gNear));
			return float4(0.0f, 0.0f, 0.0f, alpha);
		}	
	};
};
