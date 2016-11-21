#include "$EDITOR$/PickingAlphaCull.bslinc"

Technique : inherits("PickingAlphaCull") =
{
	Language = "HLSL11";
	
	Pass =
	{
		Cull = CCW;
	};
};

Technique : inherits("PickingAlphaCull") =
{
	Language = "GLSL";
	
	Pass =
	{
		Cull = CCW;
	};
};