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

		public PixelVolume(int left, int top, int right, int bottom)
		{
			this.left = left;
			this.top = top;
			this.right = right;
			this.bottom = bottom;
			this.front = 0;
			this.back = 1;
		}

		public PixelVolume(int left, int top, int front, int right, int bottom, int back)
		{
			this.left = left;
			this.top = top;
			this.right = right;
			this.bottom = bottom;
			this.front = front;
			this.back = back;
		}

		public int left;
		public int top;
		public int right;
		public int bottom;
		public int front;
		public int back;
	}
}
