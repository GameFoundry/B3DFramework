//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;
using System.Runtime.CompilerServices;
using BansheeEngine;

namespace BansheeEditor
{
    /** @addtogroup Gizmos
     *  @{
     */

    /// <summary>
    /// Provides functionality for drawing gizmos. This class should only be used in methods defined with 
    /// <see cref="DrawGizmo"/> attribute.
    /// </summary>
    public class Gizmos
    {
        /// <summary>
        /// Determines the color that will be used on any following draw method.
        /// </summary>
        public static Color Color
        {
            get
            {
                Color value;
                Internal_GetColor(out value); 
                return value; 
            }

            set
            {
                Internal_SetColor(ref value);
            }
        }

        /// <summary>
        /// Determines the world transform that will be applied to any following draw method.
        /// </summary>
        public static Matrix4 Transform
        {
            get
            {
                Matrix4 value;
                Internal_GetTransform(out value);
                return value; 
            }

            set
            {
                Internal_SetTransform(ref value);
            }
        }

        /// <summary>
        /// Draws an axis aligned solid cube.
        /// </summary>
        /// <param name="position">World coordinates of the center of the cube.</param>
        /// <param name="extents">Extents defining the half-size of the cube in each dimension.</param>
        public static void DrawCube(Vector3 position, Vector3 extents)
        {
            Internal_DrawCube(ref position, ref extents);
        }

        /// <summary>
        /// Draws a solid sphere.
        /// </summary>
        /// <param name="position">World coordinates of the center of the sphere.</param>
        /// <param name="radius">Sphere radius.</param>
        public static void DrawSphere(Vector3 position, float radius)
        {
            Internal_DrawSphere(ref position, radius);
        }

        /// <summary>
        /// Draws a solid cone.
        /// </summary>
        /// <param name="coneBase">Position of the center of the base of the cone.</param>
        /// <param name="normal">Orientation of the cone, pointing from center base to the tip of the cone.</param>
        /// <param name="height">Height of the cone (along the normal).</param>
        /// <param name="radius">Radius of the base of the cone.</param>
        public static void DrawCone(Vector3 coneBase, Vector3 normal, float height, float radius)
        {
            Vector2 scale = Vector2.One;
            Internal_DrawCone(ref coneBase, ref normal, height, radius, ref scale);
        }

        /// <summary>
        /// Draws a solid cone.
        /// </summary>
        /// <param name="coneBase">Position of the center of the base of the cone.</param>
        /// <param name="normal">Orientation of the cone, pointing from center base to the tip of the cone.</param>
        /// <param name="height">Height of the cone (along the normal).</param>
        /// <param name="radius">Radius of the base of the cone.</param>
        /// <param name="scale">Scale applied to cone's disc width & height. Allows you to create elliptical cones.</param>
        public static void DrawCone(Vector3 coneBase, Vector3 normal, float height, float radius, Vector2 scale)
        {
            Internal_DrawCone(ref coneBase, ref normal, height, radius, ref scale);
        }

        /// <summary>
        /// Draws a solid disc.
        /// </summary>
        /// <param name="position">Center of the disc.</param>
        /// <param name="normal">Normal towards which to orient the disc.</param>
        /// <param name="radius">Radius of the disc.</param>
        public static void DrawDisc(Vector3 position, Vector3 normal, float radius)
        {
            Internal_DrawDisc(ref position, ref normal, radius);
        }

        /// <summary>
        /// Draws an axis aligned wireframe cube.
        /// </summary>
        /// <param name="position">World coordinates of the center of the cube.</param>
        /// <param name="extents">Extents defining the half-size of the cube in each dimension.</param>
        public static void DrawWireCube(Vector3 position, Vector3 extents)
        {
            Internal_DrawWireCube(ref position, ref extents);
        }

        /// <summary>
        /// Draws a wireframe sphere.
        /// </summary>
        /// <param name="position">World coordinates of the center of the sphere.</param>
        /// <param name="radius">Sphere radius.</param>
        public static void DrawWireSphere(Vector3 position, float radius)
        {
            Internal_DrawWireSphere(ref position, radius);
        }

        /// <summary>
        /// Draws a wireframe capsule. Capsule is assumed to be extending along the Y axis.
        /// </summary>
        /// <param name="position">World coordinates of the center of the capsule.</param>
        /// <param name="height">Distance between the centers of the capsule's hemispheres.</param>
        /// <param name="radius">Distance of each point from the capsule's center-line.</param>
        public static void DrawWireCapsule(Vector3 position, float height, float radius)
        {
            Internal_DrawWireCapsule(ref position, height, radius);
        }

        /// <summary>
        /// Draws a wireframe cone.
        /// </summary>
        /// <param name="coneBase">Position of the center of the base of the cone.</param>
        /// <param name="normal">Orientation of the cone, pointing from center base to the tip of the cone.</param>
        /// <param name="height">Height of the cone (along the normal).</param>
        /// <param name="radius">Radius of the base of the cone.</param>
        public static void DrawWireCone(Vector3 coneBase, Vector3 normal, float height, float radius)
        {
            Vector2 scale = Vector2.One;
            Internal_DrawWireCone(ref coneBase, ref normal, height, radius, ref scale);
        }

        /// <summary>
        /// Draws a wireframe cone.
        /// </summary>
        /// <param name="coneBase">Position of the center of the base of the cone.</param>
        /// <param name="normal">Orientation of the cone, pointing from center base to the tip of the cone.</param>
        /// <param name="height">Height of the cone (along the normal).</param>
        /// <param name="radius">Radius of the base of the cone.</param>
        /// <param name="scale">Scale applied to cone's disc width & height. Allows you to create elliptical cones.</param>
        public static void DrawWireCone(Vector3 coneBase, Vector3 normal, float height, float radius, Vector2 scale)
        {
            Internal_DrawWireCone(ref coneBase, ref normal, height, radius, ref scale);
        }

