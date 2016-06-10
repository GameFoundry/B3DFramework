#include "$EDITOR$/PickingAlphaCull.bslinc"

Technique =
{
	Language = "HLSL11";
	
	Pass =
	{
		Cull = NOCULL;
	};
};

Technique =
{
	Language = "HLSL9";
	
	Pass =
	{
		Cull = NOCULL;
	};
};

Technique =
{
	Language = "GLSL";
	
	Pass =
	{
		Cull = NOCULL;
	};
};