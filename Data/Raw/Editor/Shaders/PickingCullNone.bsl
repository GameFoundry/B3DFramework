#include "$EDITOR$/PickingCull.bslinc"

Technique : inherits("PickingCull") =
{
	Language = "HLSL11";
	
	Pass =
	{
		Cull = NOCULL;
	};
};

Technique : inherits("PickingCull") =
{
	Language = "HLSL9";
	
	Pass =
	{
		Cull = NOCULL;
	};
};

Technique : inherits("PickingCull") =
{
	Language = "GLSL";
	
	Pass =
	{
		Cull = NOCULL;
	};
};