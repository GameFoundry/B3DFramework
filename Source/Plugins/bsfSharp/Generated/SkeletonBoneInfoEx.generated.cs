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
	/// <summary>Contains internal information about a single bone in a Skeleton.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct BoneInfo
	{
		/// <summary>Unique name of the bone.</summary>
		public string name;
		/// <summary>Index of the parent bone (within the relevant Skeleton object). -1 if root bone.</summary>
		public int parent;
		/// <summary>Inverse transform of the pose the skeleton was initially created in.</summary>
		public Matrix4 invBindPose;
	}
}
