//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;
using System.Runtime.CompilerServices;
using BansheeEngine;

namespace BansheeEditor
{
    /** @addtogroup Library
     *  @{
     */

    /// <summary>
    /// Base class for all import options. Allows control over how is a specific resource type imported.
    /// </summary>
    public class ImportOptions : ScriptObject
    {

    }

    /// <summary>
    /// Determines the type of the source image for generating cubemaps.
    /// </summary>
    public enum CubemapSourceType
    {
        /// <summary>
        /// Source is a single image that will be replicated on all cubemap faces.
        /// </summary>
        Single,

		/// <summary>
        /// Source is a list of 6 images, either sequentially next to each other or in a cross format. The system will 
        /// automatically guess the layout and orientation based on the aspect ratio.
        /// </summary>
		Faces,

        /// <summary>
        /// Source is a single spherical panoramic image.
        /// </summary>
        Spherical,

        /// <summary>
        /// Source is a single cylindrical panoramic image.
        /// </summary>
        Cylindrical
    };

    /// <summary>
    /// Provides options for controlling how is a texture resource imported.
    /// </summary>
    public class TextureImportOptions : ImportOptions
    {
        /// <summary>
        /// Creates new texture import options with default values.
        /// </summary>
        public TextureImportOptions()
        {
            Internal_CreateInstance(this);
        }

        /// <summary>
        /// Pixel format to import as.
        /// </summary>
        public PixelFormat Format
        {
            get { return Internal_GetPixelFormat(mCachedPtr); }
            set { Internal_SetPixelFormat(mCachedPtr, value); }
        }

        /// <summary>
        /// Determines whether the imported texture will have mipmaps generated.
        /// </summary>
        public bool GenerateMipmaps
        {
            get { return Internal_GetGenerateMipmaps(mCachedPtr); }
            set { Internal_SetGenerateMipmaps(mCachedPtr, value); }
        }

        /// <summary>
        /// Maximum mipmap level to generate, if mipmap generation is enabled.
        /// </summary>
        public int MaxMipmapLevel
        {
            get { return Internal_GetMaxMipmapLevel(mCachedPtr); }
            set { Internal_SetMaxMipmapLevel(mCachedPtr, value); }
        }

        /// <summary>
        /// Determines whether the texture data is also stored in main memory, available for fast CPU access.
        /// </summary>
        public bool CPUCached
        {
            get { return Internal_GetCPUCached(mCachedPtr); }
            set { Internal_SetCPUCached(mCachedPtr, value); }
        }

        /// <summary>
        /// Determines should the texture data be treated as if its in sRGB (gamma) space. Such texture will be converted by
        /// hardware to linear space before use on the GPU.
        /// </summary>
        public bool IsSRGB
        {
            get { return Internal_GetIsSRGB(mCachedPtr); }
            set { Internal_SetIsSRGB(mCachedPtr, value); }
        }

        /// <summary>
        /// Determines should the texture be imported as a cubemap. See <see cref="CubemapSourceType"/> to choose how will
        /// the source texture be converted to a cubemap.
        /// </summary>
        public bool IsCubemap
        {
            get { return Internal_GetIsCubemap(mCachedPtr); }
            set { Internal_SetIsCubemap(mCachedPtr, value); }
        }

