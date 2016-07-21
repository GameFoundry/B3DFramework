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
    /// A GUI element that allows the user to draw custom graphics. All drawn elements relative to the canvas, to its origin
    /// in the top left corner.
    /// </summary>
    public sealed class GUICanvas : GUIElement
    {
        /// <summary>
        /// Creates a new canvas element.
        /// </summary>
        /// <param name="style">Optional style to use for the element. Style controls the look of the element, as well as 
        ///                     default layout options. Style will be retrieved from the active GUISkin. If not specified 
        ///                     default element style is used.</param>
        /// <param name="options">Options that allow you to control how is the element  positioned and sized. This will 
        ///                       override any similar options set by style.</param>
        public GUICanvas(string style, params GUIOption[] options)
        {
            Internal_CreateInstance(this, style, options);
        }

        /// <summary>
        /// Creates a new canvas element.
        /// </summary>
        /// <param name="style">Optional style to use for the element. Style controls the look of the element, as well as 
        ///                     default layout options. Style will be retrieved from the active GUISkin. If not specified 
        ///                     default element style is used.</param>
        public GUICanvas(string style = "")
        {
            Internal_CreateInstance(this, style, new GUIOption[0]);
        }

        /// <summary>
        /// Creates a new canvas element.
        /// </summary>
        /// <param name="options">Options that allow you to control how is the element positioned and sized. This will 
        ///                       override any similar options set by style.</param>
        public GUICanvas(params GUIOption[] options)
        {
            Internal_CreateInstance(this, "", options);
        }

        /// <summary>
        /// Draws a line going from <paramref name="a"/> to <paramref name="b"/>.
        /// </summary>
        /// <param name="a">Starting point of the line, relative to the canvas origin (top-left).</param>
        /// <param name="b">Ending point of the line, relative to the canvas origin (top-left).</param>
        /// <param name="depth">Depth at which to draw the element. Elements with higher depth will be drawn before others.
        ///                     Additionally elements of the same type (triangle or line) will be drawn in order they are
        ///                     submitted if they share the same depth.</param>
        public void DrawLine(Vector2I a, Vector2I b, byte depth = 128)
        {
            Color color = Color.White;
            Internal_DrawLine(mCachedPtr, ref a, ref b, ref color, depth);
        }

        /// <summary>
        /// Draws a line going from <paramref name="a"/> to <paramref name="b"/>.
        /// </summary>
        /// <param name="a">Starting point of the line, relative to the canvas origin (top-left).</param>
        /// <param name="b">Ending point of the line, relative to the canvas origin (top-left).</param>
        /// <param name="color">Color of the line.</param>
        /// <param name="depth">Depth at which to draw the element. Elements with higher depth will be drawn before others.
        ///                     Additionally elements of the same type (triangle or line) will be drawn in order they are
        ///                     submitted if they share the same depth.</param>
        public void DrawLine(Vector2I a, Vector2I b, Color color, byte depth = 128)
        {
            Internal_DrawLine(mCachedPtr, ref a, ref b, ref color, depth);
        }

        /// <summary>
        /// Draws multiple lines following the path by the provided vertices. First vertex connects to the second vertex,
        /// and every following vertex connects to the previous vertex.
        /// </summary>
        /// <param name="vertices">Points to use for drawing the line. Must have at least two elements. All points are 
        ///                        relative to the canvas origin(top-left).</param>
        /// <param name="depth">Depth at which to draw the element. Elements with higher depth will be drawn before others.
        ///                     Additionally elements of the same type (triangle or line) will be drawn in order they are
        ///                     submitted if they share the same depth.</param>
        public void DrawPolyLine(Vector2I[] vertices, byte depth = 128)
        {
            Color color = Color.White;
            Internal_DrawPolyLine(mCachedPtr, vertices, ref color, depth);
        }

        /// <summary>
        /// Draws multiple lines following the path by the provided vertices. First vertex connects to the second vertex,
        /// and every following vertex connects to the previous vertex.
        /// </summary>
        /// <param name="vertices">Points to use for drawing the line. Must have at least two elements. All points are 
        ///                        relative to the canvas origin(top-left).</param>
        /// <param name="color">Color of the line.</param>
        /// <param name="depth">Depth at which to draw the element. Elements with higher depth will be drawn before others.
        ///                     Additionally elements of the same type (triangle or line) will be drawn in order they are
        ///                     submitted if they share the same depth.</param>
        public void DrawPolyLine(Vector2I[] vertices, Color color, byte depth = 128)
        {
            Internal_DrawPolyLine(mCachedPtr, vertices, ref color, depth);
        }

        /// <summary>
        /// Draws a quad with a the provided texture displayed.
        /// </summary>
        /// <param name="texture">Texture to draw.</param>
        /// <param name="area">Position and size of the texture to draw. Position is relative to the canvas origin 
        ///                    (top-left). If size is zero, the default texture size will be used.</param>
        /// <param name="scaleMode">Scale mode to use when sizing the texture. Only relevant if the provided quad size 
        ///                         doesn't match the texture size.</param>
        /// <param name="depth">Depth at which to draw the element. Elements with higher depth will be drawn before others.
        ///                     Additionally elements of the same type (triangle or line) will be drawn in order they are
        ///                     submitted if they share the same depth.</param>
        public void DrawTexture(SpriteTexture texture, Rect2I area,
            GUITextureScaleMode scaleMode = GUITextureScaleMode.StretchToFit, byte depth = 128)
        {
            IntPtr texturePtr = IntPtr.Zero;
            if (texture != null)
                texturePtr = texture.GetCachedPtr();

            Color color = Color.White;
            Internal_DrawTexture(mCachedPtr, texturePtr, ref area, scaleMode, ref color, depth);
        }

        /// <summary>
        /// Draws a quad with a the provided texture displayed.
        /// </summary>
        /// <param name="texture">Texture to draw.</param>
        /// <param name="area">Position and size of the texture to draw. Position is relative to the canvas origin 
        ///                    (top-left). If size is zero, the default texture size will be used.</param>
        /// <param name="color">Color to tint the drawn texture with.</param>
        /// <param name="scaleMode">Scale mode to use when sizing the texture. Only relevant if the provided quad size 
        ///                         doesn't match the texture size.</param>
        /// <param name="depth">Depth at which to draw the element. Elements with higher depth will be drawn before others.
        ///                     Additionally elements of the same type (triangle or line) will be drawn in order they are
        ///                     submitted if they share the same depth.</param>
        public void DrawTexture(SpriteTexture texture, Rect2I area, Color color,
            GUITextureScaleMode scaleMode = GUITextureScaleMode.StretchToFit, byte depth = 128)
        {
            IntPtr texturePtr = IntPtr.Zero;
            if (texture != null)
                texturePtr = texture.GetCachedPtr();

            Internal_DrawTexture(mCachedPtr, texturePtr, ref area, scaleMode, ref color, depth);
        }

        /// <summary>
        /// Draws a triangle strip. First three vertices are used to form the initial triangle, and every next vertex will
        /// form a triangle with the previous two.
        /// </summary>
        /// <param name="vertices">A set of points defining the triangles. Must have at least three elements. All points
        ///                        are relative to the canvas origin(top-left).</param>
        /// <param name="depth">Depth at which to draw the element. Elements with higher depth will be drawn before others.
        ///                     Additionally elements of the same type (triangle or line) will be drawn in order they are
        ///                     submitted if they share the same depth.</param>
        public void DrawTriangleStrip(Vector2I[] vertices, byte depth = 128)
        {
            Color color = Color.White;
            Internal_DrawTriangleStrip(mCachedPtr, vertices, ref color, depth);
        }

        /// <summary>
        /// Draws a triangle strip. First three vertices are used to form the initial triangle, and every next vertex will
        /// form a triangle with the previous two.
        /// </summary>
        /// <param name="vertices">A set of points defining the triangles. Must have at least three elements. All points
        ///                        are relative to the canvas origin(top-left).</param>
        /// <param name="color">Color of the triangles.</param>
        /// <param name="depth">Depth at which to draw the element. Elements with higher depth will be drawn before others.
        ///                     Additionally elements of the same type (triangle or line) will be drawn in order they are
        ///                     submitted if they share the same depth.</param>
        public void DrawTriangleStrip(Vector2I[] vertices, Color color, byte depth = 128)
        {
            Internal_DrawTriangleStrip(mCachedPtr, vertices, ref color, depth);
        }

        /// <summary>
        /// Draws a triangle list. Every three vertices in the list represent a unique triangle.
        /// </summary>
        /// <param name="vertices">A set of points defining the triangles. Must have at least three elements, and its size
        ///                        must be a multiple of three.</param>
        /// <param name="depth">Depth at which to draw the element. Elements with higher depth will be drawn before others.
        ///                     Additionally elements of the same type (triangle or line) will be drawn in order they are
        ///                     submitted if they share the same depth.</param>
        public void DrawTriangleList(Vector2I[] vertices, byte depth = 128)
        {
            Color color = Color.White;
            Internal_DrawTriangleList(mCachedPtr, vertices, ref color, depth);
        }

        /// <summary>
        /// Draws a triangle list. Every three vertices in the list represent a unique triangle.
        /// </summary>
        /// <param name="vertices">A set of points defining the triangles. Must have at least three elements, and its size
        ///                        must be a multiple of three.</param>
        /// <param name="color">Color of the triangles.</param>
        /// <param name="depth">Depth at which to draw the element. Elements with higher depth will be drawn before others.
        ///                     Additionally elements of the same type (triangle or line) will be drawn in order they are
        ///                     submitted if they share the same depth.</param>
        public void DrawTriangleList(Vector2I[] vertices, Color color, byte depth = 128)
        {
            Internal_DrawTriangleList(mCachedPtr, vertices, ref color, depth);
        }

        /// <summary>
        /// Draws a piece of text with the wanted font. The text will be aligned to the top-left corner of the provided
        /// position, and will not be word wrapped.
        /// </summary>
        /// <param name="text">Text to draw.</param>
        /// <param name="position">Position of the text to draw. This represents the top-left corner of the text. It is
        ///                        relative to the canvas origin(top-left).</param>
        /// <param name="font">Font to draw the text with.</param>
        /// <param name="color">Color of the text.</param>
        /// <param name="size">Size of the font.</param>
        /// <param name="depth">Depth at which to draw the element. Elements with higher depth will be drawn before others.
        ///                     Additionally elements of the same type (triangle or line) will be drawn in order they are
        ///                     submitted if they share the same depth.</param>
        public void DrawText(string text, Vector2I position, Font font, Color color, int size = 10, byte depth = 128)
        {
            IntPtr fontPtr = IntPtr.Zero;
            if (font != null)
                fontPtr = font.GetCachedPtr();

            Internal_DrawText(mCachedPtr, text, ref position, fontPtr, size, ref color, depth);
        }

        /// <summary>
        /// Draws a piece of text with the wanted font. The text will be aligned to the top-left corner of the provided
        /// position, and will not be word wrapped.
        /// </summary>
        /// <param name="text">Text to draw.</param>
        /// <param name="position">Position of the text to draw. This represents the top-left corner of the text. It is
        ///                        relative to the canvas origin(top-left).</param>
        /// <param name="font">Font to draw the text with.</param>
        /// <param name="size">Size of the font.</param>
        /// <param name="depth">Depth at which to draw the element. Elements with higher depth will be drawn before others.
        ///                     Additionally elements of the same type (triangle or line) will be drawn in order they are
        ///                     submitted if they share the same depth.</param>
        public void DrawText(string text, Vector2I position, Font font, int size = 10, byte depth = 128)
        {
            IntPtr fontPtr = IntPtr.Zero;
            if (font != null)
                fontPtr = font.GetCachedPtr();

            Color color = Color.White;
            Internal_DrawText(mCachedPtr, text, ref position, fontPtr, size, ref color, depth);
        }

        /// <summary>
        /// Clears the canvas, removing any previously drawn elements.
        /// </summary>
        public void Clear()
        {
            Internal_Clear(mCachedPtr);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_CreateInstance(GUICanvas instance, string style, GUIOption[] options);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_DrawLine(IntPtr nativeInstance, ref Vector2I a, ref Vector2I b,
            ref Color color, byte depth);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_DrawPolyLine(IntPtr nativeInstance, Vector2I[] vertices,
            ref Color color, byte depth);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_DrawTexture(IntPtr nativeInstance, IntPtr texture, ref Rect2I area,
            GUITextureScaleMode scaleMode, ref Color color, byte depth);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_DrawTriangleStrip(IntPtr nativeInstance, Vector2I[] vertices, ref Color color, 
            byte depth);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_DrawTriangleList(IntPtr nativeInstance, Vector2I[] vertices, ref Color color, 
            byte depth);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_DrawText(IntPtr nativeInstance, string text, ref Vector2I position,
            IntPtr font, int size, ref Color color, byte depth);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_Clear(IntPtr nativeInstance);
    }

    /** @} */
}
