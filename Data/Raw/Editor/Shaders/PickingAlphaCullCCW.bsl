#include "$EDITOR$/PickingAlphaCull.bslinc"

Technique : inherits("PickingAlphaCull") =
{
	Pass =
	{
		Cull = CCW;
	};
};