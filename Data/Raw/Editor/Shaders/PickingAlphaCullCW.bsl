#include "$EDITOR$/PickingAlphaCull.bslinc"

Technique : inherits("PickingAlphaCull") =
{
	Language = "HLSL11";
	
	Pass =
	{
		Cull = CW;
	};
};

Technique : inherits("PickingAlphaCull") =
{
	Language = "HLSL9";
	
	Pass =
	{
		Cull = CW;
	};
};

Technique : inherits("PickingAlphaCull") =
{
	Language = "GLSL";
	
	Pass =
	{
		Cull = CW;
	};
};