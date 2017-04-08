//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;
using System.Runtime.CompilerServices;

namespace BansheeEngine
{
    /** @addtogroup Rendering
     *  @{
     */

    /// <summary>
    /// Material that controls how objects are rendered. It is represented by a shader and parameters used to set up that 
    /// shader. It provides a simple interface for manipulating the parameters.
    /// </summary>
    public class Material : Resource
    {
        /// <summary>
        /// Creates a new empty material that references no shader.
        /// </summary>
        public Material()
        {
            Internal_CreateInstance(this, IntPtr.Zero);
        }

        /// <summary>
        /// Constructor for internal runtime use only.
        /// </summary>
        /// <param name="dummy">Dummy parameter to differentiate it from other constructors.</param>
        private Material(bool dummy)
        { }

        /// <summary>
        /// Creates a new material with the specified shader.
        /// </summary>
        /// <param name="shader">Shader to initialize the material with.</param>
        public Material(Shader shader)
        {
            IntPtr nativeShader = IntPtr.Zero;
            if (shader != null)
                nativeShader = shader.GetCachedPtr();

            Internal_CreateInstance(this, nativeShader);
        }

        /// <summary>
        /// Shader used by the material. Best technique from the shader will be used for rendering, depending on currently
        /// active renderer and render API.
        /// </summary>
        public Shader Shader
        {
            get { return Internal_GetShader(mCachedPtr); }
            set
            {
                IntPtr nativeShader = IntPtr.Zero;
                if (value != null)
                    nativeShader = value.GetCachedPtr();

                Internal_SetShader(mCachedPtr, nativeShader); 
            }
        }

        /// <summary>
        /// Assigns a float value to the shader parameter with the specified name.
        /// </summary>
        /// <param name="name">Name of the shader parameter.</param>
        /// <param name="value">Value of the parameter.</param>
        public void SetFloat(string name, float value)
        {
            Internal_SetFloat(mCachedPtr, name, value);
        }

        /// <summary>
        /// Assigns a 2D vector to the shader parameter with the specified name.
        /// </summary>
        /// <param name="name">Name of the shader parameter.</param>
        /// <param name="value">Value of the parameter.</param>
        public void SetVector2(string name, Vector2 value)
        {
            Internal_SetVector2(mCachedPtr, name, ref value);
        }

        /// <summary>
        /// Assigns a 3D vector to the shader parameter with the specified name.
        /// </summary>
        /// <param name="name">Name of the shader parameter.</param>
        /// <param name="value">Value of the parameter.</param>
        public void SetVector3(string name, Vector3 value)
        {
            Internal_SetVector3(mCachedPtr, name, ref value);
        }

        /// <summary>
        /// Assigns a 4D vector to the shader parameter with the specified name.
        /// </summary>
        /// <param name="name">Name of the shader parameter.</param>
        /// <param name="value">Value of the parameter.</param>
        public void SetVector4(string name, Vector4 value)
        {
            Internal_SetVector4(mCachedPtr, name, ref value);
        }

        /// <summary>
        /// Assigns a 3x3 matrix to the shader parameter with the specified name.
        /// </summary>
        /// <param name="name">Name of the shader parameter.</param>
        /// <param name="value">Value of the parameter.</param>
        public void SetMatrix3(string name, Matrix3 value)
        {
            Internal_SetMatrix3(mCachedPtr, name, ref value);
        }

        /// <summary>
        /// Assigns a 4x4 matrix to the shader parameter with the specified name.
        /// </summary>
        /// <param name="name">Name of the shader parameter.</param>
        /// <param name="value">Value of the parameter.</param>
        public void SetMatrix4(string name, Matrix4 value)
        {
            Internal_SetMatrix4(mCachedPtr, name, ref value);
        }

        /// <summary>
        /// Assigns a color to the shader parameter with the specified name.
        /// </summary>
        /// <param name="name">Name of the shader parameter.</param>
        /// <param name="value">Value of the parameter.</param>
        public void SetColor(string name, Color value)
        {
            Internal_SetColor(mCachedPtr, name, ref value);
        }

        /// <summary>
        /// Assigns a texture to the shader parameter with the specified name.
        /// </summary>
        /// <param name="name">Name of the shader parameter.</param>
        /// <param name="value">Value of the parameter.</param>
        public void SetTexture(string name, Texture value)
        {
            IntPtr texturePtr = IntPtr.Zero;
            if (value != null)
                texturePtr = value.GetCachedPtr();

            Internal_SetTexture(mCachedPtr, name, texturePtr);
        }
        
