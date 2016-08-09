//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;
using BansheeEngine;

namespace BansheeEditor
{
    /** @addtogroup AnimationEditor
     *  @{
     */

    /// <summary>
    /// Type of tangent on a keyframe in an animation curve.
    /// </summary>
    internal enum TangentType
    {
        In = 1 << 0,
        Out = 1 << 1
    }

    /// <summary>
    /// Flags that are used for describing how are tangents calculated for a specific keyframe in an animation curve. 
    /// Modes for "in" and "out" tangents can be combined.
    /// </summary>
    [Flags]
    internal enum TangentMode
    {
        /// <summary>
        /// Both tangents are calculated automatically based on the two surrounding keyframes.
        /// </summary>
        Auto = 0,
        /// <summary>
        /// Left tangent is calculated automatically based on the two surrounding keyframes.
        /// </summary>
        InAuto = TangentType.In | 1 << 2,
        /// <summary>
        /// Left tangent is manually adjusted by the user.
        /// </summary>
        InFree = TangentType.In | 1 << 3,
        /// <summary>
        /// Tangent is calculated automatically based on the previous keyframe.
        /// </summary>
        InLinear = TangentType.In | 1 << 4,
        /// <summary>
        /// Tangent is infinite, ensuring there is a instantaneus jump between previous and current keyframe value.
        /// </summary>
        InStep = TangentType.In | 1 << 5,
        /// <summary>
        /// Right tangents are calculated automatically based on the two surrounding keyframes.
        /// </summary>
        OutAuto = TangentType.Out | 1 << 6,
        /// <summary>
        /// Right tangent is manually adjusted by the user.
        /// </summary>
        OutFree = TangentType.Out | 1 << 7,
        /// <summary>
        /// Tangent is calculated automatically based on the next keyframe.
        /// </summary>
        OutLinear = TangentType.Out | 1 << 8,
        /// <summary>
        /// Tangent is infinite, ensuring there is a instantaneus jump between current and next keyframe value.
        /// </summary>
        OutStep = TangentType.Out | 1 << 9,
        /// <summary>
        /// Both tangents are manually adjusted by the user.
        /// </summary>
        Free = 1 << 10,
    }

    /// <summary>
    /// <see cref="AnimationCurve"/> wrapper for use in editor only. Allows easier manipulation of animation keyframes, and
    /// also stores keyframe tangent modes which are not required for non-editor curves.
    /// </summary>
    internal class EdAnimationCurve
    {
        private AnimationCurve native;

        private KeyFrame[] keyFrames;
        private TangentMode[] tangentModes;

        /// <summary>
        /// Returns tangent modes for each keyframe. Array is guaranteed to be the same size as <see cref="KeyFrames"/>.
        /// If modifying the array values, make sure to call <see cref="Apply"/> to save the changes on the curve.
        /// </summary>
        public TangentMode[] TangentModes
        {
            get { return tangentModes; }
        }

        /// <summary>
        /// All keyframes belonging to the animation curve. If modifying the keyframe values, make sure to call 
        /// <see cref="Apply"/> to save the changes on the curve.
        /// </summary>
        public KeyFrame[] KeyFrames
        {
            get { return keyFrames; }
        }

        /// <summary>
        /// Creates a new animation curve with zero keyframes.
        /// </summary>
        internal EdAnimationCurve()
        {
            keyFrames = new KeyFrame[0];
            native = new AnimationCurve(keyFrames);

            tangentModes = new TangentMode[0];
        }

        /// <summary>
        /// Creates a new editor animation curve using an existing animation curve as a basis.
        /// </summary>
        /// <param name="native">Animation curve to retrieve the keyframes from.</param>
        /// <param name="tangentModes">A set of tangent modes for each keyframe. Should be the same size as the number
        ///                            of keyframes in the provided animation. Can be null in which case all keyframes will
        ///                            have tangents set to automatic.</param>
        internal EdAnimationCurve(AnimationCurve native, TangentMode[] tangentModes)
        {
            this.native = native;

            keyFrames = native.KeyFrames;

            this.tangentModes = new TangentMode[keyFrames.Length];
            if (tangentModes != null)
            {
                int numTangents = Math.Min(keyFrames.Length, tangentModes.Length);
                Array.Copy(tangentModes, this.tangentModes, numTangents);
            }

            Apply();
        }

