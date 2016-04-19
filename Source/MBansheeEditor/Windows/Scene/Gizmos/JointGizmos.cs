//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;
using BansheeEngine;

namespace BansheeEditor
{
    /** @addtogroup Gizmos
     *  @{
     */

    /// <summary>
    /// Handles drawing of gizmos for all the types of <see cref="Joint"/> component.
    /// </summary>
    internal class JointGizmos
    {
        /// <summary>
        /// Returns the anchor position for the specified joint body. Anchor represents the world position of the rigidbody
        /// added to the offset provided by the joint.
        /// </summary>
        /// <param name="joint">Joint from which to retrieve the body.</param>
        /// <param name="body">Body to retrieve the anchor for.</param>
        /// <returns>Anchor position in world space.</returns>
        private static Vector3 GetAnchor(Joint joint, JointBody body)
        {
            Rigidbody rigidbody = joint.GetRigidbody(body);
            Vector3 anchor = joint.GetPosition(body);

            if (rigidbody != null)
            {
                Quaternion worldRot = rigidbody.SceneObject.Rotation;
                anchor = worldRot.Rotate(anchor) + rigidbody.SceneObject.Position;
            }
            else
            {
                Quaternion worldRot = joint.SceneObject.Rotation;
                anchor = worldRot.Rotate(anchor) + joint.SceneObject.Position;
            }

            return anchor;
        }

        /// <summary>
        /// Method called by the runtime when joints are meant to be drawn.
        /// </summary>
        /// <param name="joint">Joint to draw gizmos for.</param>
        [DrawGizmo(DrawGizmoFlags.Selected | DrawGizmoFlags.ParentSelected)]
        private static void DrawFixedJoint(FixedJoint joint)
        {
            Vector3 target = GetAnchor(joint, JointBody.Target);
            Vector3 anchor = GetAnchor(joint, JointBody.Anchor);
            Vector3 center = target;

            Rigidbody rigidbody = joint.GetRigidbody(JointBody.Target);
            if (rigidbody != null)
                center = rigidbody.SceneObject.Position;

            Gizmos.Color = Color.White;
            Gizmos.DrawSphere(center, 0.05f);

            Gizmos.Color = Color.Yellow;
            Gizmos.DrawSphere(target, 0.05f);
            Gizmos.DrawSphere(anchor, 0.05f);

            Gizmos.Color = Color.Green;
            Gizmos.DrawLine(target, center);
        }

        /// <summary>
        /// Method called by the runtime when joints are meant to be drawn.
        /// </summary>
        /// <param name="joint">Joint to draw gizmos for.</param>
        [DrawGizmo(DrawGizmoFlags.Selected | DrawGizmoFlags.ParentSelected)]
        private static void DrawDistanceJoint(DistanceJoint joint)
        {
            Vector3 anchorA = GetAnchor(joint, JointBody.Target);
            Vector3 anchorB = GetAnchor(joint, JointBody.Anchor);

            Gizmos.Color = Color.White;
            Gizmos.DrawSphere(anchorA, 0.05f);
            Gizmos.DrawSphere(anchorB, 0.05f);

            Gizmos.Color = Color.Red;
            Vector3 diff = anchorB - anchorA;

            float length = diff.Length;
            Vector3 normal = diff.Normalized;

            float min = 0.0f;
            float max = length;

            if (joint.EnableMinDistanceLimit)
            {
                min = MathEx.Max(0.0f, joint.MinDistance);
                if (joint.EnableMaxDistanceLimit)
                    min = MathEx.Min(min, MathEx.Min(10000.0f, joint.MaxDistance));

                Gizmos.DrawLine(anchorA, anchorA + normal * min);
            }

            if (joint.EnableMaxDistanceLimit)
            {
                max = MathEx.Min(10000.0f, joint.MaxDistance);
                if (joint.EnableMinDistanceLimit)
                    max = MathEx.Max(max, min);

                if (length > max)
                    Gizmos.DrawLine(anchorA + normal * max, anchorA + normal * length);
            }

            Gizmos.Color = Color.Green;
            Gizmos.DrawLine(anchorA + normal * min, anchorA + normal * MathEx.Min(max, length));
        }

