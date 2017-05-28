#include "$EDITOR$/LineGizmo.bslinc"

technique LineHandle
{
	mixin LineGizmoBase;

	depth
	{
		write = false;
		read = false;
	};
	
	stencil
	{
		enabled = true;
		front = { keep, keep, inc, always };
	};
	
	raster
	{
		multisample = false;
		lineaa = true;
	};
	
	blend
	{
		target
		{
			enabled = true;
			color = { srcA, srcIA, add };
		};
	};
};