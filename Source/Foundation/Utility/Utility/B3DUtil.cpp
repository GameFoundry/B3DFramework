//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DUtilityPrerequisites.h"
#include "ThirdParty/md5.h"

namespace b3d
{
	String Md5(const WString& source)
	{
		MD5 md5;
		md5.update((u8*)source.data(), (u32)source.length() * sizeof(WString::value_type));
		md5.finalize();

		u8 digest[16];
		md5.decdigest(digest, sizeof(digest));

		String buf;
		buf.resize(32);
		for(int i = 0; i < 16; i++)
			snprintf(&(buf[0]) + i * 2, 3, "%02x", digest[i]);

		return buf;
	}

	String Md5(const String& source)
	{
		MD5 md5;
		md5.update((u8*)source.data(), (u32)source.length() * sizeof(String::value_type));
		md5.finalize();

		u8 digest[16];
		md5.decdigest(digest, sizeof(digest));

		String buf;
		buf.resize(32);
		for(int i = 0; i < 16; i++)
			snprintf(&(buf[0]) + i * 2, 3, "%02x", digest[i]);

		return buf;
	}
} // namespace b3d
