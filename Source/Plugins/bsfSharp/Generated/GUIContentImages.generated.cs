//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/** @addtogroup GUI
	 *  @{
	 */

	/// <summary>Contains separate GUI content images for every possible GUI element state.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct GUIContentImages
	{
		/// <summary>Initializes the struct with default values.</summary>
		public static GUIContentImages Default()
		{
			GUIContentImages value = new GUIContentImages();
			value.normal = null;
			value.hover = null;
			value.active = null;
			value.focused = null;
			value.normalOn = null;
			value.hoverOn = null;
			value.activeOn = null;
			value.focusedOn = null;

			return value;
		}

		public GUIContentImages(SpriteTexture image)
		{
			this.normal = image;
			this.hover = image;
			this.active = image;
			this.focused = image;
			this.normalOn = image;
			this.hoverOn = image;
			this.activeOn = image;
			this.focusedOn = image;
		}

		public SpriteTexture normal;
		public SpriteTexture hover;
		public SpriteTexture active;
		public SpriteTexture focused;
		public SpriteTexture normalOn;
		public SpriteTexture hoverOn;
		public SpriteTexture activeOn;
		public SpriteTexture focusedOn;
	}

	/** @} */
}