        /// <summary>
        /// Evaluate the animation curve at the specified time.
        /// </summary>
        /// <param name="time">Time to evaluate the curve at. </param>
        /// <param name="loop">If true the curve will loop when it goes past the end or beggining. Otherwise the curve 
        ///                    value will be clamped.</param>
        /// <returns>Interpolated value from the curve at provided time.</returns>
        internal float Evaluate(float time, bool loop = true)
        {
            return native.Evaluate(time, loop);
        }

        /// <summary>
        /// Adds a new keyframe to the animation curve. Keyframe will use the automatic tangent mode.
        /// </summary>
        /// <param name="time">Time at which to add the keyframe.</param>
        /// <param name="value">Value of the keyframe.</param>
        internal void AddKeyframe(float time, float value)
        {
            AddKeyframe(time, value, TangentMode.Auto);
        }

        /// <summary>
        /// Adds a new keyframe to the animation curve.
        /// </summary>
        /// <param name="time">Time at which to add the keyframe.</param>
        /// <param name="value">Value of the keyframe.</param>
        /// <param name="tangentMode">Tangent mode of the keyframe.</param>
        internal void AddKeyframe(float time, float value, TangentMode tangentMode)
        {
            KeyFrame[] newKeyFrames = new KeyFrame[keyFrames.Length + 1];
            newKeyFrames[newKeyFrames.Length - 1].time = float.PositiveInfinity;

            TangentMode[] newTangentModes = new TangentMode[tangentModes.Length + 1];

            int insertIdx = keyFrames.Length;
            for (int i = 0; i < keyFrames.Length; i++)
            {
                if (time < keyFrames[i].time)
                {
                    insertIdx = i;
                    break;
                }
            }

            Array.Copy(keyFrames, newKeyFrames, insertIdx);
            Array.Copy(tangentModes, newTangentModes, insertIdx);

            KeyFrame keyFrame = new KeyFrame();
            keyFrame.time = time;
            keyFrame.value = value;

            newKeyFrames[insertIdx] = keyFrame;
            newTangentModes[insertIdx] = tangentMode;

            if (insertIdx < keyFrames.Length)
            {
                int remaining = keyFrames.Length - insertIdx;
                Array.Copy(keyFrames, insertIdx, newKeyFrames, insertIdx + 1, remaining);
                Array.Copy(tangentModes, insertIdx, newTangentModes, insertIdx + 1, remaining);
            }

            tangentModes = newTangentModes;
            keyFrames = newKeyFrames;
        }

        /// <summary>
        /// Removes a keyframe at the specified index.
        /// </summary>
        /// <param name="index">Index of the keyframe, referencing the <see cref="KeyFrames"/> array.</param>
        internal void RemoveKeyframe(int index)
        {
            if (index < 0 || index >= KeyFrames.Length)
                return;

            KeyFrame[] newKeyFrames = new KeyFrame[KeyFrames.Length - 1];
            TangentMode[] newTangentModes = new TangentMode[tangentModes.Length - 1];

            Array.Copy(KeyFrames, newKeyFrames, index);
            Array.Copy(tangentModes, newTangentModes, index);

            if (index < newKeyFrames.Length)
            {
                int remaining = newKeyFrames.Length - index;
                Array.Copy(KeyFrames, index + 1, newKeyFrames, index, remaining);
                Array.Copy(tangentModes, index + 1, newTangentModes, index, remaining);
            }

            tangentModes = newTangentModes;
            keyFrames = newKeyFrames;
        }

        /// <summary>
        /// Updates key-frame time and value. Since keyframes are ordered by time the index of the keyframe might change,
        /// so a new index of the keyframe is returned by this method. 
        /// </summary>
        /// <param name="index">Index of the keyframe to update, referencing the <see cref="KeyFrames"/> array.</param>
        /// <param name="time">Time to which to set the keyframe.</param>
        /// <param name="value">Value of the keyframe.</param>
        /// <returns>New index of the keyframe, referencing the <see cref="KeyFrames"/> array.</returns>
        internal int UpdateKeyframe(int index, float time, float value)
        {
            if (index < 0 || index >= keyFrames.Length)
                return -1;

            keyFrames[index].time = time;
            keyFrames[index].value = value;

            // Check if key moved before or after other keys. Animation curve automatically sorts
            // keys and if this happens our key indices will change. So we sort it here and modify
            // indices.

            int currentKeyIndex = index;
            int prevKeyIdx = currentKeyIndex - 1;
            while (prevKeyIdx >= 0)
            {
                if (time >= keyFrames[prevKeyIdx].time)
                    break;

                KeyFrame temp = keyFrames[prevKeyIdx];
                keyFrames[prevKeyIdx] = keyFrames[currentKeyIndex];
                keyFrames[currentKeyIndex] = temp;

                TangentMode tempMode = tangentModes[prevKeyIdx];
                tangentModes[prevKeyIdx] = tangentModes[currentKeyIndex];
                tangentModes[currentKeyIndex] = tempMode;

                currentKeyIndex = prevKeyIdx;
                prevKeyIdx--;
            }

            int nextKeyIdx = currentKeyIndex + 1;
            while (nextKeyIdx < keyFrames.Length)
            {
                if (time <= keyFrames[nextKeyIdx].time)
                    break;

                KeyFrame temp = keyFrames[nextKeyIdx];
                keyFrames[nextKeyIdx] = keyFrames[currentKeyIndex];
                keyFrames[currentKeyIndex] = temp;

                TangentMode tempMode = tangentModes[nextKeyIdx];
                tangentModes[nextKeyIdx] = tangentModes[currentKeyIndex];
                tangentModes[currentKeyIndex] = tempMode;

                currentKeyIndex = nextKeyIdx;
                nextKeyIdx++;
            }

            return currentKeyIndex;
        }

