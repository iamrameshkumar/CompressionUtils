/* minizip.c
   Version 1.1, February 14h, 2010
   sample part of the MiniZip project

   Copyright (C) 1998-2010 Gilles Vollant
	 http://www.winimage.com/zLibDll/minizip.html
   Modifications of Unzip for Zip64
	 Copyright (C) 2007-2008 Even Rouault
   Modifications for Zip64 support
	 Copyright (C) 2009-2010 Mathias Svensson
	 http://result42.com

   This program is distributed under the terms of the same license as zlib.
   See the accompanying LICENSE file for the full text of the license.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>

#ifdef _WIN32
#  include <direct.h>
#  include <io.h>
#else
#  include <unistd.h>
#  include <utime.h>
#  include <sys/types.h>
#  include <sys/stat.h>
#endif

#include "zip.h"

#ifdef _WIN32
#  define USEWIN32IOAPI
#  include "iowin32.h"
#endif

#include "minishared.h"

void minizip_banner()
{
	printf("MiniZip 1.1, demo of zLib + MiniZip64 package, written by Gilles Vollant\n");
	printf("more info on MiniZip at http://www.winimage.com/zLibDll/minizip.html\n\n");
}

void minizip_help()
{
	printf("Usage : minizip [-o] [-a] [-0 to -9] [-p password] [-j] file.zip [files_to_add]\n\n" \
		"  -o  Overwrite existing file.zip\n" \
		"  -a  Append to existing file.zip\n" \
		"  -0  Store only\n" \
		"  -1  Compress faster\n" \
		"  -9  Compress better\n\n" \
		"  -j  exclude path. store only the file name.\n\n");
}

int minizip_addfile(zipFile zf, const char *path, const char *filenameinzip, int level, const char *password)
{
	zip_fileinfo zi = { 0 };
	FILE *fin = NULL;
	uint32_t crc_for_crypting = 0;
	int size_read = 0;
	int zip64 = 0;
	int err = ZIP_OK;
	char buf[UINT16_MAX];


	/* Get information about the file on disk so we can store it in zip */
	get_file_date(path, &zi.dos_date);

	if ((password != NULL) && (err == ZIP_OK))
		err = get_file_crc(path, buf, sizeof(buf), &crc_for_crypting);

	zip64 = is_large_file(path);

	/* Add to zip file */
	err = zipOpenNewFileInZip3_64(zf, filenameinzip, &zi,
		NULL, 0, NULL, 0, NULL /* comment*/,
		(level != 0) ? Z_DEFLATED : 0, level, 0,
		-MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
		password, crc_for_crypting, zip64);

	if (err != ZIP_OK)
	{
		printf("error in opening %s in zipfile (%d)\n", filenameinzip, err);
	}
	else
	{
		fin = fopen64(path, "rb");
		if (fin == NULL)
		{
			err = ZIP_ERRNO;
			printf("error in opening %s for reading\n", path);
		}
	}

	if (err == ZIP_OK)
	{
		/* Read contents of file and write it to zip */
		do
		{
			size_read = (int)fread(buf, 1, sizeof(buf), fin);
			if ((size_read < (int)sizeof(buf)) && (feof(fin) == 0))
			{
				printf("error in reading %s\n", filenameinzip);
				err = ZIP_ERRNO;
			}

			if (size_read > 0)
			{
				err = zipWriteInFileInZip(zf, buf, size_read);
				if (err < 0)
					printf("error in writing %s in the zipfile (%d)\n", filenameinzip, err);
			}
		} while ((err == ZIP_OK) && (size_read > 0));
	}

	if (fin)
		fclose(fin);

	if (err < 0)
	{
		err = ZIP_ERRNO;
	}
	else
	{
		err = zipCloseFileInZip(zf);
		if (err != ZIP_OK)
			printf("error in closing %s in the zipfile (%d)\n", filenameinzip, err);
	}

	return err;
}
