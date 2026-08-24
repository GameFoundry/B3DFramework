// Renderer-material twin of ParticlesUnlit.bsl: the fallback material the renderer uses for particle systems that
// have no material assigned. Kept as a separate shader so the renderer-material and builtin-shader caches each have
// their own entry.
#include "$ENGINE$\ParticlesUnlitBase.bslinc"

options
{
	transparent = true;
};

shader Surface
{
	mixin ParticlesUnlitBase;
};