        /// <summary>
        /// Changes the tangent mode of a keyframe at the specified index.
        /// </summary>
        /// <param name="index">Index of the keyframe to update, referencing the <see cref="KeyFrames"/> array.</param>
        /// <param name="mode">New tangent mode of the keyframe.</param>
        internal void SetTangentMode(int index, TangentMode mode)
        {
            if (index < 0 || index >= tangentModes.Length)
                return;

            tangentModes[index] = mode;
        }

        /// <summary>
        /// Converts a keyframe tangent (slope) value into a 2D normal vector.
        /// </summary>
        /// <param name="tangent">Keyframe tangent (slope).</param>
        /// <returns>Normalized 2D vector pointing in the direction of the tangent.</returns>
        internal static Vector2 TangentToNormal(float tangent)
        {
            if(tangent == float.PositiveInfinity)
                return new Vector2(0, 1);

            Vector2 normal = new Vector2(1, tangent);
            return Vector2.Normalize(normal);
        }

        /// <summary>
        /// Converts a 2D normal vector into a keyframe tangent (slope).
        /// </summary>
        /// <param name="normal">Normalized 2D vector pointing in the direction of the tangent.</param>
        /// <returns>Keyframe tangent (slope).</returns>
        internal static float NormalToTangent(Vector2 normal)
        {
            // We know the X value must be one, use that to deduce pre-normalized length
            float length = 1/normal.x;

            // Use length to deduce the tangent (y coordinate)
            return MathEx.Sqrt(length*length - 1) * MathEx.Sign(normal.y);
        }

        /// <summary>
        /// Applies the changes of the editor curve, to the actual underlying animation curve.
        /// </summary>
        internal void Apply()
        {
            Array.Sort(keyFrames, (x, y) =>
            {
                return x.time.CompareTo(y.time);
            });

            UpdateTangents();
            native.KeyFrames = keyFrames;
        }

