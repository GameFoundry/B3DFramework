#include "$EDITOR$/PickingCull.bslinc"

Technique : inherits("PickingCull") =
{
	Pass =
	{
		Cull = CW;
	};
};