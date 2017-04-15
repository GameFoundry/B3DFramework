//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using BansheeEngine;

namespace BansheeEditor
{
    /** @addtogroup Gizmos
     *  @{
     */

    /// <summary>
    /// Handles drawing of gizmos for the <see cref="ReflectionProbe"/> component.
    /// </summary>
    internal class ReflectionProbeGizmos
    {
        /// <summary>
        /// Method called by the runtime when gizmos are meant to be drawn.
        /// </summary>
        /// <param name="reflProbe">Reflection probe to draw gizmos for.</param>
        [DrawGizmo(DrawGizmoFlags.Selected)]
        private static void Draw(ReflectionProbe reflProbe)
        {
            Vector3 position = reflProbe.SceneObject.Position;

            Gizmos.Color = Color.Yellow;
            switch (reflProbe.Type)
            {
                case ReflectionProbeType.Box:
                    SceneObject so = reflProbe.SceneObject;

                    Gizmos.Transform = Matrix4.TRS(so.Position, so.Rotation, Vector3.One);

                    Vector3 scaledExtents = reflProbe.Extents * so.Scale;
                    Gizmos.DrawWireCube(Vector3.Zero, scaledExtents);
                    break;
                case ReflectionProbeType.Sphere:
                    Gizmos.DrawWireSphere(position, reflProbe.Radius);
                    break;
            }
        }
    }

    /** @} */
}