        /// <summary>
        /// Draws a 3D line.
        /// </summary>
        /// <param name="start">Starting point for the line.</param>
        /// <param name="end">Ending point for the line.</param>
        public static void DrawLine(Vector3 start, Vector3 end)
        {
            Internal_DrawLine(ref start, ref end);
        }

        /// <summary>
        /// Draws a list of 3D lines.
        /// </summary>
        /// <param name="linePoints">A list of line point pairs, start point followed by end point, and so on.</param>
        public static void DrawLineList(Vector3[] linePoints)
        {
            Internal_DrawLineList(linePoints);
        }

        /// <summary>
        /// Draws a wireframe disc.
        /// </summary>
        /// <param name="position">Center of the disc.</param>
        /// <param name="normal">Normal towards which to orient the disc.</param>
        /// <param name="radius">Radius of the disc.</param>
        public static void DrawWireDisc(Vector3 position, Vector3 normal, float radius)
        {
            Internal_DrawWireDisc(ref position, ref normal, radius);
        }

        /// <summary>
        /// Draws a wireframe arc.
        /// </summary>
        /// <param name="position">Center of the disc out of which the arc is cut out of.</param>
        /// <param name="normal">Normal towards which to orient the arc.</param>
        /// <param name="radius">Radius of the  disc out of which the arc is cut out of.</param>
        /// <param name="startAngle">Angle at which the arc starts.</param>
        /// <param name="amountAngle">Length of the arc.</param>
        public static void DrawWireArc(Vector3 position, Vector3 normal, float radius, Degree startAngle, Degree amountAngle)
        {
            Internal_DrawWireArc(ref position, ref normal, radius, startAngle.Degrees, amountAngle.Degrees);
        }

        /// <summary>
        /// Draws a wireframe mesh.
        /// </summary>
        /// <param name="meshData">Object containing vertices and indices of the mesh.</param>
        public static void DrawWireMesh(MeshData meshData)
        {
            IntPtr meshDataPtr = IntPtr.Zero;
            if (meshData != null)
                meshDataPtr = meshData.GetCachedPtr();

            Internal_DrawWireMesh(meshDataPtr);
        }

        /// <summary>
        /// Draws a wireframe camera frustum.
        /// </summary>
        /// <param name="position">Origin of the frustum (place where the camera origin would normally be).</param>
        /// <param name="aspect">Aspect radio (width/height).</param>
        /// <param name="FOV">Horizontal field of view.</param>
        /// <param name="near">Distance from the origin to the near plane.</param>
        /// <param name="far">Distance from the origin to the far plane.</param>
        public static void DrawFrustum(Vector3 position, float aspect, Degree FOV, float near, float far)
        {
            Internal_DrawFrustum(ref position, aspect, ref FOV, near, far);
        }

        /// <summary>
        /// Draws a texture as a camera-facing quad at a specific position.
        /// </summary>
        /// <param name="position">World position of the center of the quad the texture will be drawn on.</param>
        /// <param name="image">Texture to draw.</param>
        /// <param name="fixedScale">If true the icon will remain consistent size regardless of distance from camera. 
        ///                          If false normal perspective foreshortening effect will occurr.</param>
        public static void DrawIcon(Vector3 position, SpriteTexture image, bool fixedScale)
        {
            Internal_DrawIcon(ref position, image, fixedScale);
        }

        /// <summary>
        /// Draws camera aligned text at the specified position.
        /// </summary>
        /// <param name="position">World position to center the text on.</param>
        /// <param name="text">String to draw.</param>
        /// <param name="font">Font used for drawing the characters.</param>
        /// <param name="size">Size of the characters, in points.</param>
        public static void DrawText(Vector3 position, string text, Font font = null, int size = 16)
        {
            IntPtr scriptFont = IntPtr.Zero;
            if (font != null)
                scriptFont = font.GetCachedPtr();

            Internal_DrawText(ref position, text, scriptFont, size);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetColor(ref Color color);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_GetColor(out Color color);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetTransform(ref Matrix4 transform);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_GetTransform(out Matrix4 transform);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_DrawCube(ref Vector3 position, ref Vector3 extents);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_DrawSphere(ref Vector3 position, float radius);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_DrawCone(ref Vector3 coneBase, ref Vector3 normal, float height, float radius, ref Vector2 scale);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_DrawDisc(ref Vector3 position, ref Vector3 normal, float radius);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_DrawWireCube(ref Vector3 position, ref Vector3 extents);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_DrawWireSphere(ref Vector3 position, float radius);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_DrawWireCapsule(ref Vector3 position, float height, float radius);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_DrawWireCone(ref Vector3 coneBase, ref Vector3 normal, float height, float radius, ref Vector2 scale);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_DrawLine(ref Vector3 start, ref Vector3 end);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_DrawLineList(Vector3[] linePoints);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_DrawWireDisc(ref Vector3 position, ref Vector3 normal, float radius);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_DrawWireArc(ref Vector3 position, ref Vector3 normal, float radius, 
            float startAngle, float amountAngle);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_DrawWireMesh(IntPtr meshData);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_DrawFrustum(ref Vector3 position, float aspect, ref Degree FOV, float near, float far);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_DrawIcon(ref Vector3 position, SpriteTexture image, bool fixedScale);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_DrawText(ref Vector3 position, string text, IntPtr font, int size);
    }

    /** @} */
}