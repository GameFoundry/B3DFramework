#include "$EDITOR$/LineGizmo.bslinc"

Technique =
{
	Language = "HLSL11";
	
	Pass =
	{
		DepthWrite = false;
		DepthRead = false;
		Stencil = true;
		StencilOpFront = { KEEP, KEEP, INC, PASS };
		Multisample = false; // This controls line rendering algorithm
		AALine = true;
	
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
		Multisample = false; // This controls line rendering algorithm
		AALine = true;
		
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
		StencilOpFront = { KEEP, KEEP, INC, PASS };
		Multisample = false; // This controls line rendering algorithm
		AALine = true;
		
		Target =
		{
			Blend = true;
			Color = { SRCA, SRCIA, ADD };
		};
	};
};
