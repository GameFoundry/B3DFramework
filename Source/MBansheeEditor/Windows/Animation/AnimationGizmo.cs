//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using BansheeEngine;

namespace BansheeEditor
{
    /** @addtogroup Gizmos
     *  @{
     */

    /// <summary>
    /// Handles drawing of gizmos for the <see cref="Animation"/> component.
    /// </summary>
    internal class AnimationGizmo
    {
        /// <summary>
        /// Method called by the runtime when gizmos are meant to be drawn.
        /// </summary>
        /// <param name="animation">Animation component to draw gizmo for.</param>
        [DrawGizmo(DrawGizmoFlags.Selected | DrawGizmoFlags.ParentSelected)]
        private static void DrawGizmo(Animation animation)
        {
            SceneObject so = animation.SceneObject;

            Gizmos.Color = Color.Green;
            Gizmos.Transform = Matrix4.TRS(so.Position, so.Rotation, Vector3.One);

            AABox bounds = animation.Bounds;
            Vector3 scaledSize = bounds.Size * so.Scale;
            Gizmos.DrawWireCube(bounds.Center, scaledSize * 0.5f);
        }
    }

    /** @} */
}