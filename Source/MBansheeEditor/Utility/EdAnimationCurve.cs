//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;
using BansheeEngine;

namespace BansheeEditor
{
    internal enum TangentType
    {
        In = 1 << 0,
        Out = 1 << 1
    }

    [Flags]
    internal enum TangentMode
    {
        Auto        = 0,
        InAuto      = TangentType.In | 1 << 2,
        InFree      = TangentType.In | 1 << 3,
        InLinear    = TangentType.In | 1 << 4,
        InStep      = TangentType.In | 1 << 5,
        OutAuto     = TangentType.Out | 1 << 6,
        OutFree     = TangentType.Out | 1 << 7,
        OutLinear   = TangentType.Out | 1 << 8,
        OutStep     = TangentType.Out | 1 << 9,
        Free        = 1 << 10,
    }

    internal class EdAnimationCurve
    {
        private AnimationCurve native;
        private TangentMode[] tangentModes;

        public AnimationCurve Native
        {
            get { return native; }
        }

        public TangentMode[] TangentModes
        {
            get { return tangentModes; }
        }

        internal EdAnimationCurve()
        {
            KeyFrame[] keyframes = new KeyFrame[0];
            native = new AnimationCurve(keyframes);

            tangentModes = new TangentMode[0];
        }

        // Tangent modes should match number of curve keyframes
        internal EdAnimationCurve(AnimationCurve native, TangentMode[] tangentModes)
        {
            this.native = native;

            KeyFrame[] keyFrames = native.KeyFrames;

            this.tangentModes = new TangentMode[keyFrames.Length];
            if (tangentModes != null)
            {
                int numTangents = Math.Min(keyFrames.Length, tangentModes.Length);
                Array.Copy(tangentModes, this.tangentModes, numTangents);
            }

            UpdateTangents(keyFrames, this.tangentModes);
        }

        internal void AddKeyframe(float time, float value)
        {
            AddKeyframe(time, value, TangentMode.Auto);
        }

        internal void AddKeyframe(float time, float value, TangentMode tangentMode)
        {
            KeyFrame[] existingKeyFrames = native.KeyFrames;

            KeyFrame[] newKeyFrames = new KeyFrame[existingKeyFrames.Length + 1];
            newKeyFrames[newKeyFrames.Length - 1].time = float.PositiveInfinity;

            TangentMode[] newTangentModes = new TangentMode[tangentModes.Length + 1];

            int insertIdx = existingKeyFrames.Length;
            for (int i = 0; i < existingKeyFrames.Length; i++)
            {
                if (time < existingKeyFrames[i].time)
                {
                    insertIdx = i;
                    break;
                }
            }

            Array.Copy(existingKeyFrames, newKeyFrames, insertIdx);
            Array.Copy(tangentModes, newTangentModes, insertIdx);

            KeyFrame keyFrame = new KeyFrame();
            keyFrame.time = time;
            keyFrame.value = value;

            newKeyFrames[insertIdx] = keyFrame;
            newTangentModes[insertIdx] = tangentMode;

            if (insertIdx < existingKeyFrames.Length)
            {
                int remaining = existingKeyFrames.Length - insertIdx;
                Array.Copy(existingKeyFrames, insertIdx, newKeyFrames, insertIdx + 1, remaining);
                Array.Copy(tangentModes, insertIdx, newTangentModes, insertIdx + 1, remaining);
            }

            UpdateTangents(newKeyFrames, newTangentModes);

            tangentModes = newTangentModes;
            native.KeyFrames = newKeyFrames;
        }

        internal void RemoveKeyframe(int index)
        {
            KeyFrame[] existingKeyFrames = native.KeyFrames;
            if (index < 0 || index >= existingKeyFrames.Length)
                return;

            KeyFrame[] newKeyFrames = new KeyFrame[existingKeyFrames.Length - 1];
            TangentMode[] newTangentModes = new TangentMode[tangentModes.Length - 1];

            Array.Copy(existingKeyFrames, newKeyFrames, index);
            Array.Copy(tangentModes, newTangentModes, index);

            if (index < newKeyFrames.Length)
            {
                int remaining = newKeyFrames.Length - index;
                Array.Copy(existingKeyFrames, index + 1, newKeyFrames, index, remaining);
                Array.Copy(tangentModes, index + 1, newTangentModes, index, remaining);
            }

            UpdateTangents(newKeyFrames, newTangentModes);

            tangentModes = newTangentModes;
            native.KeyFrames = newKeyFrames;
        }

        internal void SetTangentMode(int index, TangentMode mode)
        {
            if (index < 0 || index >= tangentModes.Length)
                return;

            tangentModes[index] = mode;

            KeyFrame[] keyFrames = native.KeyFrames;
            UpdateTangents(keyFrames, tangentModes);
            native.KeyFrames = keyFrames;
        }

        internal static Vector2 TangentToNormal(float tangent)
        {
            if(tangent == float.PositiveInfinity)
                return new Vector2(0, 1);

            Vector2 normal = new Vector2(1, tangent);
            return Vector2.Normalize(normal);
        }

        internal static float NormalToTangent(Vector2 normal)
        {
            // We know the X value must be one, use that to deduce pre-normalized length
            float length = 1/normal.x;

            // Use length to deduce the tangent (y coordinate)
            return MathEx.Sqrt(length*length - 1);
        }

        private void UpdateTangents(KeyFrame[] keyFrames, TangentMode[] tangentModes)
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
}
