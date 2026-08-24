// Renderer-material twin of Decal.bsl: the fallback material the renderer uses for decals that have no material
// assigned. Kept as a separate shader so the renderer-material and builtin-shader caches each have their own entry.
#include "$ENGINE$\DecalBase.bslinc"

shader Surface
{
	mixin DecalBase;
};
