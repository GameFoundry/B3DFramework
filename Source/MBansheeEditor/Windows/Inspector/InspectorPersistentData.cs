//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using BansheeEngine;
using System.Collections.Generic;

namespace BansheeEditor
{
    /** @addtogroup Inspector
     *  @{
     */

    /// <summary>
    /// Contains Inspector specific data that should persist assembly refresh.
    /// </summary>
    internal class InspectorPersistentData : Component
    {
        [SerializeField]
        private Dictionary<ulong, SerializableProperties> componentProperties =
            new Dictionary<ulong, SerializableProperties>();

        [SerializeField]
        private Dictionary<string, SerializableProperties> resourceProperties =
            new Dictionary<string, SerializableProperties>();

        /// <summary>
        /// Returns existing, or creates new properties for a component with the specified id.
        /// </summary>
        /// <param name="componentId">Internal ID of the component to retrieve properties for.</param>
        /// <returns>A set of key value pairs representing persistent properties of an inspectable component.</returns>
        public SerializableProperties GetProperties(ulong componentId)
        {
            SerializableProperties output;
            if (!componentProperties.TryGetValue(componentId, out output))
            {
                output = new SerializableProperties();
                componentProperties[componentId] = output;
            }

            return output;
        }

        /// <summary>
        /// Returns existing, or creates new properties for a resource with the specified UUID.
        /// </summary>
        /// <param name="uuid">Unique identifier of the resource to retrieve properties for.</param>
        /// <returns>A set of key value pairs representing persistent properties of an inspectable resource.</returns>
        public SerializableProperties GetProperties(string uuid)
        {
            SerializableProperties output;
            if (!resourceProperties.TryGetValue(uuid, out output))
            {
                output = new SerializableProperties();
                resourceProperties[uuid] = output;
            }

            return output;
        }
    }

    /// <summary>
    /// Stores a serializable set of key-value pairs of various types.
    /// </summary>
    [SerializeObject]
    public class SerializableProperties
    {
        [SerializeField]
        private Dictionary<string, bool> booleans = new Dictionary<string, bool>();
        [SerializeField]
        private Dictionary<string, float> floats = new Dictionary<string, float>();
        [SerializeField]
        private Dictionary<string, int> ints = new Dictionary<string, int>();
        [SerializeField]
        private Dictionary<string, string> strings = new Dictionary<string, string>();

        /// <summary>
        /// Sets a floating point value to a property with the specified name.
        /// </summary>
        /// <param name="name">Name to record the property under.</param>
        /// <param name="value">Value of the property.</param>
        protected internal void SetFloat(string name, float value)
        {
            floats[name] = value;
        }

        /// <summary>
        /// Sets a integer value to a property with the specified name.
        /// </summary>
        /// <param name="name">Name to record the property under.</param>
        /// <param name="value">Value of the property.</param>
        protected internal void SetInt(string name, int value)
        {
            ints[name] = value;
        }

        /// <summary>
        /// Sets a boolean value to a property with the specified name.
        /// </summary>
        /// <param name="name">Name to record the property under.</param>
        /// <param name="value">Value of the property.</param>
        protected internal void SetBool(string name, bool value)
        {
            booleans[name] = value;
        }

        /// <summary>
        /// Sets a string value to a property with the specified name.
        /// </summary>
        /// <param name="name">Name to record the property under.</param>
        /// <param name="value">Value of the property.</param>
        protected internal void SetString(string name, string value)
        {
            strings[name] = value;
        }

        /// <summary>
        /// Retrieves a value of a floating point property.
        /// </summary>
        /// <param name="name">Name of the property to retrieve.</param>
        /// <param name="defaultValue">Default value to return if property cannot be found.</param>
        /// <returns>Value of the property if it exists, otherwise the default value.</returns>
        protected internal float GetFloat(string name, float defaultValue = 0.0f)
        {
            float value;
            if (floats.TryGetValue(name, out value))
                return value;

            return defaultValue;
        }

        /// <summary>
        /// Retrieves a value of an integer property.
        /// </summary>
        /// <param name="name">Name of the property to retrieve.</param>
        /// <param name="defaultValue">Default value to return if property cannot be found.</param>
        /// <returns>Value of the property if it exists, otherwise the default value.</returns>
        protected internal int GetInt(string name, int defaultValue = 0)
        {
            int value;
            if (ints.TryGetValue(name, out value))
                return value;

            return defaultValue;
        }

        /// <summary>
        /// Retrieves a value of a boolean property.
        /// </summary>
        /// <param name="name">Name of the property to retrieve.</param>
        /// <param name="defaultValue">Default value to return if property cannot be found.</param>
        /// <returns>Value of the property if it exists, otherwise the default value.</returns>
        protected internal bool GetBool(string name, bool defaultValue = false)
        {
            bool value;
            if (booleans.TryGetValue(name, out value))
                return value;

            return defaultValue;
        }

        /// <summary>
        /// Retrieves a value of a string property.
        /// </summary>
        /// <param name="name">Name of the property to retrieve.</param>
        /// <param name="defaultValue">Default value to return if property cannot be found.</param>
        /// <returns>Value of the property if it exists, otherwise the default value.</returns>
        protected internal string GetString(string name, string defaultValue = "")
        {
            string value;
            if (strings.TryGetValue(name, out value))
                return value;

            return defaultValue;
        }

        /// <summary>
        /// Checks does a persistent property with the specified name exists.
        /// </summary>
        /// <param name="name">Name of the property to check.</param>
        /// <returns>True if the property exists, false otherwise.</returns>
        protected internal bool HasKey(string name)
        {
            return floats.ContainsKey(name) || ints.ContainsKey(name) || booleans.ContainsKey(name) ||
                   strings.ContainsKey(name);
        }

        /// <summary>
        /// Deletes a persistent property with the specified name.
        /// </summary>
        /// <param name="name">Name of the property to delete.</param>
        protected internal void DeleteKey(string name)
        {
            floats.Remove(name);
            ints.Remove(name);
            booleans.Remove(name);
            strings.Remove(name);
        }
    }

    /** @} */
}
