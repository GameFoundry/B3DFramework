//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace BansheeEngine
{
    /// <summary>
    /// Animation keyframe, represented as an endpoint of a cubic hermite spline.
    /// </summary>
    [StructLayout(LayoutKind.Sequential), SerializeObject]
    public struct KeyFrame // Note: Must match C++ struct TKeyFrame<float>
    {
        /// <summary>
        /// Value of the key.
        /// </summary>
        public float value;
        /// <summary>
        /// Input tangent (going from the previous key to this one) of the key.
        /// </summary>
        public float inTangent;
        /// <summary>
        /// Output tangent (going from this key to next one) of the key.
        /// </summary>
        public float outTangent;
        /// <summary>
        /// Position of the key along the animation spline.
        /// </summary>
        public float time;
    }

    /// <summary>
    /// Animation spline represented by a set of keyframes, each representing an endpoint of a cubic hermite curve. The
    /// spline can be evaluated at any time, and uses caching to speed up multiple sequential evaluations.
    /// </summary>
    public class AnimationCurve : ScriptObject
    {
        /// <summary>
        /// Constructor for internal runtime use only.
        /// </summary>
        private AnimationCurve()
        { }

        /// <summary>
        /// Creates a new animation curve.
        /// </summary>
        /// <param name="keyFrames">Keyframes to initialize the curve with.</param>
        public AnimationCurve(KeyFrame[] keyFrames)
        {
            Internal_Create(this, keyFrames);
        }

        /// <summary>
        /// Keyframes that represent the curve.
        /// </summary>
        public KeyFrame[] KeyFrames
        {
            get { return Internal_GetKeyFrames(mCachedPtr); }
            set { Internal_SetKeyFrames(mCachedPtr, value); }
        }

        /// <summary>
        /// Evaluate the animation curve at the specified time.
        /// </summary>
        /// <param name="time">Time to evaluate the curve at. </param>
        /// <param name="loop">If true the curve will loop when it goes past the end or beggining. Otherwise the curve 
        ///                    value will be clamped.</param>
        /// <returns>Interpolated value from the curve at provided time.</returns>
        public float Evaluate(float time, bool loop = true)
        {
            return Internal_Evaluate(mCachedPtr, time, loop);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_Create(AnimationCurve instance, KeyFrame[] keyframes);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern KeyFrame[] Internal_GetKeyFrames(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetKeyFrames(IntPtr thisPtr, KeyFrame[] keyframes);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern float Internal_Evaluate(IntPtr thisPtr, float time, bool loop);
    }
    
    /// <summary>
    /// A set of animation curves for a 3D vector paired with a name.
    /// </summary>
    public class NamedVector3Curve
    {
        /// <summary>
        /// Constructs a new named animation curve.
        /// </summary>
        /// <param name="name">Name of the curve.</param>
        /// <param name="x">Curve representing the x axis of the vector.</param>
        /// <param name="y">Curve representing the y axis of the vector.</param>
        /// <param name="z">Curve representing the z axis of the vector.</param>
        public NamedVector3Curve(string name, AnimationCurve x, AnimationCurve y, AnimationCurve z)
        {
            Name = name;
            X = x;
            Y = y;
            Z = z;
        }

        /// <summary>
        /// Name of the curve.
        /// </summary>
        public string Name;

        /// <summary>
        /// Animation curve for the x axis.
        /// </summary>
        public AnimationCurve X;

        /// <summary>
        /// Animation curve for the y axis.
        /// </summary>
        public AnimationCurve Y;

        /// <summary>
        /// Animation curve for the z axis.
        /// </summary>
        public AnimationCurve Z;
    }

    /// <summary>
    /// An animatio curve for a single floating point value paired with a name.
    /// </summary>
    public class NamedFloatCurve
    {
        /// <summary>
        /// Constructs a new named animation curve.
        /// </summary>
        /// <param name="name">Name of the curve.</param>
        /// <param name="curve">Curve representing the floating point values.</param>
        public NamedFloatCurve(string name, AnimationCurve curve)
        {
            Name = name;
            Curve = curve;
        }

        /// <summary>
        /// Name of the curve.
        /// </summary>
        public string Name;

        /// <summary>
        /// Animation curve.
        /// </summary>
        public AnimationCurve Curve;
    }
}
