// CompressionUtilDemo.cpp : This file contains the 'main' function. Program execution begins and ends there.
//Author: Ramesh Kumar K

#include "pch.h"
#include "stdafx.h"
#include <iostream>
#include "zipper.h"
#include "strsafe.h"
#include "zip.h"
#include "zlib.h"
#include "minishared.h"
#include "iowin32.h"

#define USEWIN32IOAPI

using namespace zipper;

extern "C" {

	void minizip_banner();

	void minizip_help();

	int minizip_addfile(zipFile zf, const char *path, const char *filenameinzip, int level, const char *password);
}

CString GetWorkingDirectory()
{
	TCHAR current_directory[MAX_PATH + 1];
	GetCurrentDirectory(MAX_PATH, current_directory);
	CString current_dir_str;
	current_dir_str.Format(_T("%s"), current_directory);
	return current_dir_str;
}

BOOL  DeleteFileFrom(CString Filename, DWORD &Error)
{
	BOOL  Status = DeleteFile(Filename);
	Error = GetLastError();

	return Status;
}

int main(int argc, char *argv[])
{
	zipFile zf = NULL;
#ifdef USEWIN32IOAPI
	zlib_filefunc64_def ffunc = { 0 };
#endif
	char *zipfilename = NULL;
	const char *password = NULL;
	int zipfilenamearg = 0;
	int errclose = 0;
	int err = 0;
	int i = 0;
	int opt_overwrite = APPEND_STATUS_CREATE;
	int opt_compress_level = Z_DEFAULT_COMPRESSION;
	int opt_exclude_path = 0;

	minizip_banner();
	if (argc == 1)
	{
		minizip_help();
		return 0;
	}

	/* Parse command line options */
	for (i = 1; i < argc; i++)
	{
		if ((*argv[i]) == '-')
		{
			const char *p = argv[i] + 1;

			while ((*p) != '\0')
			{
				char c = *(p++);;
				if ((c == 'o') || (c == 'O'))
					opt_overwrite = APPEND_STATUS_CREATEAFTER;
				if ((c == 'a') || (c == 'A'))
					opt_overwrite = APPEND_STATUS_ADDINZIP;
				if ((c >= '0') && (c <= '9'))
					opt_compress_level = (c - '0');
				if ((c == 'j') || (c == 'J'))
					opt_exclude_path = 1;

				if (((c == 'p') || (c == 'P')) && (i + 1 < argc))
				{
					password = argv[i + 1];
					i++;
				}
			}
		}
		else
		{
			if (zipfilenamearg == 0)
				zipfilenamearg = i;
		}
	}

	if (zipfilenamearg == 0)
	{
		minizip_help();
		return 0;
	}
	zipfilename = argv[zipfilenamearg];

	if (opt_overwrite == 2)
	{
		/* If the file don't exist, we not append file */
		if (check_file_exists(zipfilename) == 0)
			opt_overwrite = 1;
	}
	else if (opt_overwrite == 0)
	{
		/* If ask the user what to do because append and overwrite args not set */
		if (check_file_exists(zipfilename) != 0)
		{
			char rep = 0;
			do
			{
				char answer[128];
				printf("The file %s exists. Overwrite ? [y]es, [n]o, [a]ppend : ", zipfilename);
				if (scanf("%1s", answer) != 1)
					exit(EXIT_FAILURE);
				rep = answer[0];

				if ((rep >= 'a') && (rep <= 'z'))
					rep -= 0x20;
			} while ((rep != 'Y') && (rep != 'N') && (rep != 'A'));

			if (rep == 'A')
			{
				opt_overwrite = 2;
			}
			else if (rep == 'N')
			{
				minizip_help();
				return 0;
			}
		}
	}

#ifdef USEWIN32IOAPI
	fill_win32_filefunc64A(&ffunc);
	zf = zipOpen2_64(zipfilename, opt_overwrite, NULL, &ffunc);
#else
	zf = zipOpen64(zipfilename, opt_overwrite);
#endif

	if (zf == NULL)
	{
		printf("error opening %s\n", zipfilename);
		err = ZIP_ERRNO;
	}
	else
		printf("creating %s\n", zipfilename);

	/* Go through command line args looking for files to add to zip */
	for (i = zipfilenamearg + 1; (i < argc) && (err == ZIP_OK); i++)
	{
		const char *filename = argv[i];
		const char *filenameinzip;

		/* Skip command line options */
		if ((((*(argv[i])) == '-') || ((*(argv[i])) == '/')) && (strlen(argv[i]) == 2) &&
			((argv[i][1] == 'o') || (argv[i][1] == 'O') || (argv[i][1] == 'a') || (argv[i][1] == 'A') ||
			(argv[i][1] == 'p') || (argv[i][1] == 'P') || ((argv[i][1] >= '0') && (argv[i][1] <= '9'))))
			continue;

		/* Construct the filename that our file will be stored in the zip as.
		The path name saved, should not include a leading slash.
		If it did, windows/xp and dynazip couldn't read the zip file. */

		filenameinzip = filename;
		while (filenameinzip[0] == '\\' || filenameinzip[0] == '/')
			filenameinzip++;

		/* Should the file be stored with any path info at all? */
		if (opt_exclude_path)
		{
			const char *tmpptr = NULL;
			const char *lastslash = 0;

			for (tmpptr = filenameinzip; *tmpptr; tmpptr++)
			{
				if (*tmpptr == '\\' || *tmpptr == '/')
					lastslash = tmpptr;
			}

			if (lastslash != NULL)
				filenameinzip = lastslash + 1; /* base filename follows last slash. */
		}

		err = minizip_addfile(zf, filename, filenameinzip, opt_compress_level, password);
	}

	errclose = zipClose(zf, NULL);
	if (errclose != ZIP_OK)
		printf("error in closing %s (%d)\n", zipfilename, errclose);

	return err;
}