        /// <summary>
        /// Method called by the runtime when joints are meant to be drawn.
        /// </summary>
        /// <param name="joint">Joint to draw gizmos for.</param>
        [DrawGizmo(DrawGizmoFlags.Selected | DrawGizmoFlags.ParentSelected)]
        private static void DrawSliderJoint(SliderJoint joint)
        {
            Vector3 anchor = GetAnchor(joint, JointBody.Anchor);
            Vector3 target = GetAnchor(joint, JointBody.Target);
            Vector3 normal = -joint.SceneObject.Right;

            if (joint.EnableLimit)
            {
                LimitLinearRange limit = joint.Limit;

                float max = MathEx.Min(10000.0f, limit.Upper);
                float min = MathEx.Clamp(limit.Lower, 0.0f, max);
                max = MathEx.Max(max, min);

                Gizmos.Color = Color.Red;
                Gizmos.DrawLine(anchor, anchor + normal*min);

                Gizmos.Color = Color.Green;
                Gizmos.DrawLine(anchor + normal*min, anchor + normal*max);
            }
            else
            {
                Gizmos.Color = Color.Green;

                float length = 100.0f;
                Gizmos.DrawLine(anchor, anchor + normal * length);
            }

            Gizmos.Color = Color.Yellow;
            Gizmos.DrawSphere(target, 0.05f);
        }

        /// <summary>
        /// Method called by the runtime when joints are meant to be drawn.
        /// </summary>
        /// <param name="joint">Joint to draw gizmos for.</param>
        [DrawGizmo(DrawGizmoFlags.Selected | DrawGizmoFlags.ParentSelected)]
        private static void DrawSphericalJoint(SphericalJoint joint)
        {
            Vector3 anchorA = GetAnchor(joint, JointBody.Target);
            Vector3 anchorB = GetAnchor(joint, JointBody.Anchor);

            Gizmos.Color = Color.White;
            Gizmos.DrawSphere(anchorA, 0.05f);
            Gizmos.DrawSphere(anchorB, 0.05f);

            Gizmos.Color = Color.Green;
            if (joint.EnableLimit)
            {
                LimitConeRange limit = joint.Limit;

                Radian zAngle = MathEx.Min(new Degree(360), limit.ZLimitAngle*2.0f);
                Radian yAngle = MathEx.Min(new Degree(360), limit.YLimitAngle*2.0f);

                Gizmos.Transform = joint.SceneObject.WorldTransform;
                Gizmos.DrawWireArc(Vector3.Zero, Vector3.ZAxis, 0.25f, zAngle * -0.5f, zAngle);
                Gizmos.DrawWireArc(Vector3.Zero, Vector3.YAxis, 0.25f, yAngle * -0.5f, yAngle);

                Gizmos.Color = Color.Red;
                Radian remainingZAngle = new Degree(360) - zAngle;
                Radian remainingYAngle = new Degree(360) - yAngle;

                Gizmos.DrawWireArc(Vector3.Zero, Vector3.ZAxis, 0.25f, zAngle * 0.5f, remainingZAngle);
                Gizmos.DrawWireArc(Vector3.Zero, Vector3.YAxis, 0.25f, yAngle * 0.5f, remainingYAngle);
            }
            else
            {
                Gizmos.Color = Color.Green;
                Gizmos.Transform = joint.SceneObject.WorldTransform;

                Gizmos.DrawWireDisc(Vector3.Zero, Vector3.ZAxis, 0.25f);
                Gizmos.DrawWireDisc(Vector3.Zero, Vector3.YAxis, 0.25f);
            }
        }

