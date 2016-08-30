#include "$EDITOR$/LineGizmo.bslinc"

Technique : inherits("LineGizmo") =
{
	Language = "HLSL11";
	
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

Technique : inherits("LineGizmo") =
{
	Language = "HLSL9";
	
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

Technique : inherits("LineGizmo") =
{
	Language = "GLSL";
	
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
