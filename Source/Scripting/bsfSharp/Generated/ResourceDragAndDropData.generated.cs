//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/// <summary>Information about a single or multiple dragged resources, represented as paths.</summary>
	[ShowInInspector]
	public partial class ResourceDragAndDropData : DragAndDropData
	{
		private ResourceDragAndDropData(bool __dummy0) { }

		public ResourceDragAndDropData()
		{
			Internal_ResourceDragAndDropData(this);
		}

		public ResourceDragAndDropData(string resourcePath)
		{
			Internal_ResourceDragAndDropData0(this, resourcePath);
		}

		public ResourceDragAndDropData(string[] resourcePaths)
		{
			Internal_ResourceDragAndDropData1(this, resourcePaths);
		}

		[ShowInInspector]
		[NativeWrapper]
		public string[] ResourcePaths
		{
			get { return Internal_GetResourcePaths(mCachedPtr); }
			set { Internal_SetResourcePaths(mCachedPtr, value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_ResourceDragAndDropData(ResourceDragAndDropData managedInstance);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_ResourceDragAndDropData0(ResourceDragAndDropData managedInstance, string resourcePath);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_ResourceDragAndDropData1(ResourceDragAndDropData managedInstance, string[] resourcePaths);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern string[] Internal_GetResourcePaths(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetResourcePaths(IntPtr thisPtr, string[] value);
	}
}
