#include "$EDITOR$/PickingCull.bslinc"

Technique =
{
	Language = "HLSL11";
	
	Pass =
	{
		Cull = CW;
	};
};

Technique =
{
	Language = "HLSL9";
	
	Pass =
	{
		Cull = CW;
	};
};

Technique =
{
	Language = "GLSL";
	
	Pass =
	{
		Cull = CW;
	};
};