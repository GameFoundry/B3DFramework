//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/** @addtogroup GUI
	 *  @{
	 */

	/// <summary>A GUI element that displays a texture.</summary>
	[ShowInInspector]
	public partial class GUITexture : GUIElement
	{
		private GUITexture(bool __dummy0) { }
		protected GUITexture() { }

		/// <summary>Creates a new GUI element.</summary>
		/// <param name="contents">Structure describing the contents of the GUI element to create.</param>
		/// <param name="styleClass">
		/// Style class that will be used for determining GUI element visuals from the current style sheet. If no class is 
		/// provided, default style is determined based on GUI element type.
		/// </param>
		/// <param name="options">
		/// Additional options that control GUI element size and position. This will override options set in the style sheet.
		/// </param>
		public GUITexture(GUITextureContents contents, string styleClass, params GUIOption[] options)
		{
			Internal_Create(this, ref contents, styleClass, options);
		}

		/// <summary>Creates a new GUI element.</summary>
		/// <param name="contents">Structure describing the contents of the GUI element to create.</param>
		/// <param name="options">
		/// Additional options that control GUI element size and position. This will override options set in the style sheet.
		/// </param>
		public GUITexture(GUITextureContents contents, params GUIOption[] options)
		{
			Internal_Create0(this, ref contents, options);
		}

		/// <summary>Creates a new GUI element.</summary>
		/// <param name="styleClass">
		/// Style class that will be used for determining GUI element visuals from the current style sheet. If no class is 
		/// provided, default style is determined based on GUI element type.
		/// </param>
		/// <param name="options">
		/// Additional options that control GUI element size and position. This will override options set in the style sheet.
		/// </param>
		public GUITexture(string styleClass, params GUIOption[] options)
		{
			Internal_Create1(this, styleClass, options);
		}

		/// <summary>Creates a new GUI element.</summary>
		/// <param name="options">
		/// Additional options that control GUI element size and position. This will override options set in the style sheet.
		/// </param>
		public GUITexture(params GUIOption[] options)
		{
			Internal_Create2(this, options);
		}

		/// <summary>
		/// Changes the active image. If the provided image is null then the image specified by the style will be used.
		/// </summary>
		public void SetImage(RRef<SpriteImage> image)
		{
			Internal_SetImage(mCachedPtr, image);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetImage(IntPtr thisPtr, RRef<SpriteImage> image);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Create(GUITexture managedInstance, ref GUITextureContents contents, string styleClass, params GUIOption[] options);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Create0(GUITexture managedInstance, ref GUITextureContents contents, params GUIOption[] options);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Create1(GUITexture managedInstance, string styleClass, params GUIOption[] options);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Create2(GUITexture managedInstance, params GUIOption[] options);
	}

	/** @} */
}
