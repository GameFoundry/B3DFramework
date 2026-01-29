#define USES_GS
#include "$ENGINE$\ShadowDepthBase.bslinc"

shader ShadowDepth
{ 
	mixin ShadowDepthBase;
	
	code
	{
		struct GSToPS
		{
			float4 position : SV_Position;
			uint targetIdx : SV_RenderTargetArrayIndex;
		};

		cbuffer ShadowCubeMatrices
		{
			float4x4 gFaceVPMatrices[6];
		};
		
		cbuffer ShadowCubeMasks
		{
			uint gFaceMasks[6];
		};
		
		[maxvertexcount(18)]
		void gsmain(triangle ShadowVStoFS inputs[3], inout TriangleStream<GSToPS> outStream)
		{
			// Output a triangle to all relevant faces
			[unroll]
			for (int faceIdx = 0; faceIdx < 6; faceIdx++)
			{
				// Check the per-object masks that were determined based on CPU frustum culling
				[branch]
				if (gFaceMasks[faceIdx] > 0)
				{
					float4 clipPos[3];
					
					[unroll]
					for (int vertIdx = 0; vertIdx < 3; vertIdx++)
						clipPos[vertIdx] = mul(gFaceVPMatrices[faceIdx], inputs[vertIdx].worldPos);

					// Per-triangle frustum culling
					// Note: Test if this helps or hurts performance
					float4 testMask = saturate(clipPos[0].xyxy * float4(-1, -1, 1, 1) - clipPos[0].w);
					testMask *= saturate(clipPos[1].xyxy * float4(-1, -1, 1, 1) - clipPos[1].w);
					testMask *= saturate(clipPos[2].xyxy * float4(-1, -1, 1, 1) - clipPos[2].w);

					[branch]	
					if (all(testMask == 0))
					{				
						GSToPS output;
						output.targetIdx = faceIdx;

						// Note: I'm reversing the order here because otherwise the triangle order is reversed
						// (not sure why, but GS seems like it could be the only culprit). This doesn't happen in
						// OpenGL, because we flip the projection matrix Y axis due to upside-down UV coordinates
						[unroll]
						#ifdef OPENGL
						for (int vertIdx = 0; vertIdx < 3; vertIdx++)
						#else
						for (int vertIdx = 2; vertIdx >= 0; vertIdx--)
						#endif
						{
							output.position = clipPos[vertIdx];
							outStream.Append(output);
						}
						
						outStream.RestartStrip();
					}
				}
			}
		}
	};
};
