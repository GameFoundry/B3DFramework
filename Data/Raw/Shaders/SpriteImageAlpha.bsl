#include "$ENGINE$\SpriteImage.bslinc"

shader SpriteImageAlpha
{
	mixin SpriteImage;

	blend
	{
		target	
		{
			enabled = true;
			color = { srcA, srcIA, add };
			writemask = RGB;
		};
	};
};