        /// <summary>
        /// Sets a value that determines how should the source texture be interpreted when generating a cubemap. Only
        /// relevant when <see cref="IsCubemap"/> is set to true.
        /// </summary>
        public CubemapSourceType CubemapSourceType
        {
            get { return Internal_GetCubemapSourceType(mCachedPtr); }
            set { Internal_SetCubemapSourceType(mCachedPtr, value); }
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_CreateInstance(TextureImportOptions instance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern PixelFormat Internal_GetPixelFormat(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetPixelFormat(IntPtr thisPtr, PixelFormat value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool Internal_GetGenerateMipmaps(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetGenerateMipmaps(IntPtr thisPtr, bool value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern int Internal_GetMaxMipmapLevel(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetMaxMipmapLevel(IntPtr thisPtr, int value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool Internal_GetCPUCached(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetCPUCached(IntPtr thisPtr, bool value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool Internal_GetIsSRGB(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetIsSRGB(IntPtr thisPtr, bool value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool Internal_GetIsCubemap(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetIsCubemap(IntPtr thisPtr, bool value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern CubemapSourceType Internal_GetCubemapSourceType(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetCubemapSourceType(IntPtr thisPtr, CubemapSourceType value);
    }

    /// <summary>
    /// Information about how to split an AnimationClip into multiple separate clips.
    /// </summary>
    public class AnimationSplitInfo
    {
        public AnimationSplitInfo() { }

        public AnimationSplitInfo(string name, int startFrame, int endFrame, bool isAdditive)
        {
            this.name = name;
            this.startFrame = startFrame;
            this.endFrame = endFrame;
            this.isAdditive = isAdditive;
        }

        public string name;
        public int startFrame = 0;
        public int endFrame = 0;
        public bool isAdditive = false;
    }

    /// <summary>
    /// A set of animation events that will be added to an animation clip during animation import.
    /// </summary>
    public class ImportedAnimationEvents
    {
        public string name;
        public AnimationEvent[] events;
    }
    
    /// <summary>
    /// Provides options for controlling how is a mesh resource imported.
    /// </summary>
    public class MeshImportOptions : ImportOptions
    {
        /// <summary>
        /// Creates new mesh import options with default values.
        /// </summary>
        public MeshImportOptions()
        {
            Internal_CreateInstance(this);
        }

        /// <summary>
        /// Determines whether the mesh data is also stored in main memory, available for fast CPU access. 
        /// </summary>
        public bool CPUCached
        {
            get { return Internal_GetCPUCached(mCachedPtr); }
            set { Internal_SetCPUCached(mCachedPtr, value); }
        }

        /// <summary>
        /// Controls should mesh normals be imported if available.
        /// </summary>
        public bool ImportNormals
        {
            get { return Internal_GetImportNormals(mCachedPtr); }
            set { Internal_SetImportNormals(mCachedPtr, value); }
        }

        /// <summary>
        /// Controls should mesh tangents/bitangents be imported if available.
        /// </summary>
        public bool ImportTangents
        {
            get { return Internal_GetImportTangents(mCachedPtr); }
            set { Internal_SetImportTangents(mCachedPtr, value); }
        }

        /// <summary>
        /// Controls should mesh skin data like bone weights, indices and bind poses be imported if available.
        /// </summary>
        public bool ImportSkin
        {
            get { return Internal_GetImportSkin(mCachedPtr); }
            set { Internal_SetImportSkin(mCachedPtr, value); }
        }

        /// <summary>
        /// Controls should animation clips be imported if available.
        /// </summary>
        public bool ImportAnimation
        {
            get { return Internal_GetImportAnimation(mCachedPtr); }
            set { Internal_SetImportAnimation(mCachedPtr, value); }
        }

        /// <summary>
        /// Controls should mesh blend shapes be imported if available.
        /// </summary>
        public bool ImportBlendShapes
        {
            get { return Internal_GetImportBlendShapes(mCachedPtr); }
            set { Internal_SetImportBlendShapes(mCachedPtr, value); }
        }

        /// <summary>
        /// Uniformly scales the imported mesh by the specified value.
        /// </summary>
        public float Scale
        {
            get { return Internal_GetScale(mCachedPtr); }
            set { Internal_SetScale(mCachedPtr, value); }
        }

        /// <summary>
        /// Determines if keyframe reduction is enabled. Keyframe reduction will reduce the number of key-frames in an
        /// animation clip by removing identical keyframes, and therefore reducing the size of the clip.
        /// </summary>
        public bool KeyframeReduction
        {
            get { return Internal_GetKeyFrameReduction(mCachedPtr); }
            set { Internal_SetKeyFrameReduction(mCachedPtr, value); }
        }

        /// <summary>
        /// Determines if import of root motion curves is enabled. When enabled, any animation curves in imported animations 
        /// affecting the root bone will be available through a set of separate curves in AnimationClip, and they won't be
        /// evaluated through normal animation process. Instead it is expected that the user evaluates the curves manually
        /// and applies them as required.
        /// </summary>
        public bool ImportRootMotion
        {
            get { return Internal_GetRootMotion(mCachedPtr); }
            set { Internal_SetRootMotion(mCachedPtr, value); }
        }

        /// <summary>
        /// Controls what type (if any) of collision mesh should be imported.
        /// </summary>
        public CollisionMeshType CollisionMeshType
        {
            get { return (CollisionMeshType)Internal_GetCollisionMeshType(mCachedPtr); }
            set { Internal_SetCollisionMeshType(mCachedPtr, (int)value); }
        }

        /// <summary>
        /// Split information that allows you to split the animation clip contained in the mesh file into multiple separate
        /// clips. The split always applies to the first clip in the file (if the file contains multiple), other clips are
        /// imported as is.
        /// </summary>
        public AnimationSplitInfo[] AnimationClipSplits
        {
            get { return Internal_GetAnimationClipSplits(mCachedPtr); }
            set { Internal_SetAnimationClipSplits(mCachedPtr, value); }
        }

        /// <summary>
        /// A set of events that will be added to the animation clip, if animation import is enabled.
        /// </summary>
        public ImportedAnimationEvents[] AnimationEvents
        {
            get { return Internal_GetAnimationEvents(mCachedPtr); }
            set { Internal_SetAnimationEvents(mCachedPtr, value); }
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_CreateInstance(MeshImportOptions instance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool Internal_GetCPUCached(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetCPUCached(IntPtr thisPtr, bool value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool Internal_GetImportNormals(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetImportNormals(IntPtr thisPtr, bool value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool Internal_GetImportTangents(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetImportTangents(IntPtr thisPtr, bool value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool Internal_GetImportSkin(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetImportSkin(IntPtr thisPtr, bool value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool Internal_GetImportAnimation(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetImportAnimation(IntPtr thisPtr, bool value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool Internal_GetImportBlendShapes(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetImportBlendShapes(IntPtr thisPtr, bool value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool Internal_GetKeyFrameReduction(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetKeyFrameReduction(IntPtr thisPtr, bool value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool Internal_GetRootMotion(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetRootMotion(IntPtr thisPtr, bool value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern AnimationSplitInfo[] Internal_GetAnimationClipSplits(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetAnimationClipSplits(IntPtr thisPtr, AnimationSplitInfo[] value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern ImportedAnimationEvents[] Internal_GetAnimationEvents(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetAnimationEvents(IntPtr thisPtr, ImportedAnimationEvents[] value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern float Internal_GetScale(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetScale(IntPtr thisPtr, float value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern int Internal_GetCollisionMeshType(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetCollisionMeshType(IntPtr thisPtr, int value);
    }

    /// <summary>
    /// Controls what type of collision mesh should be imported during mesh import.
    /// </summary>
    public enum CollisionMeshType // Note: Must match C++ enum CollisionMeshImport
    {
        /// <summary>
        /// No collision mesh will be imported.
        /// </summary>
        None,
        /// <summary>
        /// Normal triangle mesh will be imported.
        /// </summary>
        Normal,
        /// <summary>
        /// A convex hull will be generated from the source mesh.
        /// </summary>
        Convex
    }

    /// <summary>
    /// Import options that provide various options for controlling how is a font resource imported.
    /// </summary>
    public class FontImportOptions : ImportOptions
    {
        /// <summary>
        /// Creates new font import options with default values.
        /// </summary>
        public FontImportOptions()
        {
            Internal_CreateInstance(this);
        }

        /// <summary>
        /// Font sizes in points that are to be imported.
        /// </summary>
        public int[] FontSizes
        {
            get { return Internal_GetFontSizes(mCachedPtr); }
            set { Internal_SetFontSizes(mCachedPtr, value); }
        }

        /// <summary>
        /// Dots per inch resolution to use when rendering the characters into the texture.
        /// </summary>
        public int DPI
        {
            get { return Internal_GetDPI(mCachedPtr); }
            set { Internal_SetDPI(mCachedPtr, value); }
        }

        /// <summary>
        /// Determines rendering mode used when rendering the characters into the bitmap.
        /// </summary>
        public FontRenderMode RenderMode
        {
            get { return Internal_GetRenderMode(mCachedPtr); }
            set { Internal_SetRenderMode(mCachedPtr, value); }
        }

        /// <summary>
        /// Determines should the characters be rendered in bold style.
        /// </summary>
        public bool Bold
        {
            get { return Internal_GetBold(mCachedPtr); }
            set { Internal_SetBold(mCachedPtr, value); }
        }

        /// <summary>
        /// Determines should the characters be rendered in italic style.
        /// </summary>
        public bool Italic
        {
            get { return Internal_GetItalic(mCachedPtr); }
            set { Internal_SetItalic(mCachedPtr, value); }
        }

        /// <summary>
        /// Determines character ranges to import from the font. Ranges are defined as unicode numbers.
        /// </summary>
        public CharRange[] CharRanges
        {
            get { return Internal_GetCharRanges(mCachedPtr); }
            set { Internal_SetCharRanges(mCachedPtr, value); }
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_CreateInstance(FontImportOptions instance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern int[] Internal_GetFontSizes(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetFontSizes(IntPtr thisPtr, int[] value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern int Internal_GetDPI(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetDPI(IntPtr thisPtr, int value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern FontRenderMode Internal_GetRenderMode(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetRenderMode(IntPtr thisPtr, FontRenderMode value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool Internal_GetBold(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetBold(IntPtr thisPtr, bool value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool Internal_GetItalic(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetItalic(IntPtr thisPtr, bool value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern CharRange[] Internal_GetCharRanges(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetCharRanges(IntPtr thisPtr, CharRange[] value);
    }

    /// <summary>
    /// Provides various options for controlling how is a script file imported.
    /// </summary>
    public class ScriptCodeImportOptions : ImportOptions
    {
        /// <summary>
        /// Creates new script import options with default values.
        /// </summary>
        public ScriptCodeImportOptions()
        {
            Internal_CreateInstance(this);
        }

        /// <summary>
        /// Determines whether the script is editor-only or a normal game script. Editor scripts are compiled in a separate
        /// assembly and may reference editor specific functionality, but are not available in the final game code.
        /// </summary>
        public bool EditorScript
        {
            get { return Internal_IsEditorScript(mCachedPtr); }
            set { Internal_SetEditorScript(mCachedPtr, value); }
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_CreateInstance(ScriptCodeImportOptions instance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool Internal_IsEditorScript(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetEditorScript(IntPtr thisPtr, bool value);
    }

    /// <summary>
    /// Determines how is a font rendered into the bitmap texture.
    /// </summary>
	public enum FontRenderMode // Note: Must match C++ enum FontRenderMode
    {
        /// <summary>Render antialiased fonts without hinting (slightly more blurry).</summary>
		Smooth,
        /// <summary>Render non-antialiased fonts without hinting (slightly more blurry).</summary>
		Raster,
        /// <summary>Render antialiased fonts with hinting.</summary>
		HintedSmooth,
        /// <summary>Render non-antialiased fonts with hinting.</summary>
		HintedRaster
    }

    /// <summary>
    /// Numbers of bits per audio sample.
    /// </summary>
    public enum AudioBitDepth
    {
        Bits8 = 8,
        Bits16 = 16,
        Bits24 = 24,
        Bits32 = 32
    }

    /// <summary>
    /// Provides various options for controlling how is an audio clip file imported.
    /// </summary>
    public class AudioClipImportOptions : ImportOptions
    {
        /// <summary>
        /// Creates new audio clip import options with default values.
        /// </summary>
        public AudioClipImportOptions()
        {
            Internal_CreateInstance(this);
        }

        /// <summary>
        /// Format to import the audio clip as.
        /// </summary>
        public AudioFormat Format
        {
            get { return Internal_GetFormat(mCachedPtr); }
            set { Internal_SetFormat(mCachedPtr, value); }
        }

        /// <summary>
        /// Determines how is audio data loaded into memory.
        /// </summary>
        public AudioReadMode ReadMode
        {
            get { return Internal_GetReadMode(mCachedPtr); }
            set { Internal_SetReadMode(mCachedPtr, value); }
        }

        /// <summary>
        /// Determines will the clip be played as spatial (3D) audio or as normal audio.
        /// </summary>
        public bool Is3D
        {
            get { return Internal_GetIs3D(mCachedPtr); }
            set { Internal_SetIs3D(mCachedPtr, value); }
        }

        /// <summary>
        /// Size of a single sample in bits.
        /// </summary>
        public AudioBitDepth BitDepth
        {
            get
            {
                int bits = Internal_GetBitDepth(mCachedPtr);
                switch (bits)
                {
                    case 8:
                        return AudioBitDepth.Bits8;
                    case 16:
                        return AudioBitDepth.Bits16;
                    case 24:
                        return AudioBitDepth.Bits24;
                    case 32:
                        return AudioBitDepth.Bits32;
                }

                return AudioBitDepth.Bits16;
            }

            set { Internal_SetBitDepth(mCachedPtr, (int)value); }
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_CreateInstance(AudioClipImportOptions instance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern AudioFormat Internal_GetFormat(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetFormat(IntPtr thisPtr, AudioFormat format);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern AudioReadMode Internal_GetReadMode(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetReadMode(IntPtr thisPtr, AudioReadMode readMode);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool Internal_GetIs3D(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetIs3D(IntPtr thisPtr, bool is3d);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern int Internal_GetBitDepth(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetBitDepth(IntPtr thisPtr, int bitDepth);
    }

    /** @} */
}
