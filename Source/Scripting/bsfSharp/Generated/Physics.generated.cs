//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/** @addtogroup Physics
	 *  @{
	 */

	/// <summary>Provides global physics settings, factory methods for physics objects and scene queries.</summary>
	[ShowInInspector]
	public partial class Physics : ScriptObject
	{
		private Physics(bool __dummy0) { }
		protected Physics() { }

		/// <summary>Checks is the physics simulation update currently in progress.</summary>
		[NativeWrapper]
		public static bool IsUpdateInProgress
		{
			get { return Internal__isUpdateInProgress(); }
		}

		/// <summary>
		/// Enables or disables collision between two layers. Each physics object can be assigned a specific layer, and here you 
		/// can determine which layers can interact with each other.
		/// </summary>
		public static void ToggleCollision(ulong groupA, ulong groupB, bool enabled)
		{
			Internal_toggleCollision(groupA, groupB, enabled);
		}

		/// <summary>Checks if two collision layers are allowed to interact.</summary>
		public static bool IsCollisionEnabled(ulong groupA, ulong groupB)
		{
			return Internal_isCollisionEnabled(groupA, groupB);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_toggleCollision(ulong groupA, ulong groupB, bool enabled);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_isCollisionEnabled(ulong groupA, ulong groupB);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal__isUpdateInProgress();
	}

	/** @} */
}
