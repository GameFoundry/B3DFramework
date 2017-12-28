//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2017 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using BansheeEngine;

namespace BansheeEditor
{
    /** @addtogroup Gizmos
     *  @{
     */

    /// <summary>
    /// Handles drawing of gizmos for the <see cref="Camera"/> component.
    /// </summary>
    internal class CameraGizmo
    {
        /// <summary>
        /// Method called by the runtime when gizmos are meant to be drawn.
        /// </summary>
        /// <param name="camera">Camera to draw gizmos for.</param>
        [DrawGizmo(DrawGizmoFlags.Selected)]
        private static void Draw(Camera camera)
        {
            SceneObject so = camera.SceneObject;

            Gizmos.Color = Color.Yellow;
            Gizmos.Transform = Matrix4.TRS(so.Position, Quaternion.LookRotation(so.Rotation.Forward, so.Rotation.Up), Vector3.One);

            Gizmos.DrawFrustum(Vector3.Zero, camera.AspectRatio, camera.FieldOfView, camera.NearClipPlane,
                camera.FarClipPlane);
        }
    }

    /** @} */
}
