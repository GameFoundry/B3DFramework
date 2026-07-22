//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace b3d
{
	/// <summary>Options used to control texture compression.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct CompressionOptions
	{
		/// <summary>Initializes the struct with default values.</summary>
		public static CompressionOptions Default()
		{
			CompressionOptions value = new CompressionOptions();
			value.Format = PixelFormat.BC1;
			value.AlphaMode = AlphaMode.None;
			value.IsNormalMap = false;
			value.IsSrgb = false;
			value.Quality = CompressionQuality.Normal;
			value.MaxTileSize = 1024;

			return value;
		}

		public PixelFormat Format;
		public AlphaMode AlphaMode;
		public bool IsNormalMap;
		public bool IsSrgb;
		public CompressionQuality Quality;
		public int MaxTileSize;
	}
}
