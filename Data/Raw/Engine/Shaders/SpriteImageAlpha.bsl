#include "$ENGINE$\SpriteImage.bslinc"

Technique : inherits("SpriteImage") =
{
	Language = "HLSL11";
	
	Pass =
	{
		Target = 
		{
			Blend = true;
			Color = { SRCA, SRCIA, ADD };
			WriteMask = RGB;
		};
	};
};

Technique : inherits("SpriteImage") =
{
	Language = "HLSL9";
	
	Pass =
	{
		Target = 
		{
			Blend = true;
			Color = { SRCA, SRCIA, ADD };
			WriteMask = RGB;
		};
	};
};

Technique : inherits("SpriteImage") =
{
	Language = "GLSL";
	
	Pass =
	{
		Target = 
		{
			Blend = true;
			Color = { SRCA, SRCIA, ADD };
			WriteMask = RGB;
		};
	};
};