        /// <summary>
        /// Recalculates tangents for all keyframes using the keyframe values and set tangent modes.
        /// </summary>
        private void UpdateTangents()
        {
            if (keyFrames.Length == 0)
                return;

            if (keyFrames.Length == 1)
            {
                keyFrames[0].inTangent = 0.0f;
                keyFrames[0].outTangent = 0.0f;

                return;
            }

            // First keyframe
            {
                KeyFrame keyThis = keyFrames[0];
                KeyFrame keyNext = keyFrames[1];

                keyThis.inTangent = 0.0f;

                TangentMode tangentMode = tangentModes[0];
                if (tangentMode == TangentMode.Auto || tangentMode.HasFlag(TangentMode.OutAuto) || tangentMode.HasFlag(TangentMode.OutLinear))
                {
                    float diff = keyNext.time - keyThis.time;

                    if(!MathEx.ApproxEquals(diff, 0.0f))
                        keyThis.outTangent = (keyNext.value - keyThis.value) / diff;
                    else
                        keyThis.outTangent = float.PositiveInfinity;
                }
                else if (tangentMode.HasFlag(TangentMode.OutStep))
                {
                    keyThis.outTangent = float.PositiveInfinity;
                }

                keyFrames[0] = keyThis;
            }

            // Inner keyframes
            for(int i = 1; i < keyFrames.Length - 1; i++)
            {
                KeyFrame keyPrev = keyFrames[i - 1];
                KeyFrame keyThis = keyFrames[i];
                KeyFrame keyNext = keyFrames[i + 1];

                keyThis.inTangent = 0.0f;

                TangentMode tangentMode = tangentModes[i];
                if (tangentMode == TangentMode.Auto) // Both automatic
                {
                    float diff = keyNext.time - keyPrev.time;

                    if (!MathEx.ApproxEquals(diff, 0.0f))
                        keyThis.outTangent = (keyNext.value - keyPrev.value) / diff;
                    else
                        keyThis.outTangent = float.PositiveInfinity;

                    keyThis.inTangent = keyThis.outTangent;
                }
                else if (tangentMode == TangentMode.Free) // Both free
                {
                    keyThis.inTangent = keyThis.outTangent;
                }
                else // Different per-tangent modes
                {
                    // In tangent
                    if (tangentMode.HasFlag(TangentMode.InAuto))
                    {
                        float diff = keyNext.time - keyPrev.time;

                        if (!MathEx.ApproxEquals(diff, 0.0f))
                            keyThis.inTangent = (keyNext.value - keyPrev.value)/diff;
                        else
                            keyThis.inTangent = float.PositiveInfinity;
                    }
                    else if (tangentMode.HasFlag(TangentMode.InLinear))
                    {
                        float diff = keyThis.time - keyPrev.time;

                        if (!MathEx.ApproxEquals(diff, 0.0f))
                            keyThis.inTangent = (keyThis.value - keyPrev.value) / diff;
                        else
                            keyThis.inTangent = float.PositiveInfinity;
                    }
                    else if (tangentMode.HasFlag(TangentMode.InStep))
                    {
                        keyThis.inTangent = float.PositiveInfinity;
                    }

                    // Out tangent
                    if (tangentMode.HasFlag(TangentMode.OutAuto))
                    {
                        float diff = keyNext.time - keyPrev.time;

                        if (!MathEx.ApproxEquals(diff, 0.0f))
                            keyThis.outTangent = (keyNext.value - keyPrev.value) / diff;
                        else
                            keyThis.outTangent = float.PositiveInfinity;
                    }
                    else if (tangentMode.HasFlag(TangentMode.OutLinear))
                    {
                        float diff = keyNext.time - keyThis.time;

                        if (!MathEx.ApproxEquals(diff, 0.0f))
                            keyThis.outTangent = (keyNext.value - keyThis.value) / diff;
                        else
                            keyThis.outTangent = float.PositiveInfinity;
                    }
                    else if (tangentMode.HasFlag(TangentMode.OutStep))
                    {
                        keyThis.outTangent = float.PositiveInfinity;
                    }
                }

                keyFrames[i] = keyThis;
            }

            // Last keyframe
            {
                KeyFrame keyThis = keyFrames[keyFrames.Length - 1];
                KeyFrame keyPrev = keyFrames[keyFrames.Length - 2];

                keyThis.outTangent = 0.0f;

                TangentMode tangentMode = tangentModes[tangentModes.Length - 1];
                if (tangentMode == TangentMode.Auto || tangentMode.HasFlag(TangentMode.InAuto) || tangentMode.HasFlag(TangentMode.InLinear))
                {
                    float diff = keyThis.time - keyPrev.time;

                    if (!MathEx.ApproxEquals(diff, 0.0f))
                        keyThis.inTangent = (keyThis.value - keyPrev.value)/diff;
                    else
                        keyThis.inTangent = float.PositiveInfinity;
                }
                else if (tangentMode.HasFlag(TangentMode.InStep))
                {
                    keyThis.inTangent = float.PositiveInfinity;
                }

                keyFrames[keyFrames.Length - 1] = keyThis;
            }
        }
    }

    /// <summary>
    /// Structure containing a reference to a keyframe as a curve index, and a keyframe index within that curve.
    /// </summary>
    internal struct KeyframeRef
    {
        public KeyframeRef(int curveIdx, int keyIdx)
        {
            this.curveIdx = curveIdx;
            this.keyIdx = keyIdx;
        }

        public int curveIdx;
        public int keyIdx;
    }

    /// <summary>
    /// Structure containing a reference to a keyframe tangent, as a keyframe reference and type of the tangent.
    /// </summary>
    internal struct TangentRef
    {
        public TangentRef(KeyframeRef keyframeRef, TangentType type)
        {
            this.keyframeRef = keyframeRef;
            this.type = type;
        }

        public KeyframeRef keyframeRef;
        public TangentType type;
    }

    /** @} */
}
