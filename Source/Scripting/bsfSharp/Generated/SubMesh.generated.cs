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

	/// <summary>Data about a sub-mesh range and the type of primitives contained in the range.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct SubMesh
	{
		/// <summary>Initializes the struct with default values.</summary>
		public static SubMesh Default()
		{
			SubMesh value = new SubMesh();
			value.IndexOffset = 0;
			value.IndexCount = 0;
			value.DrawOp = MeshTopology.TriangleList;

			return value;
		}

		public SubMesh(int indexOffset, int indexCount, MeshTopology drawOp)
		{
			this.IndexOffset = indexOffset;
			this.IndexCount = indexCount;
			this.DrawOp = drawOp;
		}

		public int IndexOffset;
		public int IndexCount;
		public MeshTopology DrawOp;
	}

	/** @} */
}
