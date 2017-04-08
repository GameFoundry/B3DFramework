//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace BansheeEngine
{
    /** @addtogroup GUI-Engine
     *  @{
     */

    /// <summary>
    /// Font resource containing data about textual characters and how to render text. Contains one or multiple font 
    /// bitmaps, each for a specific size.
    /// </summary>
    public sealed class Font : Resource
    {
        /// <summary>
        /// Creates a new font resource. For runtime use only.
        /// </summary>
        private Font()
        { }

        /// <summary>
        /// Returns font bitmap for a specific font size.
        /// </summary>
        /// <param name="size">Size of the bitmap in points.</param>
        /// <returns>Bitmap object if size was found, null otherwise.</returns>
        public FontBitmap GetBitmap(int size)
        {
            return Internal_GetBitmap(mCachedPtr, size);
        }

        /// <summary>
        /// Finds the available font bitmap size closest to the provided size.
        /// </summary>
        /// <param name="size">Size of the bitmap in points.</param>
        /// <returns>Nearest available bitmap size.</returns>
        public int GetClosestSize(int size)
        {
            return Internal_GetClosestSize(mCachedPtr, size);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern FontBitmap Internal_GetBitmap(IntPtr instance, int size);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern int Internal_GetClosestSize(IntPtr instance, int size);
    }

    /// <summary>
    /// Contains textures and data about every character for a bitmap font of a specific size.
    /// </summary>
    public sealed class FontBitmap : ScriptObject
    {
        /// <summary>
        /// Constructor for internal runtime use only.
        /// </summary>
        internal FontBitmap()
        { }

        /// <summary>
        /// Size of the font in the bitmap, in points.
        /// </summary>
        public int Size { get { return Internal_GetSize(mCachedPtr); } }

        /// <summary>
        /// Y offset to the baseline on which the characters are placed, in pixels.
        /// </summary>
        public int BaselineOffset { get { return Internal_GetBaselineOffset(mCachedPtr); } }

        /// <summary>
        /// Height of a single line of the font, in pixels.
        /// </summary>
        public int LineHeight { get { return Internal_GetLineHeight(mCachedPtr); } }

        /// <summary>
        /// Width of a space in pixels.
        /// </summary>
        public int SpaceWidth { get { return Internal_GetSpaceWidth(mCachedPtr); } }

        /// <summary>
        /// Character to use when data for a character is missing.
        /// </summary>
        public CharDesc MissingChar { get { CharDesc value; Internal_GetMissingChar(mCachedPtr, out value); return value; } }

        /// <summary>
        /// Textures in which the character's pixels are stored.
        /// </summary>
        public Texture[] Pages { get { return Internal_GetPages(mCachedPtr); } }

        /// <summary>
        /// Returns a description of the character in the bitmap.
        /// </summary>
        /// <param name="id">Unicode key of the character to retrieve.</param>
        /// <returns>Character data if it was found in the bitmap, missing character data otherwise.</returns>
        public CharDesc GetChar(int id)
        {
            CharDesc value;
            Internal_GetChar(mCachedPtr, id, out value);
            return value;
        }

        /// <summary>
        /// Returns a set of pairs that determine if the provided character should be closer or father together than normal
        /// with a specific other character. For example the combination of "A" and "V" characters is normally a kerning
        /// pair "AV" as their bounds overlap and are closer together than characters would be normally.
        /// </summary>
        /// <param name="id">Unicode key of the character to retrieve kerning pairs for.</param>
        /// <returns>A set of kerning pairs for the character.</returns>
        public KerningPair[] GetKerning(int id)
        {
            return Internal_GetKerning(mCachedPtr, id);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern int Internal_GetSize(IntPtr instance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern int Internal_GetBaselineOffset(IntPtr instance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern int Internal_GetLineHeight(IntPtr instance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern int Internal_GetSpaceWidth(IntPtr instance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_GetMissingChar(IntPtr instance, out CharDesc output);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern Texture[] Internal_GetPages(IntPtr instance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_GetChar(IntPtr instance, int id, out CharDesc output);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern KerningPair[] Internal_GetKerning(IntPtr instance, int id);
    }

    /// <summary>
    /// Marks a range of characters in a font.
    /// </summary>
    [StructLayout(LayoutKind.Sequential), SerializeObject]
    public struct CharRange
    {
        public int start;
        public int end;
    }

    /// <summary>
    /// Kerning pair representing larger or smaller offset between a specific pair of characters.
    /// </summary>
    [StructLayout(LayoutKind.Sequential), SerializeObject]
    public struct KerningPair // Note: Must match C++ struct KerningPair
    {
        public int OtherChar;
        public int Amount;
    }

    /// <summary>
    /// Describes a single character in a font of a specific size.
    /// </summary>
    [StructLayout(LayoutKind.Sequential), SerializeObject]
    public struct CharDesc // Note: Must match C++ struct ScriptCharDesc
    {
        /// <summary>Character ID, corresponding to a Unicode key.</summary>
        public int Id;

        /// <summary>Index of the texture the character is located on.</summary>
        public int Page; 

        /// <summary>Texture coordinates of the character in the page texture.</summary>
        public float UVX;

        /// <summary>Texture coordinates of the character in the page texture.</summary>
        public float UVY;

        /// <summary>Size of the character in texture coordinates.</summary>
        public float UVWidth;

        /// <summary>Size of the character in texture coordinates.</summary>
        public float UVHeight;

        /// <summary>Size of the character in pixels.</summary>
        public int Width;

        /// <summary>Size of the character in pixels.</summary>
        public int Height;

        /// <summary>Offset for the visible portion of the character in pixels.</summary>
        public int XOffset;

        /// <summary>Offset for the visible portion of the character in pixels.</summary>
        public int YOffset;

        /// <summary>Determines how much to advance the pen after writing this character, in pixels.</summary>
        public int XAdvance;

        /// <summary>Determines how much to advance the pen after writing this character, in pixels.</summary>
        public int YAdvance;
    }

    /** @} */
}
