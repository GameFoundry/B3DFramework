//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/** @addtogroup Physics
	 *  @{
	 */

	[ShowInInspector]
	public partial class CharacterController : Component
	{
		private CharacterController(bool __dummy0) { }
		protected CharacterController() { }

		[NativeWrapper]
		public Vector3 FootPosition
		{
			get
			{
				Vector3 temp;
				Internal_GetFootPosition(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_SetFootPosition(mCachedPtr, ref value); }
		}

		[ShowInInspector]
		[NativeWrapper]
		public float Radius
		{
			get { return Internal_GetRadius(mCachedPtr); }
			set { Internal_SetRadius(mCachedPtr, value); }
		}

		[ShowInInspector]
		[NativeWrapper]
		public float Height
		{
			get { return Internal_GetHeight(mCachedPtr); }
			set { Internal_SetHeight(mCachedPtr, value); }
		}

		[ShowInInspector]
		[NativeWrapper]
		public Vector3 Up
		{
			get
			{
				Vector3 temp;
				Internal_GetUp(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_SetUp(mCachedPtr, ref value); }
		}

		[ShowInInspector]
		[NativeWrapper]
		public CharacterClimbingMode ClimbingMode
		{
			get { return Internal_GetClimbingMode(mCachedPtr); }
			set { Internal_SetClimbingMode(mCachedPtr, value); }
		}

		[ShowInInspector]
		[NativeWrapper]
		public CharacterNonWalkableMode NonWalkableMode
		{
			get { return Internal_GetNonWalkableMode(mCachedPtr); }
			set { Internal_SetNonWalkableMode(mCachedPtr, value); }
		}

		[ShowInInspector]
		[NativeWrapper]
		public float MinMoveDistance
		{
			get { return Internal_GetMinMoveDistance(mCachedPtr); }
			set { Internal_SetMinMoveDistance(mCachedPtr, value); }
		}

		[ShowInInspector]
		[NativeWrapper]
		public float ContactOffset
		{
			get { return Internal_GetContactOffset(mCachedPtr); }
			set { Internal_SetContactOffset(mCachedPtr, value); }
		}

		[ShowInInspector]
		[NativeWrapper]
		public float StepOffset
		{
			get { return Internal_GetStepOffset(mCachedPtr); }
			set { Internal_SetStepOffset(mCachedPtr, value); }
		}

		[ShowInInspector]
		[Range(0f, 180f, true)]
		[NativeWrapper]
		public Radian SlopeLimit
		{
			get
			{
				Radian temp;
				Internal_GetSlopeLimit(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_SetSlopeLimit(mCachedPtr, ref value); }
		}

		[ShowInInspector]
		[LayerMask]
		[NativeWrapper]
		public ulong Layer
		{
			get { return Internal_GetLayer(mCachedPtr); }
			set { Internal_SetLayer(mCachedPtr, value); }
		}

		public event Action<ControllerColliderCollision> OnColliderHit;

		public event Action<ControllerControllerCollision> OnControllerHit;

		public CharacterCollisionFlag Move(Vector3 displacement)
		{
			return Internal_Move(mCachedPtr, ref displacement);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern CharacterCollisionFlag Internal_Move(IntPtr thisPtr, ref Vector3 displacement);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetFootPosition(IntPtr thisPtr, out Vector3 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetFootPosition(IntPtr thisPtr, ref Vector3 position);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetRadius(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetRadius(IntPtr thisPtr, float radius);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetHeight(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetHeight(IntPtr thisPtr, float height);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetUp(IntPtr thisPtr, out Vector3 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetUp(IntPtr thisPtr, ref Vector3 up);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern CharacterClimbingMode Internal_GetClimbingMode(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetClimbingMode(IntPtr thisPtr, CharacterClimbingMode mode);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern CharacterNonWalkableMode Internal_GetNonWalkableMode(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetNonWalkableMode(IntPtr thisPtr, CharacterNonWalkableMode mode);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetMinMoveDistance(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetMinMoveDistance(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetContactOffset(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetContactOffset(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetStepOffset(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetStepOffset(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetSlopeLimit(IntPtr thisPtr, out Radian __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetSlopeLimit(IntPtr thisPtr, ref Radian value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ulong Internal_GetLayer(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetLayer(IntPtr thisPtr, ulong layer);
		private void Internal_OnColliderHit(ref ControllerColliderCollision p0)
		{
			OnColliderHit?.Invoke(p0);
		}
		private void Internal_OnControllerHit(ref ControllerControllerCollision p0)
		{
			OnControllerHit?.Invoke(p0);
		}
	}

	/** @} */
}
