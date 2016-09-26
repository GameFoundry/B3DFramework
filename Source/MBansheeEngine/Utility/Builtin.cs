//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System.Runtime.CompilerServices;

namespace BansheeEngine
{
    /** @addtogroup Utility
     *  @{
     */


    /// <summary>
    /// Available builtin shader types.
    /// </summary>
    public enum BuiltinShader // Note: Must match C++ BuiltinShader enum
    {
        Standard,
        Custom
    }

    /// <summary>
    /// Contains various builtin resources that are always available.
    /// </summary>
    public static class Builtin
    {
        /// <summary>
        /// Types of builtin meshes that are always available in the engine.
        /// </summary>
        private enum BuiltinMesh // Note: Must match C++ enum BuiltinMesh
        {
            Box, Sphere, Cone, Quad, Disc
        }

        /// <summary>
        /// Returns a pure white texture.
        /// </summary>
        public static SpriteTexture WhiteTexture
        {
            get { return Internal_GetWhiteTexture(); }
        }

        /// <summary>
        /// Returns one of the builtin shaders
        /// </summary>
        public static Shader GetShader(BuiltinShader shader)
        {
            if (shader == BuiltinShader.Custom)
                return null;

            return Internal_GetBuiltinShader((int)shader);
        }

        /// <summary>
        /// Returns a axis aligned box of unit size.
        /// </summary>
        public static Mesh Box
        {
            get { return Internal_GetMesh(BuiltinMesh.Box); }
        }

        /// <summary>
        /// Returns a unit sphere mesh.
        /// </summary>
        public static Mesh Sphere
        {
            get { return Internal_GetMesh(BuiltinMesh.Sphere); }
        }

        /// <summary>
        /// Returns a cone mesh.
        /// </summary>
        public static Mesh Cone
        {
            get { return Internal_GetMesh(BuiltinMesh.Cone); }
        }

        /// <summary>
        /// Returns a quad mesh with unit size edges.
        /// </summary>
        public static Mesh Quad
        {
            get { return Internal_GetMesh(BuiltinMesh.Quad); }
        }

        /// <summary>
        /// Returns a disc mesh with unit radius.
        /// </summary>
        public static Mesh Disc
        {
            get { return Internal_GetMesh(BuiltinMesh.Disc); }
        }

        /// <summary>
        /// Returns the default Font used in the engine.
        /// </summary>
        public static Font DefaultFont
        {
            get { return Internal_GetDefaultFont(); }
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern SpriteTexture Internal_GetWhiteTexture();

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern Shader Internal_GetBuiltinShader(int shaderType);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern Mesh Internal_GetMesh(BuiltinMesh mesh);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern Font Internal_GetDefaultFont();
    }

    /** @} */
}
