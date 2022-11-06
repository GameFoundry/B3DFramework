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
	/// Descriptor that describes a simple sprite sheet animation. The parent texture is split into a grid of <see 
	/// cref="numRows"/> x <see cref="numColumns"/>, each representing one frame of the animation. Every frame is of equal 
	/// size. Frames are sequentially evaluated starting from the top-most row, iterating over all columns in a row and then 
	/// moving to next row, up to <see cref="count"/> frames. Frames in rows/colums past <see cref="count"/>. <see 
	/// cref="fps"/> frames are evaluated every second, allowing you to control animation speed.
	/// </summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct SpriteSheetGridAnimation
	{
		/// <summary>Initializes the struct with default values.</summary>
		public static SpriteSheetGridAnimation Default()
		{
			SpriteSheetGridAnimation value = new SpriteSheetGridAnimation();
			value.NumRows = 1;
			value.NumColumns = 1;
			value.Count = 1;
			value.Fps = 8;

			return value;
		}

		public SpriteSheetGridAnimation(int numRows, int numColumns, int count, int fps)
		{
			this.NumRows = numRows;
			this.NumColumns = numColumns;
			this.Count = count;
			this.Fps = fps;
		}

		/// <summary>
		/// Number of rows to divide the parent&apos;s texture area. Determines height of the individual frame (depends on parent 
		/// texture size).
		/// </summary>
		public int NumRows;
		/// <summary>
		/// Number of columns to divide the parent&apos;s texture area. Determines column of the individual frame (depends on 
		/// parent texture size).
		/// </summary>
		public int NumColumns;
		/// <summary>
		/// Number of frames in the animation. Must be less or equal than <see cref="numRows"/> * <see cref="numColumns"/>.
		/// </summary>
		public int Count;
		/// <summary>How many frames to evaluate each second. Determines the animation speed.</summary>
		public int Fps;
	}

	/** @} */
}
