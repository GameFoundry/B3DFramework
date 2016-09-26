//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;
using System.Runtime.CompilerServices;
using BansheeEngine;

namespace BansheeEditor
{
    /** @addtogroup Utility-Editor
     *  @{
     */

    /// <summary>
    /// Container and functionality to creating and storing differences between two managed objects of the same type.
    /// </summary>
    public class SerializedDiff : ScriptObject
    {
        /// <summary>
        /// Creates a difference of properties that are different in <paramref name="newObj"/> compared to 
        /// <paramref name="oldObj"/>. The difference can later be applied to <paramref name="oldObj"/> to restore it to
        /// the same state as <paramref name="newObj"/> .
        /// </summary>
        /// <param name="oldObj">Object to compare from.</param>
        /// <param name="newObj">Object to compare to.</param>
        /// <returns>A set of differences between the two objects.</returns>
        public static SerializedDiff Create(object oldObj, object newObj)
        {
            SerializedObject serializedOldObj = SerializedObject.Create(oldObj);
            return Create(serializedOldObj, newObj);
        }

        /// <summary>
        /// Creates a difference of properties that are different in <paramref name="newObj"/> compared to 
        /// <paramref name="oldObj"/>. The difference can later be applied to <paramref name="oldObj"/> to restore it to
        /// the same state as <paramref name="newObj"/> .
        /// </summary>
        /// <param name="oldObj">Object to compare from.</param>
        /// <param name="newObj">Object to compare to.</param>
        /// <returns>A set of differences between the two objects.</returns>
        public static SerializedDiff Create(SerializedObject oldObj, object newObj)
        {
            if (oldObj == null || newObj == null)
                return null;

            IntPtr oldObjPtr = oldObj.GetCachedPtr();
            return Internal_CreateDiff(oldObjPtr, newObj);
        }

        /// <summary>
        /// Applies difference stored in this object to the provided object. The type of the object must be the same as the
        /// type of objects the difference was generated from.
        /// </summary>
        /// <param name="obj">Object to apply the difference to.</param>
        public void Apply(object obj)
        {
            if (obj == null)
                return;

            Internal_ApplyDiff(mCachedPtr, obj);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern SerializedDiff Internal_CreateDiff(IntPtr oldObj, object newObj);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_ApplyDiff(IntPtr thisPtr, object obj);
    }

    /** @} */
}
