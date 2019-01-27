//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
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
			value.left = 0;
			value.top = 0;
			value.right = 1;
			value.bottom = 1;
			value.front = 0;
			value.back = 1;

			return value;
		}

		public PixelVolume(uint left, uint top, uint right, uint bottom)
		{
			this.left = left;
			this.top = top;
			this.right = right;
			this.bottom = bottom;
			this.front = 0;
			this.back = 1;
		}

		public PixelVolume(uint left, uint top, uint front, uint right, uint bottom, uint back)
		{
			this.left = left;
			this.top = top;
			this.right = right;
			this.bottom = bottom;
			this.front = front;
			this.back = back;
		}

		public uint left;
		public uint top;
		public uint right;
		public uint bottom;
		public uint front;
		public uint back;
	}
}
