//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
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
			Internal_create(this, texture);
		}

		/// <summary>Creates a new sprite texture that references a sub-area of the provided texture.</summary>
		public SpriteTexture(Vector2 uvOffset, Vector2 uvScale, RRef<Texture> texture)
		{
			Internal_create0(this, ref uvOffset, ref uvScale, texture);
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
			get { return Internal_getTexture(mCachedPtr); }
			set { Internal_setTexture(mCachedPtr, value); }
		}

		/// <summary>Returns width of the sprite texture in pixels.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public uint Width
		{
			get { return Internal_getWidth(mCachedPtr); }
		}

		/// <summary>Returns height of the sprite texture in pixels.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public uint Height
		{
			get { return Internal_getHeight(mCachedPtr); }
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
				Internal_getOffset(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_setOffset(mCachedPtr, ref value); }
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
				Internal_getScale(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_setScale(mCachedPtr, ref value); }
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
				Internal_getAnimation(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_setAnimation(mCachedPtr, ref value); }
		}

		/// <summary>Determines if and how should the sprite animation play.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public SpriteAnimationPlayback AnimationPlayback
		{
			get { return Internal_getAnimationPlayback(mCachedPtr); }
			set { Internal_setAnimationPlayback(mCachedPtr, value); }
		}

		/// <summary>Returns a reference wrapper for this resource.</summary>
		public static implicit operator RRef<SpriteTexture>(SpriteTexture x)
		{ return Internal_GetRef(x.mCachedPtr); }

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern RRef<SpriteTexture> Internal_GetRef(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setTexture(IntPtr thisPtr, RRef<Texture> texture);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern RRef<Texture> Internal_getTexture(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern uint Internal_getWidth(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern uint Internal_getHeight(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setOffset(IntPtr thisPtr, ref Vector2 offset);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getOffset(IntPtr thisPtr, out Vector2 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setScale(IntPtr thisPtr, ref Vector2 scale);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getScale(IntPtr thisPtr, out Vector2 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setAnimation(IntPtr thisPtr, ref SpriteSheetGridAnimation anim);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getAnimation(IntPtr thisPtr, out SpriteSheetGridAnimation __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setAnimationPlayback(IntPtr thisPtr, SpriteAnimationPlayback playback);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern SpriteAnimationPlayback Internal_getAnimationPlayback(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_create(SpriteTexture managedInstance, RRef<Texture> texture);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_create0(SpriteTexture managedInstance, ref Vector2 uvOffset, ref Vector2 uvScale, RRef<Texture> texture);
	}

	/** @} */
}
