#include "$EDITOR$\SelectionBase.bslinc"

#define USE_SKELETON
#include "$EDITOR$\SelectionBase.bslinc"
#undef USE_SKELETON

#define USE_BLEND_SHAPES
#include "$EDITOR$\SelectionBase.bslinc"

#define USE_SKELETON
#include "$EDITOR$\SelectionBase.bslinc"

Technique 
 : inherits("Selection") =
{ };

Technique 
 : inherits("SelectionSkinned") =
{
	Tags = { "Skinned" };
};

Technique 
 : inherits("SelectionMorph") =
{
	Tags = { "Morph" };
};

Technique 
 : inherits("SelectionSkinnedMorph") =
{
	Tags = { "SkinnedMorph" };
};