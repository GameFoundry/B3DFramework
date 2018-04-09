shader DebugDraw
{
	variations
	{
		LINE = { 0, 1 };
		WIRE = { 0, 1 };
		SOLID = { 0, 1 };
	};

	#if LINE
	raster
	{
		multisample = false; // This controls line rendering algorithm
		lineaa = true;
	};

	blend
	{
		target
		{
			enabled = true;
			color = { srcA, srcIA, add };
		};
	};
	#endif
	
	#if WIRE
	raster
	{
		fill = wire;
	};
	#endif
	
	code
	{
		cbuffer Params
		{
			float4x4 	gMatViewProj;
			float4		gViewDir;
		}
		
		#if LINE || WIRE
		void vsmain(
			in float3 inPos : POSITION,
			in float4 color : COLOR0,
			out float4 oPosition : SV_Position,
			out float4 oColor : COLOR0)
		{
			oPosition = mul(gMatViewProj, float4(inPos.xyz, 1));
			oColor = color;
		}

		float4 fsmain(in float4 inPos : SV_Position, in float4 color : COLOR0) : SV_Target
		{
			return color;
		}
		#else
		void vsmain(
			in float3 inPos : POSITION,
			in float3 inNormal : NORMAL,
			in float4 color : COLOR0,
			out float4 oPosition : SV_Position,
			out float3 oNormal : NORMAL,
			out float4 oColor : COLOR0)
		{
			oPosition = mul(gMatViewProj, float4(inPos.xyz, 1));
			oNormal = inNormal;
			oColor = color;
		}
		float4 fsmain(in float4 inPos : SV_Position, in float3 normal : NORMAL, in float4 color : COLOR0) : SV_Target
		{
			float4 outColor = color * dot(normalize(normal), -gViewDir);
			outColor.a = color.a;
			
			return outColor;
		}
		#endif
	};
};
