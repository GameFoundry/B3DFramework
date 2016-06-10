#include "$EDITOR$/LineGizmo.bslinc"

Technique =
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

Technique =
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

Technique =
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
