//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/** @addtogroup Rendering
	 *  @{
	 */

	/// <summary>
	/// Image that references a part of a texture by specifying an UV range. When the sprite image is rendered only the 
	/// portion of the texture specified by the UV range will be rendered.
	///
	/// Sprite images also allow you to specify sprite sheet animation by varying which portion of the UV is selected over 
	/// time.
	/// </summary>
	[ShowInInspector]
	public partial class SpriteImage : Resource
	{
		private SpriteImage(bool __dummy0) { }
		protected SpriteImage() { }

		/// <summary>Returns a reference wrapper for this resource.</summary>
		public RRef<SpriteImage> Ref
		{
			get { return Internal_GetRef(mCachedPtr); }
		}

		/// <summary>
		/// Returns the pixel size of the UV subrange covered in the texture atlas. If the image includes animation, this will 
		/// return the size of the entire animation grid.
		/// </summary>
		[NativeWrapper]
		public TSize2<int> Size
		{
			get
			{
				TSize2<int> temp;
				Internal_GetSize(mCachedPtr, out temp);
				return temp;
			}
		}

		/// <summary>
		/// Returns the size of a single animation frame in pixels. If the texture has no animation this is the same as 
		/// GetWidth().
		/// </summary>
		[NativeWrapper]
		public TSize2<int> AnimationFrameSize
		{
			get
			{
				TSize2<int> temp;
				Internal_GetAnimationFrameSize(mCachedPtr, out temp);
				return temp;
			}
		}

		/// <summary>Retrieves the atlas texture where the image is stored.</summary>
		[NativeWrapper]
		public RRef<Texture> Texture
		{
			get { return Internal_GetAtlasTexture(mCachedPtr); }
		}

		/// <summary>Determines the UV range that the image is referencing.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public Rect2 UVRange
		{
			get
			{
				Rect2 temp;
				Internal_GetUVRange(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_SetUVRange(mCachedPtr, ref value); }
		}

		/// <summary>
		/// Sets properties describing sprite animation. The animation splits the sprite area into a grid of sub-images which can 
		/// be evaluated over time. In order to view the animation you must also enable playback through setAnimationPlayback().
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public SpriteSheetGridAnimation Animation
		{
			get
			{
				SpriteSheetGridAnimation temp;
				Internal_GetAnimation(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_SetAnimation(mCachedPtr, ref value); }
		}

		/// <summary>Determines if and how should the sprite animation play.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public SpriteAnimationPlayback AnimationPlayback
		{
			get { return Internal_GetAnimationPlayback(mCachedPtr); }
			set { Internal_SetAnimationPlayback(mCachedPtr, value); }
		}

		/// <summary>Returns a reference wrapper for this resource.</summary>
		public static implicit operator RRef<SpriteImage>(SpriteImage x)
		{
			if(x != null)
				return Internal_GetRef(x.mCachedPtr);
			else
				return null;
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern RRef<SpriteImage> Internal_GetRef(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetSize(IntPtr thisPtr, out TSize2<int> __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetAnimationFrameSize(IntPtr thisPtr, out TSize2<int> __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern RRef<Texture> Internal_GetAtlasTexture(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetUVRange(IntPtr thisPtr, ref Rect2 uvRange);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetUVRange(IntPtr thisPtr, out Rect2 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetAnimation(IntPtr thisPtr, ref SpriteSheetGridAnimation animation);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetAnimation(IntPtr thisPtr, out SpriteSheetGridAnimation __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetAnimationPlayback(IntPtr thisPtr, SpriteAnimationPlayback playback);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern SpriteAnimationPlayback Internal_GetAnimationPlayback(IntPtr thisPtr);
	}

	/** @} */
}
