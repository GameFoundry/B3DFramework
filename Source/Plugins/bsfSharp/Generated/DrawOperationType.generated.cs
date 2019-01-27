//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/** @addtogroup Rendering
	 *  @{
	 */

	/// <summary>Describes operation that will be used for rendering a certain set of vertices.</summary>
	public enum MeshTopology
	{
		/// <summary>Each vertex represents a point.</summary>
		PointList = 1,
		/// <summary>Each sequential pair of vertices represent a line.</summary>
		LineList = 2,
		/// <summary>Each vertex (except the first) forms a line with the previous vertex.</summary>
		LineStrip = 3,
		/// <summary>Each sequential 3-tuple of vertices represent a triangle.</summary>
		TriangleList = 4,
		/// <summary>Each vertex (except the first two) form a triangle with the previous two vertices.</summary>
		TriangleStrip = 5,
		/// <summary>Each vertex (except the first two) form a triangle with the first vertex and previous vertex.</summary>
		TriangleFan = 6
	}

	/** @} */
}
