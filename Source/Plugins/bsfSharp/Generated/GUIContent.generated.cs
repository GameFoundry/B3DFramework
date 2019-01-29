//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

#if IS_B3D
namespace BansheeEngine
#else
namespace bs
#endif
{
	/** @addtogroup GUI
	 *  @{
	 */

	/// <summary>
	/// Holds data used for displaying content in a GUIElement. Content can consist of a string, image, a tooltip or none  of 
	/// those.
	/// </summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct GUIContent
	{
		/// <summary>Initializes the struct with default values.</summary>
		public static GUIContent Default()
		{
			GUIContent value = new GUIContent();
			value.text = null;
			value.images = new GUIContentImages();
			value.tooltip = null;

			return value;
		}

		/// <summary>Constructs content with just a string.</summary>
		public GUIContent(LocString text)
		{
			this.text = text;
			this.images = new GUIContentImages();
			this.tooltip = null;
		}

		/// <summary>Constructs content with a string and a tooltip.</summary>
		public GUIContent(LocString text, LocString tooltip)
		{
			this.text = text;
			this.images = new GUIContentImages();
			this.tooltip = tooltip;
		}

		/// <summary>Constructs content with just an image.</summary>
		public GUIContent(GUIContentImages image)
		{
			this.text = null;
			this.images = image;
			this.tooltip = null;
		}

		/// <summary>Constructs content with an image and a tooltip.</summary>
		public GUIContent(GUIContentImages image, LocString tooltip)
		{
			this.text = null;
			this.images = image;
			this.tooltip = tooltip;
		}

		/// <summary>Constructs content with a string and an image.</summary>
		public GUIContent(LocString text, GUIContentImages image)
		{
			this.text = text;
			this.images = image;
			this.tooltip = null;
		}

		/// <summary>Constructs content with a string, an image and a tooltip.</summary>
		public GUIContent(LocString text, GUIContentImages image, LocString tooltip)
		{
			this.text = text;
			this.images = image;
			this.tooltip = tooltip;
		}

		public LocString text;
		public GUIContentImages images;
		public LocString tooltip;
	}

	/** @} */
}
