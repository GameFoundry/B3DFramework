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

	/// <summary>Structure describing contents of a GUITexture element.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct GUITextureContents
	{
		/// <summary>Initializes the struct with default values.</summary>
		public static GUITextureContents Default()
		{
			GUITextureContents value = new GUITextureContents();
			value.Image = null;
			value.ScaleMode = TextureScaleMode.StretchToFit;
			value.IsTransparent = true;

			return value;
		}

		public GUITextureContents(SpriteImage image, TextureScaleMode scaleMode = TextureScaleMode.StretchToFit, bool isTransparent = true)
		{
			this.Image = image;
			this.ScaleMode = scaleMode;
			this.IsTransparent = isTransparent;
		}

		/// <summary>Image to display. If this is null then the image specified by the style will be used.</summary>
		public SpriteImage Image;
		/// <summary>Scale mode to use when sizing the texture.</summary>
		public TextureScaleMode ScaleMode;
		/// <summary>Determines should the texture be rendered with transparency active.</summary>
		public bool IsTransparent;
	}

	/** @} */
}
