//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/// <summary>Projection type to use by the camera.</summary>
	public enum ProjectionType
	{
		/// <summary>Projection type where object size remains constant and parallel lines remain parallel.</summary>
		Orthographic = 0,
		/// <summary>Projection type that emulates human vision. Objects farther away appear smaller.</summary>
		Perspective = 1
	}
}
