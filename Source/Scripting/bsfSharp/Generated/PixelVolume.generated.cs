//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/// <summary>Represents a 3D region of pixels used for referencing pixel data.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct PixelVolume
	{
		/// <summary>Initializes the struct with default values.</summary>
		public static PixelVolume Default()
		{
			PixelVolume value = new PixelVolume();
			value.Left = 0;
			value.Top = 0;
			value.Right = 1;
			value.Bottom = 1;
			value.Front = 0;
			value.Back = 1;

			return value;
		}

		public PixelVolume(int left, int top, int right, int bottom)
		{
			this.Left = left;
			this.Top = top;
			this.Right = right;
			this.Bottom = bottom;
			this.Front = 0;
			this.Back = 1;
		}

		public PixelVolume(int left, int top, int front, int right, int bottom, int back)
		{
			this.Left = left;
			this.Top = top;
			this.Right = right;
			this.Bottom = bottom;
			this.Front = front;
			this.Back = back;
		}

		public int Left;
		public int Top;
		public int Right;
		public int Bottom;
		public int Front;
		public int Back;
	}
}
