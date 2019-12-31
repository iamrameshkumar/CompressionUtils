#include "stdafx.h"
#include "GZipUtils.h"


# define CHUNK 16384

bool GZipCompress2(TCHAR * source_file, TCHAR * dest_file)
{
	FILE * source = _tfsopen(source_file, _T("rb"), _SH_DENYNO);
	if (!source) return false;
	gzFile * dest = (gzFile *)gzopen(CStringA(dest_file).GetBuffer(), "wb");
	if (!dest)
	{
		gzclose(source);
		return false;
	}

	GZipCompress1(dest, source);

	if (fclose(source) != 0)
	{

	}

	int res = gzclose(dest);
	return true;

}

bool GZipCompress1(gzFile * dest, FILE * source)
{
	//bool ret = false ; 
	char* in;

	in = (char*)malloc(CHUNK * sizeof(char));

	int err;


	int bytes_read;
	do
	{
		if (in)
		{
			bytes_read = fread(in, 1, CHUNK, source);
			//gzerror(source, &err) ;
			if (bytes_read <= 0)
			{
				//   Some error handling needs to be done (check if full source file has been read and archived).
				//(void)deflateEnd(&strm);
				break;
				//return Z_ERRNO;
			}

			if (gzwrite(dest, in, bytes_read) != bytes_read)
			{
				//(void)deflateEnd(&strm);
				//return Z_ERRNO;
				break;
			}

			gzerror(dest, &err);

			if (err)
			{
				//(void)deflateEnd(&strm);
				//return Z_ERRNO;
				break;
			}

			memset(in, 0, CHUNK * sizeof(char));
		}

	} while (bytes_read > 0);

	if (in) free(in);

	return true;
}