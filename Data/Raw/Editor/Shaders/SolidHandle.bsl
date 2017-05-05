#include "$EDITOR$/SolidGizmo.bslinc"
Priority = 10;

Technique : inherits("SolidGizmo") =
{
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