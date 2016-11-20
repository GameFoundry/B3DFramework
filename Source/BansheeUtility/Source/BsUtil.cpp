//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsPrerequisitesUtil.h"
#include "ThirdParty/md5.h"

namespace bs
{
	String md5(const WString& source)
	{
		MD5 md5;
		md5.update((UINT8*)source.c_str(), (UINT32)source.length() * sizeof(WString::value_type));
		md5.finalize();

		UINT8 digest[16];
		md5.decdigest(digest, sizeof(digest));

		char buf[33];
		for (int i = 0; i < 16; i++)
			sprintf(buf + i * 2, "%02x", digest[i]);
		buf[32] = 0;
		
		return String(buf);
	}

	String md5(const String& source)
	{
		MD5 md5;
		md5.update((UINT8*)source.c_str(), (UINT32)source.length() * sizeof(String::value_type));
		md5.finalize();

		UINT8 digest[16];
		md5.decdigest(digest, sizeof(digest));

		char buf[33];
		for (int i = 0; i < 16; i++)
			sprintf(buf + i * 2, "%02x", digest[i]);
		buf[32] = 0;

		return String(buf);
	}
}