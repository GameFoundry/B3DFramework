//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.CompilerServices;

namespace bs
{
    /** @addtogroup Serialization
     *  @{
     */

    /// <summary>
    /// Allows you to transparently retrieve and set values of an entry, whether that entry is an object field or
    /// an array/list/dictionary entry.
    /// </summary>
    public sealed class SerializableProperty : ScriptObject
    {
        /// <summary>
        /// Object field types support by the serializable property.
        /// </summary>
        public enum FieldType
        {
            Int,
            Float,
            Bool,
            String,
            Color,
            Vector2,
            Vector3,
            Vector4,
            GameObjectRef,
            Resource,
            Object,
            Array,
            List,
            Dictionary,
            RRef,
            ColorGradient,
            Curve,
            FloatDistribution,
            ColorDistribution,
            Quaternion,
            Enum,
            Vector2Distribution,
            Vector3Distribution,
            ColorGradientHDR
        }

        public delegate object Getter();
        public delegate void Setter(object value);

        private FieldType type;
        private Type internalType;
        private Getter getter;
        private Setter setter;
        private bool applyOnChildChanges;

        /// <summary>
        /// Returns type of data the property contains.
        /// </summary>
        public FieldType Type
        {
            get { return type; }
        }

        /// <summary>
        /// Returns type of data the property contains, as C# type.
        /// </summary>
        public Type InternalType
        {
            get { return internalType; }
        }

        /// <summary>
        /// Is the containing type a value type (true), or a reference type (false).
        /// </summary>
        public bool IsValueType
        {
            get { return internalType.IsValueType; }
        }

        /// <summary>
        /// Only relevant if the property represents an object. When true, whenever a child property changes value,
        /// this property's setter will be called with the new modified version of the object. This is useful for
        /// value-types, for reference types being passed by copy, or for situations where changes to an object must be
        /// explicitly applied by re-assigning the object.
        /// </summary>
        public bool ApplyOnChildChanges
        {
            get { return applyOnChildChanges; }
        }

        /// <summary>
        /// Constructor for internal use by the native code.
        /// </summary>
        private SerializableProperty()
        { }

        /// <summary>
        /// Creates a new serializable property.
        /// </summary>
        /// <param name="type">Type of data the property contains.</param>
        /// <param name="internalType">Type of data the property contains, as C# type.</param>
        /// <param name="getter">Method that allows you to retrieve contents of the property.</param>
        /// <param name="setter">Method that allows you to set contents of the property</param>
        /// <param name="applyOnChildChanges">
        /// Only relevant if the property represents an object. When true, whenever a child property changes value,
        /// this property's setter will be called with the new modified version of the object. This is useful for
        /// value-types, for reference types being passed by copy, or for situations where changes to an object must be
        /// explicitly applied by re-assigning the object.</param>
        public SerializableProperty(FieldType type, Type internalType, Getter getter, Setter setter,
            bool applyOnChildChanges = false)
        {
            this.type = type;
            this.internalType = internalType;
            this.getter = getter;
            this.setter = setter;
            this.applyOnChildChanges = applyOnChildChanges;

            Internal_CreateInstance(this, internalType);
        }

        /// <summary>
        /// Finalizes construction of the serializable property. Must be called after creation.
        /// </summary>
        /// <param name="type">Type of data the property contains.</param>
        /// <param name="internalType">Type of data the property contains, as C# type.</param>
        /// <param name="getter">Method that allows you to retrieve contents of the property.</param>
        /// <param name="setter">Method that allows you to set contents of the property</param>
        /// <param name="applyOnChildChanges">
        /// Only relevant if the property represents an object. When true, whenever a child property changes value,
        /// this property's setter will be called with the new modified version of the object. This is useful for
        /// value-types, for reference types being passed by copy, or for situations where changes to an object must be
        /// explicitly applied by re-assigning the object.</param>
        internal void Construct(FieldType type, Type internalType, Getter getter, Setter setter,
            bool applyOnChildChanges = false)
        {
            this.type = type;
            this.internalType = internalType;
            this.getter = getter;
            this.setter = setter;
            this.applyOnChildChanges = applyOnChildChanges;
        }

