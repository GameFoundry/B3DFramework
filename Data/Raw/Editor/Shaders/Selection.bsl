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
{
	Language = "HLSL11";
};

Technique 
 : inherits("SelectionSkinned") =
{
	Language = "HLSL11";
	Tags = { "Skinned" };
};

Technique 
 : inherits("SelectionMorph") =
{
	Language = "HLSL11";
	Tags = { "Morph" };
};

Technique 
 : inherits("SelectionSkinnedMorph") =
{
	Language = "HLSL11";
	Tags = { "SkinnedMorph" };
};

Technique 
 : inherits("Selection") =
{
	Language = "GLSL";
};

Technique 
 : inherits("SelectionSkinned") =
{
	Language = "GLSL";
	Tags = { "Skinned" };
};

Technique 
 : inherits("SelectionMorph") =
{
	Language = "GLSL";
	Tags = { "Morph" };
};

Technique 
 : inherits("SelectionSkinnedMorph") =
{
	Language = "GLSL";
	Tags = { "SkinnedMorph" };
};