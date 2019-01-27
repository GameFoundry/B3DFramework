//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;

namespace BansheeEngine
{
    /** @addtogroup Resources
     *  @{
     */

    /// <summary>
    /// Provides a handle to a <see cref="Resource"/>. The underlying resource might or might not be loaded.
    /// </summary>
    public class RRefBase : ScriptObject
    {
        /// <summary>
        /// Constructor for internal use only.
        /// </summary>
        protected RRefBase() { }

        /// <summary>
        /// Checks is the referenced resource loaded.
        /// </summary>
        public bool IsLoaded => Internal_IsLoaded(mCachedPtr);

        /// <summary>
        /// Returns the referenced resource. If the resource hasn't been loaded it will be loaded as if calling
        /// <see cref="Resources.Load{T}(string,ResourceLoadFlag)"/> using default settings.
        /// </summary>
        public Resource GenericValue => Internal_GetResource(mCachedPtr);

        /// <summary>
        /// Returns the UUID of the resource this handle is referencing.
        /// </summary>
        public UUID UUID
        {
            get
            {
                Internal_GetUUID(mCachedPtr, out var uuid);
                return uuid;
            }
        }

        /// <summary>
        /// Casts the generic resource reference type to a specific resource reference type.
        /// </summary>
        /// <typeparam name="T">Type of resource to cast the reference to.</typeparam>
        /// <returns>New reference type.</returns>
        public RRef<T> As<T>() where T : Resource
        {
            return (RRef<T>)Internal_CastAs(mCachedPtr, typeof(T));
        }

        /// <inheritdoc/>
        public override bool Equals(object other)
        {
            if (ReferenceEquals(other, null))
                return false;

            if (!(other is RRefBase))
                return false;

            RRefBase otherRef = (RRefBase)other;

            Internal_GetUUID(mCachedPtr, out var lhs);
            Internal_GetUUID(otherRef.mCachedPtr, out var rhs);

            return lhs.Equals(rhs);
        }

        public bool Equals(RRefBase other)
        {
            if (ReferenceEquals(other, null))
                return false;

            Internal_GetUUID(mCachedPtr, out var lhs);
            Internal_GetUUID(other.mCachedPtr, out var rhs);

            return lhs.Equals(rhs);
        }

        public static bool operator==(RRefBase lhs, RRefBase rhs)
        {
            if (ReferenceEquals(lhs, rhs))
                return true;

            if (ReferenceEquals(lhs, null))
                return false;

            if (ReferenceEquals(rhs, null))
                return false;

            Internal_GetUUID(lhs.mCachedPtr, out var lhsUUID);
            Internal_GetUUID(rhs.mCachedPtr, out var rhsUUID);

            return lhsUUID.Equals(rhsUUID);
        }

        public static bool operator!=(RRefBase lhs, RRefBase rhs)
        {
            return !(lhs == rhs);
        }

        /// <inheritdoc/>
        public override int GetHashCode()
        {
            return UUID.GetHashCode();
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool Internal_IsLoaded(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        protected static extern Resource Internal_GetResource(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_GetUUID(IntPtr thisPtr, out UUID uuid);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern RRefBase Internal_CastAs(IntPtr thisPtr, Type type);
    }

    /// <summary>
    /// Provides a handle to a <see cref="Resource"/>. The underlying resource might or might not be loaded.
    /// </summary>
    /// <typeparam name="T">Resource type that the handle references</typeparam>
    public class RRef<T> : RRefBase where T : Resource
    {
        /// <summary>
        /// Returns the referenced resource. If the resource hasn't been loaded it will be loaded as if calling
        /// <see cref="Resources.Load{T}(string,ResourceLoadFlag)"/> using default settings.
        /// </summary>
        public T Value => (T) GenericValue;
    }

    /** @} */
}
