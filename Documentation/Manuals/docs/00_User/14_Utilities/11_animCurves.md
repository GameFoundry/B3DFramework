Animation curves				{#animCurves}
===============

Animation curves represent a set of values placed along a timeline that can be smoothly interpolated. They are most often used by the animation system, as well as the particle system (for setting up time-varying properties). The curve is represented with a template @ref bs::TAnimationCurve<T> "TAnimationCurve<T>" class which can be used with a **float**, **Vector2**, **Vector3** or a **Quaternion** parameter.

Entries in the curve are represented with the @ref bs::TKeyframe "TKeyFrame<T>" which can be used with the same parameter types. Each keyframe consists of four values:
 - Time - Time at which to place the keyframe
 - Value - Value of the keyframe
 - In tangent - Tangent that controls the shape of the curve to the left of the keyframe
 - Out tangent - Tangent that controls the shape of the curve to the right of the keyframe
 
The keyframes are used to generate cubic hermite curves, which are connected together in a spline which can then be evaluated. Time and value entries are the most important ones for determining the shape of the spline, while tangent values allow to finer control over the final curve.

~~~~~~~~~~~~~{.cpp}
// Curve that goes up from 0 to 1, then back to 0
TAnimationCurve<float> curve(
{ 
	TKeyframe<float>{ 0.0f, 0.0f, 2.0f, 0.0f },
	TKeyframe<float>{ 1.0f, 0.0f, 0.0f, 0.5f },
	TKeyframe<float>{ 0.0f, 2.0f, 0.0f, 1.0f }
});
~~~~~~~~~~~~~

If you are unsure how to set them, or don't care about tangents you can use @ref bs::AnimationUtility::calculateTangents "AnimationUtility::calculateTangents()" method to generate them automatically.

~~~~~~~~~~~~~{.cpp}
// All tangents set to 0, then auto-calculating them
Vector<TKeyframe<float>> keyframes = { 
	TKeyframe<float>{ 0.0f, 0.0f, 0.0f, 0.0f },
	TKeyframe<float>{ 1.0f, 0.0f, 0.0f, 0.5f },
	TKeyframe<float>{ 0.0f, 0.0f, 0.0f, 1.0f }
};

AnimationUtility::calculateTangents(keyframes);
TAnimationCurve<float> curve(keyframes);
~~~~~~~~~~~~~

Note that keyframes provided to the curve must be in ascending order with regards to time. Once the curve is created it is immutable and keyframes can no longer be changed.

To evaluate a curve call @ref bs::TAnimationCurve<T>::evaluate "TAnimationCurve<T>::evaluate()".

~~~~~~~~~~~~~{.cpp}
float val = curve.evaluate(0.25f);
~~~~~~~~~~~~~

# More about tangents
In case you want to set tangents manually there are a few things to be aware of:
 - The tangents represent a slope (same as in your basic line equation)
 - Value of 0 points the tangent horizontal, positive values upwards, negative values downwards
 - Only output tangent is relevant for the first keyframe
 - Only input tangent is relevant for the last keyframe
 - Tangents have to be normalized. This means they should be divided by the time between the keyframes. For first keyframe this is the time between first and second keyframe, for last keyframe this is the time between last and next-to-last keyframe, and for every other keyframe it is the time between previous and following keyframe.
 - For smooth curves both in and out tangents should have the same value. Using different values creates sharp edges in the curve, at the point of the keyframe.
 - Setting either tangent to positive infinity creates a 'step' at that point, meaning the evaluated values will jump from previous keyframe to the current keyframe value with no interpolated values in-between 