        /// <summary>
        /// Returns a float value assigned with the parameter with the specified name.
        /// </summary>
        /// <param name="name">Name of the shader parameter.</param>
        /// <returns>Value of the parameter.</returns>
        public float GetFloat(string name)
        {
            return Internal_GetFloat(mCachedPtr, name);
        }

        /// <summary>
        /// Returns a 2D vector assigned with the parameter with the specified name.
        /// </summary>
        /// <param name="name">Name of the shader parameter.</param>
        /// <returns>Value of the parameter.</returns>
        public Vector2 GetVector2(string name)
        {
            Vector2 value;
            Internal_GetVector2(mCachedPtr, name, out value);
            return value;
        }

        /// <summary>
        /// Returns a 3D vector assigned with the parameter with the specified name.
        /// </summary>
        /// <param name="name">Name of the shader parameter.</param>
        /// <returns>Value of the parameter.</returns>
        public Vector3 GetVector3(string name)
        {
            Vector3 value;
            Internal_GetVector3(mCachedPtr, name, out value);
            return value;
        }

        /// <summary>
        /// Returns a 4D vector assigned with the parameter with the specified name.
        /// </summary>
        /// <param name="name">Name of the shader parameter.</param>
        /// <returns>Value of the parameter.</returns>
        public Vector4 GetVector4(string name)
        {
            Vector4 value;
            Internal_GetVector4(mCachedPtr, name, out value);
            return value;
        }

        /// <summary>
        /// Returns a 3x3 matrix assigned with the parameter with the specified name.
        /// </summary>
        /// <param name="name">Name of the shader parameter.</param>
        /// <returns>Value of the parameter.</returns>
        public Matrix3 GetMatrix3(string name)
        {
            Matrix3 value;
            Internal_GetMatrix3(mCachedPtr, name, out value);
            return value;
        }

        /// <summary>
        /// Returns a 4x4 matrix assigned with the parameter with the specified name.
        /// </summary>
        /// <param name="name">Name of the shader parameter.</param>
        /// <returns>Value of the parameter.</returns>
        public Matrix4 GetMatrix4(string name)
        {
            Matrix4 value;
            Internal_GetMatrix4(mCachedPtr, name, out value);
            return value;
        }

        /// <summary>
        /// Returns a color assigned with the parameter with the specified name.
        /// </summary>
        /// <param name="name">Name of the shader parameter.</param>
        /// <returns>Value of the parameter.</returns>
        public Color GetColor(string name)
        {
            Color value;
            Internal_GetColor(mCachedPtr, name, out value);
            return value;
        }

        /// <summary>
        /// Returns a texture assigned with the parameter with the specified name.
        /// </summary>
        /// <param name="name">Name of the shader parameter.</param>
        /// <returns>Value of the parameter.</returns>
        public Texture GetTexture(string name)
        {
            return Internal_GetTexture(mCachedPtr, name);
        }
        
        /// <summary>
        /// Creates a deep copy of the material.
        /// </summary>
        /// <returns>A new object with exact data as this object.</returns>
        public Material Clone()
        {
            return Internal_Clone(mCachedPtr);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_CreateInstance(Material instance, IntPtr shader);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern Shader Internal_GetShader(IntPtr nativeInstance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetShader(IntPtr nativeInstance, IntPtr shader);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetFloat(IntPtr nativeInstance, string name, float value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetVector2(IntPtr nativeInstance, string name, ref Vector2 value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetVector3(IntPtr nativeInstance, string name, ref Vector3 value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetVector4(IntPtr nativeInstance, string name, ref Vector4 value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetMatrix3(IntPtr nativeInstance, string name, ref Matrix3 value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetMatrix4(IntPtr nativeInstance, string name, ref Matrix4 value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetColor(IntPtr nativeInstance, string name, ref Color value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetTexture(IntPtr nativeInstance, string name, IntPtr value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern float Internal_GetFloat(IntPtr nativeInstance, string name);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_GetVector2(IntPtr nativeInstance, string name, out Vector2 value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_GetVector3(IntPtr nativeInstance, string name, out Vector3 value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_GetVector4(IntPtr nativeInstance, string name, out Vector4 value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_GetMatrix3(IntPtr nativeInstance, string name, out Matrix3 value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_GetMatrix4(IntPtr nativeInstance, string name, out Matrix4 value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_GetColor(IntPtr nativeInstance, string name, out Color value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern Texture Internal_GetTexture(IntPtr nativeInstance, string name);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern Material Internal_Clone(IntPtr nativeInstance);
    }

    /** @} */
}
