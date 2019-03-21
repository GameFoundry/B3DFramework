//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System.Runtime.CompilerServices;

namespace bs
{
    /** @addtogroup Utility
     *  @{
     */


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
            Box, Sphere, Cone, Cylinder, Quad, Disc
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
        /// Returns a cylinder mesh.
        /// </summary>
        public static Mesh Cylinder
        {
            get { return Internal_GetMesh(BuiltinMesh.Cylinder); }
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
