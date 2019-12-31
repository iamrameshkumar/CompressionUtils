#pragma once

#include "zlib.h"
#include "TCHAR.h"

bool		GZipCompress2(TCHAR * source_file, TCHAR * dest_file);

bool		GZipCompress1(gzFile * dest, FILE * source);