        /// <summary>
        /// Creates a serializable property with a custom getter and setter callbacks.
        /// </summary>
        /// <typeparam name="T">Type returned/set by getter/setter.</typeparam>
        /// <param name="getter">Callback that returns the current value of the property.</param>
        /// <param name="setter">Callback that assigns a new value to the properly.</param>
        /// <returns>New property instance with the specified callbacks.</returns>
        public static SerializableProperty Create<T>(Func<T> getter, Action<T> setter)
        {
            var output = new SerializableProperty(
                DetermineFieldType(typeof(T)), typeof(T), () => getter(), x => setter((T)x));

            return output;
        }

        /// <summary>
        /// Retrieves the value contained in the property.
        /// </summary>
        /// <typeparam name="T">Type of the value to retrieve. Caller must ensure the type matches the property type.</typeparam>
        /// <returns>Value of the property.</returns>
        public T GetValue<T>()
        {
            // Cast if possible
            if (typeof(T) != internalType)
            {
                // Note: Not checking cast operators
                if (internalType.IsPrimitive || internalType.IsEnum)
                {
                    if (internalType == typeof(bool) || typeof(T) == typeof(bool))
                        throw new Exception("Attempted to retrieve a serializable value using an invalid type. " +
                            "Provided type: " + typeof(T) + ". Needed type: " + internalType);

                    return (T) Convert.ChangeType(getter(), typeof(T));
                }

                if(!typeof(T).IsAssignableFrom(internalType))
                    throw new Exception("Attempted to retrieve a serializable value using an invalid type. " +
                        "Provided type: " + typeof(T) + ". Needed type: " + internalType);
            }

            return (T)getter();
        }

        /// <summary>
        /// Changes the value of the property.
        /// </summary>
        /// <typeparam name="T">Type of the value to set. Caller must ensure the type matches the property type.</typeparam>
        /// <param name="value">New value to assign to the property.</param>
        public void SetValue<T>(T value)
        {
            // Cast if possible
            if (typeof(T) != internalType)
            {
                // Note: Not checking cast operators
                if (internalType.IsPrimitive || internalType.IsEnum)
                {
                    if (internalType == typeof(bool) || typeof(T) == typeof(bool))
                        throw new Exception("Attempted to set a serializable value using an invalid type. " +
                            "Provided type: " + typeof(T) + ". Needed type: " + internalType);

                   setter((T) Convert.ChangeType(value, typeof(T)));
                   return;
                }

                if (!typeof(T).IsAssignableFrom(internalType))
                    throw new Exception("Attempted to set a serializable value using an invalid type. " +
                        "Provided type: " + typeof(T) + ". Needed type: " + internalType);
            }

            setter(value);
        }

        /// <summary>
        /// Returns a serializable object wrapper around the value contained in the property.
        /// </summary>
        /// <returns>Serializable object wrapper around the value contained in the property.</returns>
        public SerializableObject GetObject()
        {
            if (type != FieldType.Object)
                throw new Exception("Attempting to retrieve object information from a field that doesn't contain an object.");

            // Get the most-derived type so the serializable object lists the exact fields
            object obj = GetValue<object>();
            Type actualType = obj != null ? obj.GetType() : internalType;

            return Internal_CreateObject(mCachedPtr, this, actualType);
        }

        /// <summary>
        /// Returns a serializable array around the value contained in the property. Caller must ensure the property
        /// contains an array.
        /// </summary>
        /// <returns>Serializable array around the value contained in the property.</returns>
        public SerializableArray GetArray()
        {
            if (type != FieldType.Array)
                throw new Exception("Attempting to retrieve array information from a field that doesn't contain an array.");

            return Internal_CreateArray(mCachedPtr, this);
        }

        /// <summary>
        /// Returns a serializable list around the value contained in the property. Caller must ensure the property
        /// contains a list.
        /// </summary>
        /// <returns>Serializable list around the value contained in the property.</returns>
        public SerializableList GetList()
        {
            if (type != FieldType.List)
                throw new Exception("Attempting to retrieve array information from a field that doesn't contain a list.");

            return Internal_CreateList(mCachedPtr, this);
        }

