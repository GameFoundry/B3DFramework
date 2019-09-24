//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/// <summary>Determines the number of samples to take during motion blur filtering.</summary>
	public enum MotionBlurQuality
	{
		/// <summary>4 samples per pixel.</summary>
		VeryLow = 0,
		/// <summary>6 samples per pixel.</summary>
		Low = 1,
		/// <summary>8 samples per pixel.</summary>
		Medium = 2,
		/// <summary>12 samples per pixel.</summary>
		High = 3,
		/// <summary>16 samples per pixel.</summary>
		Ultra = 4
	}
}
