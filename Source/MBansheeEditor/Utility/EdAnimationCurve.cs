//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;
using System.Collections.Generic;
using BansheeEngine;

namespace BansheeEditor
{
    internal enum TangentAutoType
    {
        Auto, Linear, Step
    }

    internal enum TangentModeType
    {
        Auto, Free, Broken
    }

    internal struct TangentMode
    {
        public TangentAutoType inTangent;
        public TangentAutoType outTangent;
        public TangentModeType type;

        public static readonly TangentMode AUTO = new TangentMode();
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
            AddKeyframe(time, value, TangentMode.AUTO);
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

                if (tangentModes[0].type == TangentModeType.Auto)
                {
                    TangentAutoType autoType = tangentModes[0].outTangent;

                    switch (autoType)
                    {
                        case TangentAutoType.Auto:
                        case TangentAutoType.Linear:
                            float diff = keyNext.time - keyThis.time;
                            keyThis.outTangent = (keyNext.value - keyThis.value)/diff;

                            break;
                        case TangentAutoType.Step:
                            keyThis.outTangent = float.PositiveInfinity;
                            break;
                    }
                }

                keyFrames[0] = keyThis;
            }

            // TODO - Middle keyframes
            
            // TODO - How to handle transition from broken to free? 

            // Last keyframe
            {
                KeyFrame keyThis = keyFrames[keyFrames.Length - 1];
                KeyFrame keyPrev = keyFrames[keyFrames.Length - 2];

                keyThis.outTangent = 0.0f;

                if (tangentModes[0].type == TangentModeType.Auto)
                {
                    TangentAutoType autoType = tangentModes[0].outTangent;

                    switch (autoType)
                    {
                        case TangentAutoType.Auto:
                        case TangentAutoType.Linear:
                            float diff = keyThis.time - keyPrev.time;
                            keyThis.inTangent = (keyThis.value - keyPrev.value) / diff;

                            break;
                        case TangentAutoType.Step:
                            keyThis.inTangent = float.PositiveInfinity;
                            break;
                    }
                }

                keyFrames[keyFrames.Length - 1] = keyThis;
            }
        }
    }
}
