#include "$EDITOR$/SolidGizmo.bslinc"
Priority = 10;

Technique =
{
	Language = "HLSL11";
	
	Pass =
	{
		DepthWrite = false;
		DepthRead = false;
		Stencil = true;
		StencilOpFront = { KEEP, KEEP, INC, PASS };
		
		Target =
		{
			Blend = true;
			Color = { SRCA, SRCIA, ADD };
		};
	};
};

Technique =
{
	Language = "HLSL9";
	
	Pass =
	{
		DepthWrite = false;
		DepthRead = false;
		Stencil = true;
		StencilOpFront = { KEEP, KEEP, INC, PASS };
		
		Target =
		{
			Blend = true;
			Color = { SRCA, SRCIA, ADD };
		};
	};
};

Technique =
{
	Language = "GLSL";
	
	Pass =
	{
		DepthWrite = false;
		DepthRead = false;
		Stencil = true;
		StencilRef = 1;
		StencilOpBack = { KEEP, KEEP, INC, PASS };
		
		Target =
		{
			Blend = true;
			Color = { SRCA, SRCIA, ADD };
		};
	};
};
