//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsSettings.h"
#include "BsSettingsRTTI.h"

namespace bs
{
	Settings::Settings()
		:mHash(0)
	{ }

	void Settings::setFloat(const String& name, float value)
	{
		mFloatProperties[name] = value;
		mIntProperties.erase(name);
		mBoolProperties.erase(name);
		mStringProperties.erase(name);

		markAsDirty();
	}

	void Settings::setInt(const String& name, INT32 value)
	{
		mFloatProperties.erase(name);
		mIntProperties[name] = value;
		mBoolProperties.erase(name);
		mStringProperties.erase(name);

		markAsDirty();
	}

	void Settings::setBool(const String& name, bool value)
	{
		mFloatProperties.erase(name);
		mIntProperties.erase(name);
		mBoolProperties[name] = value;
		mStringProperties.erase(name);

		markAsDirty();
	}

	void Settings::setString(const String& name, const WString& value)
	{
		mFloatProperties.erase(name);
		mIntProperties.erase(name);
		mBoolProperties.erase(name);
		mStringProperties[name] = value;

		markAsDirty();
	}

	float Settings::getFloat(const String& name, float defaultValue)
	{
		auto iterFind = mFloatProperties.find(name);
		if (iterFind != mFloatProperties.end())
			return iterFind->second;

		return defaultValue;
	}

	INT32 Settings::getInt(const String& name, INT32 defaultValue)
	{
		auto iterFind = mIntProperties.find(name);
		if (iterFind != mIntProperties.end())
			return iterFind->second;

		return defaultValue;
	}

	bool Settings::getBool(const String& name, bool defaultValue)
	{
		auto iterFind = mBoolProperties.find(name);
		if (iterFind != mBoolProperties.end())
			return iterFind->second;

		return defaultValue;
	}

	WString Settings::getString(const String& name, const WString& defaultValue)
	{
		auto iterFind = mStringProperties.find(name);
		if (iterFind != mStringProperties.end())
			return iterFind->second;

		return defaultValue;
	}

	bool Settings::hasKey(const String& name)
	{
		if (mFloatProperties.find(name) != mFloatProperties.end())
			return true;

		if (mIntProperties.find(name) != mIntProperties.end())
			return true;

		if (mBoolProperties.find(name) != mBoolProperties.end())
			return true;

		if (mStringProperties.find(name) != mStringProperties.end())
			return true;

		return false;
	}

	void Settings::deleteKey(const String& name)
	{
		mFloatProperties.erase(name);
		mIntProperties.erase(name);
		mBoolProperties.erase(name);
		mStringProperties.erase(name);

		markAsDirty();
	}

	void Settings::deleteAllKeys()
	{
		mFloatProperties.clear();
		mIntProperties.clear();
		mBoolProperties.clear();
		mStringProperties.clear();

		markAsDirty();
	}


	RTTITypeBase* Settings::getRTTIStatic()
	{
		return SettingsRTTI::instance();
	}

	RTTITypeBase* Settings::getRTTI() const
	{
		return Settings::getRTTIStatic();
	}
}