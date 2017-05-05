#include "$EDITOR$/LineGizmo.bslinc"

Technique : inherits("LineGizmo") =
{
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