//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsEditorPrerequisites.h"
#include "BsIReflectable.h"

namespace bs
{
	/** @addtogroup Settings
	 *  @{
	 */

	/**	Contains a serializable set of generic key-value pairs. */
	class BS_ED_EXPORT Settings : public IReflectable
	{
	public:
		Settings();

		/**	Adds or updates a property key/value pair with a floating point value. */
		void setFloat(const String& name, float value);

		/**	Adds or updates a property key/value pair with a signed integer value. */
		void setInt(const String& name, INT32 value);

		/**	Adds or updates a property key/value pair with a boolean value. */
		void setBool(const String& name, bool value);

		/**	Adds or updates a property key/value pair with a string value. */
		void setString(const String& name, const WString& value);

		/** Returns the floating point value of the specified key, or the default value if such key cannot be found. */
		float getFloat(const String& name, float defaultValue = 0.0f);

		/** Returns the integer point value of the specified key, or the default value if such key cannot be found. */
		INT32 getInt(const String& name, INT32 defaultValue = 0);

		/** Returns the boolean point value of the specified key, or the default value if such key cannot be found. */
		bool getBool(const String& name, bool defaultValue = false);

		/** Returns the string point value of the specified key, or the default value if such key cannot be found. */
		WString getString(const String& name, const WString& defaultValue = StringUtil::WBLANK);

		/**	Returns true if the key with the specified name exists. */
		bool hasKey(const String& name);

		/**	Deletes a key with the specified name. */
		void deleteKey(const String& name);

		/**	Deletes all key/value pairs. */
		void deleteAllKeys();

		/**	Returns a hash value that may be used for checking if any internal settings were modified. */
		UINT32 getHash() const { return mHash; }

	protected:
		/**	Marks the object as dirty so that outside objects know when to update. */
		void markAsDirty() const { mHash++; }

		UnorderedMap<String, float> mFloatProperties;
		UnorderedMap<String, INT32> mIntProperties;
		UnorderedMap<String, bool> mBoolProperties;
		UnorderedMap<String, WString> mStringProperties;

		mutable UINT32 mHash;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class SettingsRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** @} */
}