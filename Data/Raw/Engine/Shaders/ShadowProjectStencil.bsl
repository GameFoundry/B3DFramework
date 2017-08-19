#include "$ENGINE$/ShadowProjectionCommon.bslinc"

technique ShadowProjectStencil
{
	mixin ShadowProjectionCommon;

	depth
	{
		write = false;
	};
	
	blend
	{
		target
		{
			writemask = empty;
		};
	};	
	
	raster
	{
		cull = none;
	};
	
	#ifdef USE_ZFAIL_STENCIL
	stencil
	{
		enabled = true;
		front = { keep, incwrap, keep, always };
		back = { keep, decwrap, keep, always };
		writemask = 0x7F;
	};
	#else
	stencil
	{
		enabled = true;
		front = { keep, keep, incwrap, always };
		back = { keep, keep, decwrap, always };
		writemask = 0x7F;
	};	
	#endif
};