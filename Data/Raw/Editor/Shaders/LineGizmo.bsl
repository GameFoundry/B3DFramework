#include "$EDITOR$/LineGizmo.bslinc"

Technique : inherits("LineGizmo") =
{
	Pass =
	{
		Multisample = false; // This controls line rendering algorithm
		AALine = true;
		
		Target =
		{
			Blend = true;
			Color = { SRCA, SRCIA, ADD };
		};
	};
};
