//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;

#if IS_B3D
namespace BansheeEngine
#else
namespace bs
#endif
{
    /** @addtogroup GUI_Engine
     *  @{
     */

    /// <summary>
    /// Allows you to display a render texture in the GUI. Has the same functionality as GUITexture, but also forwards any 
    /// input to underlying GUI elements being rendered on the provided render texture.
    /// </summary>
    public sealed class GUIRenderTexture : GUIElement
    {
        private RenderTexture renderTexture;

        /// <summary>
        /// Render texture that is displayed on the GUI element.
        /// </summary>
        public RenderTexture RenderTexture
        {
            get
            {
                return renderTexture;
            }

            set
            {
                IntPtr texturePtr = IntPtr.Zero;
                if (value != null)
                    texturePtr = value.GetCachedPtr();

                renderTexture = value;
                Internal_SetTexture(mCachedPtr, texturePtr);
            }
        }

        /// <summary>
        /// Creates a new render texture element.
        /// </summary>
        /// <param name="texture">Render texture to display in the element.</param>
        /// <param name="transparent">Determines should the texture be rendered with transparency active.</param>
        /// <param name="style">Optional style to use for the element. Style controls the look of the element, as well as 
        ///                     default layout options. Style will be retrieved from the active GUISkin. If not specified 
        ///                     default element style is used.</param>
        /// <param name="options">Options that allow you to control how is the element  positioned and sized. This will 
        ///                       override any similar options set by style.</param>
        public GUIRenderTexture(RenderTexture texture, bool transparent, string style, params GUIOption[] options)
        {
            IntPtr texturePtr = IntPtr.Zero;
            if (texture != null)
                texturePtr = texture.GetCachedPtr();

            Internal_CreateInstance(this, texturePtr, transparent, style, options);
        }

        /// <summary>
        /// Creates a new render texture element.
        /// </summary>
        /// <param name="texture">Render texture to display in the element.</param>
        /// <param name="transparent">Determines should the texture be rendered with transparency active.</param>
        /// <param name="options">Options that allow you to control how is the element  positioned and sized. This will 
        ///                       override any similar options set by style.</param>
        public GUIRenderTexture(RenderTexture texture, bool transparent, params GUIOption[] options)
        {
            IntPtr texturePtr = IntPtr.Zero;
            if (texture != null)
                texturePtr = texture.GetCachedPtr();

            Internal_CreateInstance(this, texturePtr, transparent, "", options);
        }

        /// <summary>
        /// Creates a new render texture element.
        /// </summary>
        /// <param name="texture">Render texture to display in the element.</param>
        /// <param name="style">Optional style to use for the element. Style controls the look of the element, as well as 
        ///                     default layout options. Style will be retrieved from the active GUISkin. If not specified 
        ///                     default element style is used.</param>
        /// <param name="options">Options that allow you to control how is the element  positioned and sized. This will 
        ///                       override any similar options set by style.</param>
        public GUIRenderTexture(RenderTexture texture, string style, params GUIOption[] options)
        {
            IntPtr texturePtr = IntPtr.Zero;
            if (texture != null)
                texturePtr = texture.GetCachedPtr();

            Internal_CreateInstance(this, texturePtr, false, style, options);
        }

        /// <summary>
        /// Creates a new render texture element.
        /// </summary>
        /// <param name="texture">Render texture to display in the element.</param>
        /// <param name="options">Options that allow you to control how is the element  positioned and sized. This will 
        ///                       override any similar options set by style.</param>
        public GUIRenderTexture(RenderTexture texture, params GUIOption[] options)
        {
            IntPtr texturePtr = IntPtr.Zero;
            if (texture != null)
                texturePtr = texture.GetCachedPtr();

            Internal_CreateInstance(this, texturePtr, false, "", options);
        }

        /// <summary>
        /// Colors the element with a specific tint.
        /// </summary>
        /// <param name="color">Tint to apply to the element.</param>
        public void SetTint(Color color)
        {
            Internal_SetTint(mCachedPtr, ref color);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_CreateInstance(GUIRenderTexture instance, IntPtr texture, bool transparency,
            string style, GUIOption[] options);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetTexture(IntPtr nativeInstance, IntPtr texture);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetTint(IntPtr nativeInstance, ref Color color);
    }

    /** @} */
}