#define ZIP_FOLDER_NAME _T("Test")

bool ZipFolder(CString& folder_path, CString& zipFileName, CString& zipped_file_location)
{
	try
	{
		WIN32_FIND_DATA ffd;
		LARGE_INTEGER filesize;
		TCHAR szDir[MAX_PATH];
		HANDLE hFind = INVALID_HANDLE_VALUE;
		DWORD dwError = 0;

		CString WorkDir = GetWorkingDirectory();

		CString ZippedFileDir;
		ZippedFileDir.Format(_T("%s\\%s"), WorkDir, ZIP_FOLDER_NAME);
		CreateDirectory(ZippedFileDir, NULL);
		CString TempFileName, Temp;

		StringCchCopy(szDir, MAX_PATH, folder_path.GetBuffer());
		StringCchCat(szDir, MAX_PATH, TEXT("\\*"));

		// Find the first file in the directory.
		hFind = FindFirstFile(szDir, &ffd);

		if (INVALID_HANDLE_VALUE == hFind)
		{
			return false;
		}

		CStringA zipped_file_name = CStringA(ZippedFileDir) + "\\" + CStringA(zipFileName);

		zipped_file_location = zipped_file_name;

		Zipper zipper(zipped_file_name.GetBuffer());

		BOOL status = DeleteFileFrom(CString(zipped_file_name), dwError);

		if (TRUE != status)
		{
			// error handling
		}

		do
		{
			if (_tcscmp(ffd.cFileName, _T("")) != 0 && _tcscmp(ffd.cFileName, _T(".")) != 0 && _tcscmp(ffd.cFileName, _T("..")) != 0) {

				filesize.LowPart = ffd.nFileSizeLow;
				filesize.HighPart = ffd.nFileSizeHigh;
				Temp = ffd.cFileName;

				TempFileName.Format(_T("%s\\%s"), folder_path, ffd.cFileName);
				zipper.add(CStringA(TempFileName).GetBuffer(), Zipper::zipFlags::Store);
			}

		} while (FindNextFile(hFind, &ffd) != 0);

		zipper.close();

		return true;
	}
	catch (std::exception& e) {
	}
	catch (...)
	{
	}

	return false;
}