        /// <summary>
        /// Returns a serializable dictionary around the value contained in the property. Caller must ensure the property
        /// contains a dictionary.
        /// </summary>
        /// <returns>Serializable dictionary around the value contained in the property.</returns>
        public SerializableDictionary GetDictionary()
        {
            if (type != FieldType.Dictionary)
                throw new Exception("Attempting to retrieve array information from a field that doesn't contain a dictionary.");

            return Internal_CreateDictionary(mCachedPtr, this);
        }

        /// <summary>
        /// Creates a new instance of the type wrapped by this property.
        /// </summary>
        /// <typeparam name="T">Type of the object to create. Caller must ensure the type matches the property type and that
        ///                     the property wraps an object.</typeparam>
        /// <returns>A new instance of an object of type <typeparamref name="T"/>.</returns>
        public T CreateObjectInstance<T>()
        {
            if (type != FieldType.Object)
                throw new Exception("Attempting to retrieve object information from a field that doesn't contain an object.");

            return (T)Internal_CreateManagedObjectInstance(mCachedPtr);
        }

        /// <summary>
        /// Creates a new instance of the array wrapped by this property. Caller must ensure this property contains an array.
        /// </summary>
        /// <param name="lengths">Size of each dimension of the array. Number of dimensions must match the number
        ///                       of dimensions in the array wrapped by this property.</param>
        /// <returns>A new array containing the same element type as the array wrapped by this property, of
        ///          <paramref name="lengths"/> sizes.</returns>
        public Array CreateArrayInstance(int[] lengths)
        {
            if (type != FieldType.Array)
                throw new Exception("Attempting to retrieve array information from a field that doesn't contain an array.");

            return Internal_CreateManagedArrayInstance(mCachedPtr, lengths);
        }

        /// <summary>
        /// Creates a new instance of the list wrapped by this property. Caller must ensure this property contains a list.
        /// </summary>
        /// <param name="length">Size of the list.</param>
        /// <returns>A new list containing the same element type as the list wrapped by this property, of
        ///          <paramref name="length"/> size.</returns>
        public IList CreateListInstance(int length)
        {
            if (type != FieldType.List)
                throw new Exception("Attempting to retrieve array information from a field that doesn't contain a list.");

            return Internal_CreateManagedListInstance(mCachedPtr, length);
        }

        /// <summary>
        /// Creates a new instance of the dictionary wrapped by this property. Caller must ensure this property contains
        /// a dictionary.
        /// </summary>
        /// <returns>A new dictionary containing the same key/value types as the dictionary wrapped by this property.
        ///          </returns>
        public IDictionary CreateDictionaryInstance()
        {
            if (type != FieldType.Dictionary)
                throw new Exception("Attempting to retrieve array information from a field that doesn't contain a dictionary.");

            return Internal_CreateManagedDictionaryInstance(mCachedPtr);
        }

