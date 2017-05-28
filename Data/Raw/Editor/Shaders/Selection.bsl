#include "$EDITOR$\SelectionBase.bslinc"

#define USE_SKELETON
#include "$EDITOR$\SelectionBase.bslinc"
#undef USE_SKELETON

#define USE_BLEND_SHAPES
#include "$EDITOR$\SelectionBase.bslinc"

#define USE_SKELETON
#include "$EDITOR$\SelectionBase.bslinc"

technique Selection1
{ 
	mixin Selection;
};

technique Selection2
{
	mixin SelectionSkinned;

	tags = { "Skinned" };
};

technique Selection3
{
	mixin SelectionMorph;

	tags = { "Morph" };
};

technique Selection4
{
	mixin SelectionSkinnedMorph;

	tags = { "SkinnedMorph" };
};