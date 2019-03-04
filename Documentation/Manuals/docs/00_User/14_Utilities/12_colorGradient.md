Color gradient				{#colorGradient}
===============

Color gradient allows you to interpolate between multiple colors placed along a line at specified intervals. It is represented with a @ref bs::ColorGradient "ColorGradient" class. It can be initialized with 1 or multiple (up to 8) @ref bs::ColorGradientKey "ColorGradientKey" values, each containing a single **Color** and a time value.

~~~~~~~~~~~~~{.cpp}
// Color gradient that moves from white, to red, to green
ColorGradient gradient({ 
	ColorGradientKey(Color::White, 0.0f),
	ColorGradientKey(Color::Red, 0.5f),
	ColorGradientKey(Color::Green, 1.0f),
});
~~~~~~~~~~~~~

Once created you can manually evaluate the gradient by calling @ref bs::ColorGradient::evaluate "ColorGradient::evaluate()" with a value of @p t specifying a point at which to sample the gradient. The returned value is a color encoded in a 32-bit format.

~~~~~~~~~~~~~{.cpp}
// Evaluates a gradient yielding a color half way between red and green
RGBA color32 = gradient.evaluate(0.75f);

// Optionally, convert to Color
Color color = Color::fromRGBA(color32);
~~~~~~~~~~~~~

In most cases you will not be evaluating the gradient yourself, but rather passing it to various systems, such as the **Material** or **ParticleEmitter**.