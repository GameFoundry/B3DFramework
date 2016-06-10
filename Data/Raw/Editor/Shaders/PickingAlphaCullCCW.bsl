#include "$EDITOR$/PickingAlphaCull.bslinc"

Technique =
{
	Language = "HLSL11";
	
	Pass =
	{
		Cull = CCW;
	};
};

Technique =
{
	Language = "HLSL9";
	
	Pass =
	{
		Cull = CCW;
	};
};

Technique =
{
	Language = "GLSL";
	
	Pass =
	{
		Cull = CCW;
	};
};