        /// <summary>
        /// Helper method used for finding child properties of the specified property, using a property path.
        /// <see cref="SerializableObject.FindProperty(string)"/>.
        /// </summary>
        /// <param name="pathElements">Path elements representing field names and keys to look for.</param>
        /// <param name="elementIdx">Index in the <paramref name="pathElements"/> array to start the search at.</param>
        /// <returns>Property representing the final path element, or null if not found (array index is out of range, or
        ///          property with that path doesn't exist).</returns>
        internal SerializableProperty FindProperty(PropertyPathElement[] pathElements, int elementIdx)
        {
            switch (type)
            {
                case FieldType.Object:
                    {
                        SerializableObject childObject = GetObject();

                        return childObject.FindProperty(pathElements, elementIdx);
                    }
                case FieldType.Array:
                    {
                        SerializableArray childArray = GetArray();

                        return childArray.FindProperty(pathElements, elementIdx);
                    }
                case FieldType.List:
                    {
                        SerializableList childList = GetList();

                        return childList.FindProperty(pathElements, elementIdx);
                    }
                case FieldType.Dictionary:
                    {
                        SerializableDictionary childDictionary = GetDictionary();

                        return childDictionary.FindProperty(pathElements, elementIdx);
                    }
            }

            return null;
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_CreateInstance(SerializableProperty instance, Type type);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern SerializableObject Internal_CreateObject(IntPtr nativeInstance, object managedInstance,
            Type actualType);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern SerializableArray Internal_CreateArray(IntPtr nativeInstance, object managedInstance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern SerializableList Internal_CreateList(IntPtr nativeInstance, object managedInstance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern SerializableDictionary Internal_CreateDictionary(IntPtr nativeInstance, object managedInstance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern object Internal_CreateManagedObjectInstance(IntPtr nativeInstance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern Array Internal_CreateManagedArrayInstance(IntPtr nativeInstance, int[] lengths);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern IList Internal_CreateManagedListInstance(IntPtr nativeInstance, int length);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern IDictionary Internal_CreateManagedDictionaryInstance(IntPtr nativeInstance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern object Internal_CloneManagedInstance(IntPtr nativeInstance, object original);

        /// <summary>
        /// Converts a C# type into internal serialization type.
        /// </summary>
        /// <param name="internalType">C# to convert.</param>
        /// <returns>Internal serialization type. Throws an exception if matching type cannot be found.</returns>
        public static FieldType DetermineFieldType(Type internalType)
        {
            if (!internalType.IsArray)
            {
                if (internalType.IsEnum)
                    return FieldType.Enum;
                else if (internalType == typeof (Byte))
                    return FieldType.Int;
                else if (internalType == typeof (SByte))
                    return FieldType.Int;
                else if (internalType == typeof (Int16))
                    return FieldType.Int;
                else if (internalType == typeof (UInt16))
                    return FieldType.Int;
                else if (internalType == typeof (Int32))
                    return FieldType.Int;
                else if (internalType == typeof (UInt32))
                    return FieldType.Int;
                else if (internalType == typeof (Int64))
                    return FieldType.Int;
                else if (internalType == typeof (UInt64))
                    return FieldType.Int;
                else if (internalType == typeof (bool))
                    return FieldType.Bool;
                else if (internalType == typeof (float))
                    return FieldType.Float;
                else if (internalType == typeof (double))
                    return FieldType.Float;
                else if (internalType == typeof (string))
                    return FieldType.String;
                else if (internalType == typeof (Vector2))
                    return FieldType.Vector2;
                else if (internalType == typeof (Vector3))
                    return FieldType.Vector3;
                else if (internalType == typeof (Vector4))
                    return FieldType.Vector4;
                else if (internalType == typeof(Quaternion))
                    return FieldType.Quaternion;
                else if (internalType == typeof (Color))
                    return FieldType.Color;
                else if (internalType == typeof(ColorGradient))
                    return FieldType.ColorGradient;
                else if (internalType == typeof(ColorGradientHDR))
                    return FieldType.ColorGradientHDR;
                else if (internalType == typeof(AnimationCurve))
                    return FieldType.Curve;
                else if (internalType == typeof(FloatDistribution))
                    return FieldType.FloatDistribution;
                else if (internalType == typeof(Vector2Distribution))
                    return FieldType.Vector2Distribution;
                else if (internalType == typeof(Vector3Distribution))
                    return FieldType.Vector3Distribution;
                else if (internalType == typeof(ColorDistribution))
                    return FieldType.ColorDistribution;
                else if (internalType.IsSubclassOf(typeof (GameObject)))
                    return FieldType.GameObjectRef;
                else if (internalType.IsSubclassOf(typeof (Resource)))
                    return FieldType.Resource;
                else if (internalType.IsSubclassOf(typeof (RRefBase)))
                    return FieldType.RRef;
                else if (internalType.IsGenericType)
                {
                    Type genericType = internalType.GetGenericTypeDefinition();

                    if (genericType == typeof (List<>))
                        return FieldType.List;
                    else if (genericType == typeof (Dictionary<,>))
                        return FieldType.Dictionary;
                    else if (genericType == typeof(RRef<>))
                        return FieldType.RRef;

                    // Shouldn't happen because native code should only supply us with supported types
                    throw new Exception("Cannot determine field type. Found an unsupported generic type.");
                }

                // Otherwise the type must be an object, unless some error occurred
                return FieldType.Object;
            }

            return FieldType.Array;
        }
    }

    /** @} */
}
