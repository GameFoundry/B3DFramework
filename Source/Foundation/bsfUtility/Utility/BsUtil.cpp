//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Prerequisites/BsPrerequisitesUtil.h"
#include "ThirdParty/md5.h"

namespace bs
{
	String Md5(const WString& source)
	{
		MD5 md5;
		md5.Update((UINT8*)source.data(), (UINT32)source.length() * sizeof(WString::value_type));
		md5.Finalize();

		UINT8 digest[16];
		md5.Decdigest(digest, sizeof(digest));

		String buf;
		buf.Resize(32);
		for (int i = 0; i < 16; i++)
			snprintf(&(buf[0]) + i * 2, 3, "%02x", digest[i]);

		return buf;
	}

	String Md5(const String& source)
	{
		MD5 md5;
		md5.Update((UINT8*)source.data(), (UINT32)source.length() * sizeof(String::value_type));
		md5.Finalize();

		UINT8 digest[16];
		md5.Decdigest(digest, sizeof(digest));

		String buf;
		buf.Resize(32);
		for (int i = 0; i < 16; i++)
			snprintf(&(buf[0]) + i * 2, 3, "%02x", digest[i]);

		return buf;
	}
}
