#include "$EDITOR$/SolidGizmo.bslinc"
Priority = 10;

Technique : inherits("SolidGizmo") =
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

Technique : inherits("SolidGizmo") =
{
	Language = "GLSL";
	
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
