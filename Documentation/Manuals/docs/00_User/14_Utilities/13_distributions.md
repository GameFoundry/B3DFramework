Distributions				{#distributions}
===============

Distributions can be used for describing properties that can either take on a constant value, a range of values, or vary their value by time. They are most commonly used by properties in the particle system.

In particular all types of distributions can take on the following values:
 - A constant value - This is what we have done in all the examples so far.
 - A range of values - The system will then pick actual values randomly in the specified range.
 - A time-varying curve - The system will sample a value on the curve depending on how long is the particle system running
 - Dual curves - Similar to a single curve, except both curves will be sampled and a random value will then be chosen in the range determined by the sampled values

There are different types of distributions:
 - @ref bs::TDistribution<T> "FloatDistribution" - Distribution that outputs a single floating point number. Curves are represented using a **TAnimationCurve<float>**, as described in the [animation curves](@ref animCurves) manual.
 - @ref bs::TDistribution<T> "Vector2Distribution" - Distribution that outputs a 2D vector. Curves are represented using a **TAnimationCurve<Vector2>**, as described in the [animation curves](@ref animCurves) manual.
 - @ref bs::TDistribution<T> "Vector3Distribution" - Distribution that outputs a 3D vector. Curves are represented using a **TAnimationCurve<Vector3>**, as described in the [animation curves](@ref animCurves) manual.
 - @ref bs::ColorDistribution "ColorDistribution" - Distribution that outputs a color. Curves are represented using a **ColorGradient**, as described in the [color gradient](@ref colorGradient) manual.

~~~~~~~~~~~~~{.cpp}
// 'Distribution' that always evaluates to a constant value
FloatDistribution constantFloat(50.0f);
Vector3Distribution constantVec3(Vector3(0.0f, 5.0f, 1.0f));
ColorDistribution constantColor(Color::Red);

// Distribution that returns a random value in some range
FloatDistribution rangeFloat(10.0f, 20.0f);
Vector3Distribution rangeVec3(Vector3(0.0f, 0.0f, 0.0f), Vector3(1.0f, 1.0f, 1.0f));
ColorDistribution rangeColor(Color::Black, Color::Red);

// Distribution that returns a different value depending on some parameter 't'
FloatDistribution curveFloat(TAnimationCurve<float>(
{
	TKeyframe<float>{1.0f, 0.0f, 1.0f, 0.0f},
	TKeyframe<float>{4.0f, 1.0f, 0.0f, 1.0f},
}));
ColorDistribution curveColor(ColorGradient(
{
	ColorGradientKey(Color::White, 0.0f),
	ColorGradientKey(Color(0.1f, 0.1f, 0.1f, 1.0f), 1.0f)
}));

// Distribution that returns a different range depending on some parameter 't',
// and then picks a random number in that range
FloatDistribution curveRangeFloat(TAnimationCurve<float>(
{
	TKeyframe<float>{0.0f, 0.0f, 1.0f, 0.0f},
	TKeyframe<float>{3.0f, 1.0f, 0.0f, 1.0f},
}),
TAnimationCurve<float>(
{
	TKeyframe<float>{2.0f, 0.0f, 1.0f, 0.0f},
	TKeyframe<float>{4.0f, 1.0f, 0.0f, 1.0f},
}));
ColorDistribution curveRangeColor(ColorGradient(
{
	ColorGradientKey(Color(0.0f, 0.0f, 0.0f, 1.0f), 0.0f),
	ColorGradientKey(Color(0.5f, 0.5f, 0.5f, 1.0f), 1.0f)
}),
ColorGradient(
{
	ColorGradientKey(Color(0.1f, 0.1f, 0.1f, 1.0f), 0.0f),
	ColorGradientKey(Color(0.6f, 0.6f, 0.6f, 1.0f), 1.0f)
}));
~~~~~~~~~~~~~

All distribution types can be evaluated by calling their **evaluate** method, which accepts two parameters:
 - 't' - Time to evaluate the curve at. Irrelevant for non-curve distributions.
 - 'factor' - A random factor that is used to pick a value in specified random range. Can be a float in [0, 1] range, or a **Random** object that automatically generates a random factor. Irrelevant for distributions not representing a range.
 
~~~~~~~~~~~~~{.cpp}
Random rng;

// Evaluate half-way along the curve, and use a random number to evaluate the range
float valueFloat = curveRangeFloat.evaluate(0.5f, rng);
RGBA valueColor = curveRangeColor(0.5f, rng);
~~~~~~~~~~~~~

Normally you will not need to evaluate distributions manually, but rather pass them along to other systems such as the particle system.