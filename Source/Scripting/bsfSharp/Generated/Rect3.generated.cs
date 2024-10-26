//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/// <summary>
	/// Represents a rectangle in three dimensional space. It is represented by two axes that extend from the specified 
	/// origin. Axes should be perpendicular to each other and they extend in both positive and negative directions from the 
	/// origin by the amount specified by extents.
	/// </summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct Rect3
	{
		/// <summary>Initializes the struct with default values.</summary>
		public static Rect3 Default()
		{
			Rect3 value = new Rect3();
			value.Center = Vector3.Default();
			value.HorizontalAxis = Vector3.Default();
			value.VerticalAxis = Vector3.Default();
			value.HorizontalExtent = 0f;
			value.VerticalExtent = 0f;

			return value;
		}

		public Vector3 Center;
		public Vector3 HorizontalAxis;
		public Vector3 VerticalAxis;
		public float HorizontalExtent;
		public float VerticalExtent;
	}
}
