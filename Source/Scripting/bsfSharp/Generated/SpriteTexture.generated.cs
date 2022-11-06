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
	/// Texture that references a part of a larger texture by specifying an UV subset. When the sprite texture is rendererd 
	/// only the portion of the texture specified by the UV subset will be rendered. This allows you to use the same texture 
	/// for multiple sprites (texture atlasing). Sprite textures also allow you to specify sprite sheet animation by varying 
	/// which portion of the UV is selected over time.
	/// </summary>
	[ShowInInspector]
	public partial class SpriteTexture : Resource
	{
		private SpriteTexture(bool __dummy0) { }
		protected SpriteTexture() { }

		/// <summary>Creates a new sprite texture that references the entire area of the provided texture.</summary>
		public SpriteTexture(RRef<Texture> texture)
		{
			Internal_Create(this, texture);
		}

		/// <summary>Creates a new sprite texture that references a sub-area of the provided texture.</summary>
		public SpriteTexture(Vector2 uvOffset, Vector2 uvScale, RRef<Texture> texture)
		{
			Internal_Create0(this, ref uvOffset, ref uvScale, texture);
		}

		/// <summary>Returns a reference wrapper for this resource.</summary>
		public RRef<SpriteTexture> Ref
		{
			get { return Internal_GetRef(mCachedPtr); }
		}

		/// <summary>Determines the internal texture that the sprite texture references.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public RRef<Texture> Texture
		{
			get { return Internal_GetTexture(mCachedPtr); }
			set { Internal_SetTexture(mCachedPtr, value); }
		}

		/// <summary>Returns width of the sprite texture in pixels.</summary>
		[NativeWrapper]
		public int Width
		{
			get { return Internal_GetWidth(mCachedPtr); }
		}

		/// <summary>Returns height of the sprite texture in pixels.</summary>
		[NativeWrapper]
		public int Height
		{
			get { return Internal_GetHeight(mCachedPtr); }
		}

		/// <summary>
		/// Returns width of a single animation frame sprite texture in pixels. If the texture has no animation this is the same 
		/// as getWidth().
		/// </summary>
		[NativeWrapper]
		public int FrameWidth
		{
			get { return Internal_GetFrameWidth(mCachedPtr); }
		}

		/// <summary>
		/// Returns height of a single animation frame sprite texture in pixels. If the texture has no animation this is the same 
		/// as getHeight().
		/// </summary>
		[NativeWrapper]
		public int FrameHeight
		{
			get { return Internal_GetFrameHeight(mCachedPtr); }
		}

		/// <summary>
		/// Determines the offset into the referenced texture where the sprite starts. The offset is in UV coordinates, in range 
		/// [0, 1].
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public Vector2 Offset
		{
			get
			{
				Vector2 temp;
				Internal_GetOffset(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_SetOffset(mCachedPtr, ref value); }
		}

		/// <summary>
		/// Determines the size of the sprite in the referenced texture. Size is in UV coordinates, range [0, 1].
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public Vector2 Scale
		{
			get
			{
				Vector2 temp;
				Internal_GetScale(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_SetScale(mCachedPtr, ref value); }
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
		public static implicit operator RRef<SpriteTexture>(SpriteTexture x)
		{
			if(x != null)
				return Internal_GetRef(x.mCachedPtr);
			else
				return null;
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern RRef<SpriteTexture> Internal_GetRef(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetTexture(IntPtr thisPtr, RRef<Texture> texture);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern RRef<Texture> Internal_GetTexture(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_GetWidth(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_GetHeight(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_GetFrameWidth(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_GetFrameHeight(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetOffset(IntPtr thisPtr, ref Vector2 offset);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetOffset(IntPtr thisPtr, out Vector2 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetScale(IntPtr thisPtr, ref Vector2 scale);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetScale(IntPtr thisPtr, out Vector2 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetAnimation(IntPtr thisPtr, ref SpriteSheetGridAnimation anim);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetAnimation(IntPtr thisPtr, out SpriteSheetGridAnimation __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetAnimationPlayback(IntPtr thisPtr, SpriteAnimationPlayback playback);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern SpriteAnimationPlayback Internal_GetAnimationPlayback(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Create(SpriteTexture managedInstance, RRef<Texture> texture);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Create0(SpriteTexture managedInstance, ref Vector2 uvOffset, ref Vector2 uvScale, RRef<Texture> texture);
	}

	/** @} */
}
