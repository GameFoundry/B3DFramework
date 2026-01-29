#include "$ENGINE$/GpuParticleTileVertex.bslinc"

shader GpuParticleClear
{
	mixin GpuParticleTileVertex;
	
	code
	{	
		void fsmain(VStoFS input, 
			out float4 outPosAndTime : SV_Target0, 
			out float4 outVel : SV_Target1,
			out float4 outSizeAndRotation : SV_Target2)
		{
			// Time > 1.0f means the particle is dead
			float time = 10.0f;
		
			outPosAndTime.xyz = float3(1.0f, 0.0f, 0.0f);
			outPosAndTime.w = time;
			
			outVel = 0.0f;
			
			outSizeAndRotation = 0.0f;
		}
	};
};
