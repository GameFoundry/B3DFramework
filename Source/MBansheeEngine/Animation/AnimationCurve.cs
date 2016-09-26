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
    /// Flags that describe an <see cref="AnimationCurve"/>
    /// </summary>
    public enum AnimationCurveFlags // Note: Must match C++ enum AnimationCurveFlags
    {
        /// <summary>
        /// If enabled, the curve was imported from an external file and not created within the engine. This will affect
        /// how are animation results applied to scene objects (with imported animations it is assumed the curve is
        /// animating bones and with in-engine curves it is assumed the curve is animating scene objects).
        /// </summary>
        ImportedCurve = 1 << 0,
        /// <summary>
        /// Signifies the curve is used to animate between different frames within a morph channel. In range [0, 1].
        /// </summary>
        MorphFrame = 1 << 1,
        /// <summary>
        /// Signifies the curve is used to adjust the weight of a morph channel. In range [0, 1].
        /// </summary>
        MorphWeight = 1 << 2
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
        /// Constructor for internal runtime use only.
        /// </summary>
        /// <param name="name">Name of the curve.</param>
        /// <param name="flags">Flags that describe the animation curve, of type <see cref="AnimationCurveFlags"/>.</param>
        /// <param name="x">Curve representing the x axis of the vector.</param>
        /// <param name="y">Curve representing the y axis of the vector.</param>
        /// <param name="z">Curve representing the z axis of the vector.</param>
        private NamedVector3Curve(string name, int flags, AnimationCurve x, AnimationCurve y, AnimationCurve z)
        {
            Name = name;
            Flags = (AnimationCurveFlags) flags;
            X = x;
            Y = y;
            Z = z;
        }

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
        /// Flags that describe the animation curve.
        /// </summary>
        public AnimationCurveFlags Flags;

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
    /// An animation curve for a single floating point value paired with a name.
    /// </summary>
    public class NamedFloatCurve
    {
        /// <summary>
        /// Constructor for internal runtime use only.
        /// </summary>
        /// <param name="name">Name of the curve.</param>
        /// <param name="flags">Flags that describe the animation curve, of type <see cref="AnimationCurveFlags"/>.</param>
        /// <param name="curve">Curve representing the floating point values.</param>
        private NamedFloatCurve(string name, int flags, AnimationCurve curve)
        {
            Name = name;
            Flags = (AnimationCurveFlags)flags;
            Curve = curve;
        }

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
        /// Flags that describe the animation curve.
        /// </summary>
        public AnimationCurveFlags Flags;

        /// <summary>
        /// Animation curve.
        /// </summary>
        public AnimationCurve Curve;
    }

    /// <summary>
    /// A set of animation curves for a 3D vector.
    /// </summary>
    public class Vector3Curve
    {
        /// <summary>
        /// Constructs a new 3D vector animation curve.
        /// </summary>
        /// <param name="x">Curve representing the x axis of the vector.</param>
        /// <param name="y">Curve representing the y axis of the vector.</param>
        /// <param name="z">Curve representing the z axis of the vector.</param>
        public Vector3Curve(AnimationCurve x, AnimationCurve y, AnimationCurve z)
        {
            X = x;
            Y = y;
            Z = z;
        }

        /// <summary>
        /// Evaluate the animation curve at the specified time.
        /// </summary>
        /// <param name="time">Time to evaluate the curve at. </param>
        /// <param name="loop">If true the curve will loop when it goes past the end or beggining. Otherwise the curve 
        ///                    value will be clamped.</param>
        /// <returns>Interpolated value from the curve at provided time.</returns>
        public Vector3 Evaluate(float time, bool loop = true)
        {
            Vector3 output = new Vector3();

            if (X != null)
                output.x = X.Evaluate(time, loop);

            if (Y != null)
                output.y = Y.Evaluate(time, loop);

            if (Z != null)
                output.z = Z.Evaluate(time, loop);

            return output;
        }

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
    /// A set of animation curves for a quaternion.
    /// </summary>
    public class QuaternionCurve
    {
        /// <summary>
        /// Constructs a new quaternion animation curve.
        /// </summary>
        /// <param name="x">Curve representing the x component of the quaternion.</param>
        /// <param name="y">Curve representing the y component of the quaternion.</param>
        /// <param name="z">Curve representing the z component of the quaternion.</param>
        /// <param name="w">Curve representing the w component of the quaternion.</param>
        public QuaternionCurve(AnimationCurve x, AnimationCurve y, AnimationCurve z, AnimationCurve w)
        {
            X = x;
            Y = y;
            Z = z;
            W = w;
        }

        /// <summary>
        /// Evaluate the animation curve at the specified time.
        /// </summary>
        /// <param name="time">Time to evaluate the curve at. </param>
        /// <param name="loop">If true the curve will loop when it goes past the end or beggining. Otherwise the curve 
        ///                    value will be clamped.</param>
        /// <returns>Interpolated value from the curve at provided time.</returns>
        public Quaternion Evaluate(float time, bool loop = true)
        {
            Quaternion output = new Quaternion();

            if (X != null)
                output.x = X.Evaluate(time, loop);

            if (Y != null)
                output.y = Y.Evaluate(time, loop);

            if (Z != null)
                output.z = Z.Evaluate(time, loop);

            if (W != null)
                output.w = W.Evaluate(time, loop);

            return output;
        }

        /// <summary>
        /// Animation curve for the x component.
        /// </summary>
        public AnimationCurve X;

        /// <summary>
        /// Animation curve for the y component.
        /// </summary>
        public AnimationCurve Y;

        /// <summary>
        /// Animation curve for the z component.
        /// </summary>
        public AnimationCurve Z;

        /// <summary>
        /// Animation curve for the w component.
        /// </summary>
        public AnimationCurve W;
    }
}
