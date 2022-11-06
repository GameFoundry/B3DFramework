//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/** @addtogroup GUI_Engine
	 *  @{
	 */

	/// <summary>Describes a single character in a font of a specific size.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct CharDesc
	{
		/// <summary>Character ID, corresponding to a Unicode key.</summary>
		public int CharId;
		/// <summary>Index of the texture the character is located on.</summary>
		public int Page;
		/// <summary>Texture coordinates of the character in the page texture.</summary>
		public float UvX;
		/// <summary>Texture coordinates of the character in the page texture.</summary>
		public float UvY;
		/// <summary>Width/height of the character in texture coordinates.</summary>
		public float UvWidth;
		/// <summary>Width/height of the character in texture coordinates.</summary>
		public float UvHeight;
		/// <summary>Width/height of the character in pixels.</summary>
		public int Width;
		/// <summary>Width/height of the character in pixels.</summary>
		public int Height;
		/// <summary>Offset for the visible portion of the character in pixels.</summary>
		public int XOffset;
		/// <summary>Offset for the visible portion of the character in pixels.</summary>
		public int YOffset;
		/// <summary>Determines how much to advance the pen after writing this character, in pixels.</summary>
		public int XAdvance;
		/// <summary>Determines how much to advance the pen after writing this character, in pixels.</summary>
		public int YAdvance;
		/// <summary>
		/// Pairs that determine if certain character pairs should be closer or father together. for example &quot;AV&quot; 
		/// combination.
		/// </summary>
		public KerningPair[] KerningPairs;
	}

	/** @} */
}
