//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;
using System.Runtime.CompilerServices;

namespace BansheeEngine
{
    /** @addtogroup GUI-Engine
     *  @{
     */

    /// <summary>
    /// Texture interface that encapsulates underlying texture which allows us to create a sprite texture atlas 
    /// (for example multiple sprite textures referencing different parts of a single texture).
    /// </summary>
    public sealed class SpriteTexture : Resource
    {
        /// <summary>
        /// Constructor for internal use by the runtime.
        /// </summary>
        private SpriteTexture()
        { }

        /// <summary>
        /// Creates a new sprite texture that references the entire area of the provided texture.
        /// </summary>
        /// <param name="texture">Texture to wrap by the sprite texture.</param>
        public SpriteTexture(Texture texture)
        {
            Vector2 offset = Vector2.Zero;
            Vector2 scale = Vector2.One;

            Internal_CreateInstance(this, texture, ref offset, ref scale);
        }

        /// <summary>
        /// Creates a new sprite texture that references a sub-area of the provided texture.
        /// </summary>
        /// <param name="texture">Texture to wrap by the sprite texture.</param>
        /// <param name="uvOffset">Top-left position of the area used by the sprite texture, in normalized coordinates.
        ///                        </param>
        /// <param name="uvScale">Size of the area used by the sprite texture, in normalized coordinates.</param>
        public SpriteTexture(Texture texture, Vector2 uvOffset, Vector2 uvScale)
        {
            Internal_CreateInstance(this, texture, ref uvOffset, ref uvScale);
        }

        /// <summary>
        /// Texture that the sprite texture references.
        /// </summary>
        public Texture Texture
        {
            get { return Internal_GetTexture(mCachedPtr); }
            set
            {
                IntPtr texturePtr = IntPtr.Zero;
                if (value != null)
                    texturePtr = value.GetCachedPtr();

                Internal_SetTexture(mCachedPtr, texturePtr);
            }
        }

        /// <summary>
        /// Offset into the referenced texture where the sprite starts. In UV coordinates, range [0, 1].
        /// </summary>
        public Vector2 Offset
        {
            get { Vector2 value; Internal_GetOffset(mCachedPtr, out value); return value; }
            set { Internal_SetOffset(mCachedPtr, ref value); }
        }

        /// <summary>
        /// Size of the sprite in the referenced texture. In UV coordinates, range [0, 1].
        /// </summary>
        public Vector2 Scale
        {
            get { Vector2 value; Internal_GetScale(mCachedPtr, out value); return value; }
            set { Internal_SetScale(mCachedPtr, ref value); }
        }

        /// <summary>
        /// Returns width of the sprite texture in pixels.
        /// </summary>
        public int Width
        {
            get { return Internal_GetWidth(mCachedPtr); }
        }

        /// <summary>
        /// Returns height of the sprite texture in pixels.
        /// </summary>
        public int Height
        {
            get { return Internal_GetHeight(mCachedPtr); }
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_CreateInstance(SpriteTexture instance, 
            Texture texture, ref Vector2 offset, ref Vector2 scale);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern Texture Internal_GetTexture(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetTexture(IntPtr thisPtr, IntPtr value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_GetOffset(IntPtr thisPtr, out Vector2 value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetOffset(IntPtr thisPtr, ref Vector2 value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_GetScale(IntPtr thisPtr, out Vector2 value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetScale(IntPtr thisPtr, ref Vector2 value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern int Internal_GetWidth(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern int Internal_GetHeight(IntPtr thisPtr);
    }

    /** @} */
}