        /// <summary>
        /// Method called by the runtime when joints are meant to be drawn.
        /// </summary>
        /// <param name="joint">Joint to draw gizmos for.</param>
        [DrawGizmo(DrawGizmoFlags.Selected | DrawGizmoFlags.ParentSelected)]
        private static void DrawHingeJoint(HingeJoint joint)
        {
            Vector3 anchorA = GetAnchor(joint, JointBody.Target);
            Vector3 anchorB = GetAnchor(joint, JointBody.Anchor);

            Gizmos.Color = Color.White;
            Gizmos.DrawSphere(anchorA, 0.05f);
            Gizmos.DrawSphere(anchorB, 0.05f);

            const float radius = 0.05f;
            const float height = 0.25f;

            if (joint.EnableLimit)
            {
                Gizmos.Transform = joint.SceneObject.WorldTransform;

                LimitAngularRange limit = joint.Limit;

                Action<float> drawLimitedArc = x =>
                {
                    Degree lower = MathEx.WrapAngle(limit.Lower);
                    Degree upper = MathEx.WrapAngle(limit.Upper);

                    lower = MathEx.Min(lower, upper);
                    upper = MathEx.Max(upper, lower);

                    // Arc zero to lower limit
                    Gizmos.Color = Color.Red;
                    Gizmos.DrawWireArc(Vector3.XAxis * x, Vector3.XAxis, radius, new Degree(0.0f), lower);

                    // Arc lower to upper limit
                    Degree validRange = upper - lower;

                    Gizmos.Color = Color.Green;
                    Gizmos.DrawWireArc(Vector3.XAxis * x, Vector3.XAxis, radius, lower, validRange);

                    // Arc upper to full circle
                    Degree remainingRange = new Degree(360) - upper;

                    Gizmos.Color = Color.Red;
                    Gizmos.DrawWireArc(Vector3.XAxis * x, Vector3.XAxis, radius, upper, remainingRange);
                };

                drawLimitedArc(-height);
                drawLimitedArc(height);
            }
            else
            {
                Gizmos.Color = Color.Green;
                Gizmos.Transform = joint.SceneObject.WorldTransform;

                Gizmos.DrawWireDisc(Vector3.XAxis * -height, Vector3.XAxis, radius);
                Gizmos.DrawWireDisc(Vector3.XAxis * height, Vector3.XAxis, radius);
            }

            Vector3[] lineStartPoints = new Vector3[4];
            lineStartPoints[0] = new Vector3(-height, radius, 0);
            lineStartPoints[1] = new Vector3(-height, -radius, 0);
            lineStartPoints[2] = new Vector3(-height, 0, radius);
            lineStartPoints[3] = new Vector3(-height, 0, -radius);

            Vector3[] lineEndPoints = new Vector3[4];
            lineEndPoints[0] = new Vector3(height, radius, 0);
            lineEndPoints[1] = new Vector3(height, -radius, 0);
            lineEndPoints[2] = new Vector3(height, 0, radius);
            lineEndPoints[3] = new Vector3(height, 0, -radius);

            Gizmos.Color = Color.Green;
            for (int i = 0; i < 4; i++)
                Gizmos.DrawLine(lineStartPoints[i], lineEndPoints[i]);
        }

        /// <summary>
        /// Method called by the runtime when joints are meant to be drawn.
        /// </summary>
        /// <param name="joint">Joint to draw gizmos for.</param>
        [DrawGizmo(DrawGizmoFlags.Selected | DrawGizmoFlags.ParentSelected)]
        private static void DrawD6Joint(D6Joint joint)
        {
            Vector3 anchorA = GetAnchor(joint, JointBody.Target);
            Vector3 anchorB = GetAnchor(joint, JointBody.Anchor);

            Gizmos.Color = Color.White;
            Gizmos.DrawSphere(anchorA, 0.05f);
            Gizmos.DrawSphere(anchorB, 0.05f);
        }
    }

    /** @} */
}