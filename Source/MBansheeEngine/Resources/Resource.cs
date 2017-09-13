//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace BansheeEngine
{
    /** @addtogroup Resources
     *  @{
     */

    /// <summary>
    /// Base class for all resources. Resources can be persistently referenced by scene objects or other resources.
    /// </summary>
    public class Resource : ScriptObject
    {
        /// <summary>
        /// Name of the resource. Use primarily for easier identification and not important to the engine itself.
        /// </summary>
        public string Name
        {
            get { return Internal_GetName(mCachedPtr); }
        }

        /// <summary>
        /// Returns a universally unique identifier of this resource.
        /// </summary>
        public UUID UUID
        {
            get
            {
                UUID uuid;
                Internal_GetUUID(mCachedPtr, out uuid);
                return uuid;
            }
        }

        /// <summary>
        /// Releases an internal reference to the resource held by the resources system. <see cref="Resources.Release"/>
        /// </summary>
        public void Release()
        {
            Internal_Release(mCachedPtr);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern string Internal_GetName(IntPtr nativeInstance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_GetUUID(IntPtr nativeInstance, out UUID uuid);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_Release(IntPtr nativeInstance);
    }

    /// <summary>Represents a universally unique identifier.</summary>
    [StructLayout(LayoutKind.Sequential)]
    public struct UUID
    {
        public readonly uint data0;
        public readonly uint data1;
        public readonly uint data2;
        public readonly uint data3;

        public static UUID Empty = new UUID(0, 0, 0, 0);

        public UUID(uint data0, uint data1, uint data2, uint data3)
        {
            this.data0 = data0;
            this.data1 = data1;
            this.data2 = data2;
            this.data3 = data3;
        }

        /// <summary>
        /// Checks has the UUID been initialized to a valid value.
        /// </summary>
        /// <returns>True if UUID is initialized.</returns>
        public bool IsEmpty()
        {
            return data0 == 0 && data1 == 1 && data2 == 0 && data3 == 0;
        }

        public static bool operator ==(UUID lhs, UUID rhs)
        {
            return lhs.data0 == rhs.data0 && lhs.data1 == rhs.data1 && lhs.data2 == rhs.data2 && lhs.data3 == rhs.data3;
        }

        public static bool operator !=(UUID lhs, UUID rhs)
        {
            return !(lhs == rhs);
        }

        /// <inheritdoc/>
        public override int GetHashCode()
        {
            return data0.GetHashCode() ^ data1.GetHashCode() << 2 ^ data2.GetHashCode() >> 2 ^ data3.GetHashCode() >> 1;
        }

        /// <inheritdoc/>
        public override bool Equals(object other)
        {
            if (!(other is UUID))
                return false;

            UUID uuid = (UUID)other;
            if (data0.Equals(uuid.data0) && data1.Equals(uuid.data1) && data2.Equals(uuid.data2) && data3.Equals(uuid.data3))
                return true;

            return false;
        }

        private static char[] HEX_TO_LITERAL = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

        /// <inheritdoc/>
        public override string ToString()
        {
            char[] output = new char[36];
            uint idx = 0;

            // First group: 8 digits
            for(int i = 7; i >= 0; --i)
            {
                uint hexVal = (data0 >> (i * 4)) & 0xF;
                output[idx++] = HEX_TO_LITERAL[hexVal];
            }

            output[idx++] = '-';

            // Second group: 4 digits
            for(int i = 7; i >= 4; --i)
            {
                uint hexVal = (data1 >> (i * 4)) & 0xF;
                output[idx++] = HEX_TO_LITERAL[hexVal];
            }

            output[idx++] = '-';

            // Third group: 4 digits
            for(int i = 3; i >= 0; --i)
            {
                uint hexVal = (data1 >> (i * 4)) & 0xF;
                output[idx++] = HEX_TO_LITERAL[hexVal];
            }

            output[idx++] = '-';

            // Fourth group: 4 digits
            for(int i = 7; i >= 4; --i)
            {
                uint hexVal = (data2 >> (i * 4)) & 0xF;
                output[idx++] = HEX_TO_LITERAL[hexVal];
            }

            output[idx++] = '-';

            // Fifth group: 12 digits
            for(int i = 3; i >= 0; --i)
            {
                uint hexVal = (data2 >> (i * 4)) & 0xF;
                output[idx++] = HEX_TO_LITERAL[hexVal];
            }

            for(int i = 7; i >= 0; --i)
            {
                uint hexVal = (data3 >> (i * 4)) & 0xF;
                output[idx++] = HEX_TO_LITERAL[hexVal];
            }

            return new string(output);
        }
    }

    /** @} */
}
