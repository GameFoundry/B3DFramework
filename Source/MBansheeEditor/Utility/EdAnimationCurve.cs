//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;
using BansheeEngine;

namespace BansheeEditor
{
    internal enum TangentMode
    {
        Auto        = 0,
        Free        = 1 << 0,
        InAuto      = 1 << 1,
        OutAuto     = 1 << 2,
        InFree      = 1 << 3,
        OutFree     = 1 << 4,
        InLinear    = 1 << 5,
        OutLinear   = 1 << 6,
        InStep      = 1 << 7,
        OutStep     = 1 << 8
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
                if (tangentMode == TangentMode.Auto || tangentMode == TangentMode.OutAuto || tangentMode == TangentMode.OutLinear)
                {
                    float diff = keyNext.time - keyThis.time;
                    keyThis.outTangent = (keyNext.value - keyThis.value) / diff;
                }
                else if (tangentMode == TangentMode.OutStep)
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
                    keyThis.outTangent = (keyNext.value - keyPrev.value) / diff;
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
                        keyThis.inTangent = (keyNext.value - keyPrev.value) / diff;
                    }
                    else if (tangentMode.HasFlag(TangentMode.InLinear))
                    {
                        float diff = keyThis.time - keyPrev.time;
                        keyThis.inTangent = (keyThis.value - keyPrev.value) / diff;
                    }
                    else if (tangentMode.HasFlag(TangentMode.InStep))
                    {
                        keyThis.inTangent = float.PositiveInfinity;
                    }

                    // Out tangent
                    if (tangentMode.HasFlag(TangentMode.OutAuto))
                    {
                        float diff = keyNext.time - keyPrev.time;
                        keyThis.outTangent = (keyNext.value - keyPrev.value) / diff;
                    }
                    else if (tangentMode.HasFlag(TangentMode.OutLinear))
                    {
                        float diff = keyNext.time - keyThis.time;
                        keyThis.outTangent = (keyNext.value - keyThis.value) / diff;
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
                if (tangentMode == TangentMode.Auto || tangentMode == TangentMode.InAuto || tangentMode == TangentMode.InLinear)
                {
                    float diff = keyThis.time - keyPrev.time;
                    keyThis.inTangent = (keyThis.value - keyPrev.value) / diff;
                }
                else if (tangentMode == TangentMode.InStep)
                {
                    keyThis.inTangent = float.PositiveInfinity;
                }

                keyFrames[keyFrames.Length - 1] = keyThis;
            }
        }
    }
}
