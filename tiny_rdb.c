#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tiny_rdb.h"

static RDB_RESULT rdb_filedb_Create( const char* filepath, unsigned long headersize, unsigned long recsize, unsigned long reccount )
{
	FILE* file;
	int i,j;

	if ((file = fopen(filepath, "rb")) != NULL)
    {
		fclose(file);
		
		return TINY_RDBE_DB_EXIST;
    }
    else
    {
        if ((file = fopen(filepath, "wb")) != NULL)
        {
			RDB_HEADER_T header;
			char blank[1] = { '\0' };

			header.headersize = headersize;
            header.reccount = reccount;  
            header.recsize = recsize;
			fwrite( &header, sizeof(header), 1, file );  //db header

			//fill blank char into file
            for (i = 0; i < (int)headersize; i++)
			{
				fwrite( blank, 1, 1, file );  //user header
			}

			for (i = 0; i < (int)reccount; i++)   //record
			{
				for (j = 0; j < (int)recsize; j++)
				{
			        fwrite( blank, 1, 1, file );
				}
			}

            fclose(file);
        }
    }

	return TINY_RDBE_SUCCESS;
}

static RDB_RESULT rdb_ramdb_Create( const char* rampath, unsigned long headersize, unsigned long recsize, unsigned long reccount )
{

	return TINY_RDBE_SUCCESS;
}

static const char* rdb_dbpath_Resolve(const char* dbpath, char* site)
{
	const char *solepath;

	if ((solepath = strstr( dbpath, "::" )) == NULL)  //stored in file in default
	{
		strncpy(site, "FILE", 5);
		solepath = dbpath;
	}
    else
	{
		assert( solepath - dbpath <= TINY_RDB_SITENAME_MAX );

		strncpy( site, dbpath, solepath - dbpath );
		site[solepath - dbpath] = '\0';
		solepath += 2;
	}

	return solepath;
}

RDB_RESULT tiny_rdb_Create( const char* dbpath, unsigned long headersize, unsigned long recsize, unsigned long reccount )
{
	const char *solepath;
	char site[TINY_RDB_SITENAME_MAX+1];
	
    solepath = rdb_dbpath_Resolve(dbpath, site);
	if (strcmp(site, "FILE") == 0)
	{
		rdb_filedb_Create( solepath, headersize, recsize, reccount );
	}
	else if (strcmp(site, "RAM") == 0)
	{
		rdb_ramdb_Create( solepath, headersize, recsize, reccount );
	}

	return TINY_RDBE_SUCCESS;

}

static RDB_RESULT rdb_filedb_Delete( const char* filepath )
{
    if (remove(filepath) == 0)
    {
        return TINY_RDBE_SUCCESS;
    }
    else
    {
        return TINY_RDBE_FAILURE;
    }
}

static RDB_RESULT rdb_ramdb_Delete( const char* rampath )
{

	return TINY_RDBE_SUCCESS;
}

RDB_RESULT tiny_rdb_Delete( const char* dbpath )
{
	const char *solepath;
	char site[TINY_RDB_SITENAME_MAX+1];
	
    solepath = rdb_dbpath_Resolve(dbpath, site);
	if (strcmp(site, "FILE") == 0)
	{
		rdb_filedb_Delete( solepath );
	}
	else if (strcmp(site, "RAM") == 0)
	{
		rdb_ramdb_Delete( solepath );
	}

	return TINY_RDBE_SUCCESS;

}

RDB_HANDLE tiny_rdb_Open( const char* dbpath, RDB_FLAG_T flag )
{
	const char *solepath;
	char site[TINY_RDB_SITENAME_MAX+1];
	void* dbhandle = NULL;
	RDB_INFO_T* rdbinfo = NULL;
	
    solepath = rdb_dbpath_Resolve(dbpath, site);
	if (strcmp(site, "FILE") == 0)
	{
		dbhandle = fopen( solepath, "rb+" );
		if( dbhandle )
		{
			rdbinfo = (RDB_INFO_T*)malloc(sizeof(RDB_INFO_T));
			rdbinfo->dbhandle = dbhandle;
			rdbinfo->dbflag = flag;
			rdbinfo->dbflag.site = STORAGE_TYPE_FILE;
			rdbinfo->crecindex = 0;
			fread(&rdbinfo->dbheader, sizeof(RDB_HEADER_T), 1, dbhandle);

			tiny_rdb_MoveFirst( rdbinfo );
		}
	}
	else if (strcmp(site, "RAM") == 0)
	{
			rdbinfo = (RDB_INFO_T*)malloc(sizeof(RDB_INFO_T));
			rdbinfo->dbflag.site = STORAGE_TYPE_RAM;
			rdbinfo->crecindex = 0;
	}

	return rdbinfo;

}

RDB_RESULT tiny_rdb_Close( RDB_HANDLE dbhandle )
{
	RDB_INFO_T* rdbinfo ;

	rdbinfo = (RDB_INFO_T*)dbhandle;
	if( rdbinfo->dbflag.site == STORAGE_TYPE_FILE )
	{
        fclose(rdbinfo->dbhandle);
	}
	else if( rdbinfo->dbflag.site == STORAGE_TYPE_RAM )
	{
	}

	free(rdbinfo);

	return TINY_RDBE_SUCCESS;
}

RDB_BOOL tiny_rdb_IsExist( const char* dbpath )
{
	const char *solepath;
	char site[TINY_RDB_SITENAME_MAX+1];
	void* dbhandle = NULL;
	
    solepath = rdb_dbpath_Resolve(dbpath, site);
	if (strcmp(site, "FILE") == 0)
	{
		dbhandle = fopen( solepath, "rb" );
		if (dbhandle)
		{
			fclose(dbhandle);
			return TINY_RDB_TRUE;
		}
	}
	else if (strcmp(site, "RAM") == 0)
	{
	}

	return TINY_RDB_FALSE;
}


static RDB_RESULT tiny_rdb_GetDBHeader( RDB_HANDLE dbhandle, RDB_HEADER_T* dbheader )
{
	RDB_INFO_T* rdbinfo;
	long curpos;
	size_t count;

    assert( dbhandle && dbheader );

	rdbinfo = (RDB_INFO_T*)dbhandle;
	if( rdbinfo->dbflag.site == STORAGE_TYPE_FILE )
	{
		curpos = 0;
		if( fseek(rdbinfo->dbhandle, curpos, SEEK_SET) )
			return TINY_RDBE_INVALID_RECORD;

		count = fread(dbheader, sizeof(RDB_HEADER_T), 1, rdbinfo->dbhandle);
		if( count != 1 )
			return TINY_RDBE_FAILURE;
	}
	else if( rdbinfo->dbflag.site == STORAGE_TYPE_RAM )
	{

	}

	return TINY_RDBE_SUCCESS;
}

static RDB_RESULT tiny_rdb_SetDBHeader( RDB_HANDLE dbhandle, const RDB_HEADER_T *dbheader )
{
	RDB_INFO_T* rdbinfo;
	long curpos;
	size_t count;

    assert( dbhandle && dbheader );

	rdbinfo = (RDB_INFO_T*)dbhandle;
	if( rdbinfo->dbflag.site == STORAGE_TYPE_FILE )
	{
		curpos = 0;
		if (fseek(rdbinfo->dbhandle, curpos, SEEK_SET))    //locate the current position in file
		    return TINY_RDBE_INVALID_RECORD;

		count = fwrite(dbheader, sizeof(RDB_HEADER_T), 1, rdbinfo->dbhandle);
		if( count != 1 )
			return TINY_RDBE_FAILURE;
	}
	else if( rdbinfo->dbflag.site == STORAGE_TYPE_RAM )
	{
	}

	return TINY_RDBE_SUCCESS;
}

RDB_RESULT tiny_rdb_GetHeader( RDB_HANDLE dbhandle, void* header )
{
	RDB_INFO_T* rdbinfo;
	long curpos;
	size_t count;

    assert( dbhandle && header );

	rdbinfo = (RDB_INFO_T*)dbhandle;
	if( rdbinfo->dbflag.site == STORAGE_TYPE_FILE )
	{
		curpos = sizeof(RDB_HEADER_T);
		if (fseek(rdbinfo->dbhandle, curpos, SEEK_SET))    //locate the current position in file
		    return TINY_RDBE_INVALID_RECORD;

		count = fread(header, rdbinfo->dbheader.headersize, 1, rdbinfo->dbhandle);
		if( count != 1 )
			return TINY_RDBE_FAILURE;
	}
	else if( rdbinfo->dbflag.site == STORAGE_TYPE_RAM )
	{
	}


	return TINY_RDBE_SUCCESS;
}

RDB_RESULT tiny_rdb_SetHeader( RDB_HANDLE dbhandle, const void *header )
{
	RDB_INFO_T* rdbinfo;
	long curpos;
	size_t count;

    assert( dbhandle && header );

	rdbinfo = (RDB_INFO_T*)dbhandle;
	if( rdbinfo->dbflag.site == STORAGE_TYPE_FILE )
	{
		curpos = sizeof(RDB_HEADER_T);
		if (fseek(rdbinfo->dbhandle, curpos, SEEK_SET))    //locate the current position in file
		    return TINY_RDBE_INVALID_RECORD;

		count = fwrite(header, rdbinfo->dbheader.headersize, 1, rdbinfo->dbhandle);
		if( count != 1 )
			return TINY_RDBE_FAILURE;
	}
	else if( rdbinfo->dbflag.site == STORAGE_TYPE_RAM )
	{
	}

	return TINY_RDBE_SUCCESS;
}


RDB_RESULT tiny_rdb_GetRecord( RDB_HANDLE dbhandle, void* record )
{
	RDB_INFO_T* rdbinfo;
	long curpos;
	size_t count;

    assert( dbhandle && record );

	rdbinfo = (RDB_INFO_T*)dbhandle;
	if (rdbinfo->dbflag.site == STORAGE_TYPE_FILE)
	{
		curpos = sizeof(RDB_HEADER_T) + rdbinfo->dbheader.headersize + rdbinfo->dbheader.recsize*rdbinfo->crecindex;
		if (fseek(rdbinfo->dbhandle, curpos, SEEK_SET))    //locate the current position in file
		    return TINY_RDBE_INVALID_RECORD;

		count = fread(record, rdbinfo->dbheader.recsize, 1, rdbinfo->dbhandle);
		if( count != 1 )
			return TINY_RDBE_FAILURE;
	}
	else if( rdbinfo->dbflag.site == STORAGE_TYPE_RAM )
	{

	}

	return TINY_RDBE_SUCCESS;
}

RDB_RESULT tiny_rdb_SetRecord( RDB_HANDLE dbhandle, const void *record )
{
	RDB_INFO_T* rdbinfo;
	long curpos;
	size_t count;

    assert( dbhandle && record );

	rdbinfo = (RDB_INFO_T*)dbhandle;
	if( rdbinfo->dbflag.site == STORAGE_TYPE_FILE )
	{
		curpos = sizeof(RDB_HEADER_T) + rdbinfo->dbheader.headersize + rdbinfo->dbheader.recsize*rdbinfo->crecindex;
		if (fseek(rdbinfo->dbhandle, curpos, SEEK_SET))     //locate the current position in file
		    return TINY_RDBE_INVALID_RECORD;

		count = fwrite(record, rdbinfo->dbheader.recsize, 1, rdbinfo->dbhandle);
		if( count != 1 )
            return TINY_RDBE_FAILURE;
	}
	else if( rdbinfo->dbflag.site == STORAGE_TYPE_RAM )
	{

	}

	return TINY_RDBE_SUCCESS;

}

RDB_RESULT tiny_rdb_MoveFirst( RDB_HANDLE dbhandle )
{
	RDB_INFO_T* rdbinfo;

    assert( dbhandle );

	rdbinfo = (RDB_INFO_T*)dbhandle;
	if( rdbinfo->dbflag.site == STORAGE_TYPE_FILE )
	{
		if (rdbinfo->dbheader.reccount == 0)
		{
		    rdbinfo->crecindex = -1;
			return TINY_RDBE_OVER_RANGE;
		}

		rdbinfo->crecindex = 0;
	}
	else if( rdbinfo->dbflag.site == STORAGE_TYPE_RAM )
	{
		if (rdbinfo->dbheader.reccount == 0)
		{
		    rdbinfo->crecindex = -1;
			return TINY_RDBE_OVER_RANGE;
		}

		rdbinfo->crecindex = 0;
	}

	return TINY_RDBE_SUCCESS;
}

RDB_RESULT tiny_rdb_MoveLast( RDB_HANDLE dbhandle )
{
	RDB_INFO_T* rdbinfo;

    assert( dbhandle );

	rdbinfo = (RDB_INFO_T*)dbhandle;
	if( rdbinfo->dbflag.site == STORAGE_TYPE_FILE )
	{
		if (rdbinfo->dbheader.reccount == 0)
		{
		    rdbinfo->crecindex = -1;
			return TINY_RDBE_OVER_RANGE;
		}

		rdbinfo->crecindex = rdbinfo->dbheader.reccount - 1;
	}
	else if( rdbinfo->dbflag.site == STORAGE_TYPE_RAM )
	{
		if (rdbinfo->dbheader.reccount == 0)
		{
		    rdbinfo->crecindex = -1;
			return TINY_RDBE_OVER_RANGE;
		}

		rdbinfo->crecindex = rdbinfo->dbheader.reccount - 1;
	}

	return TINY_RDBE_SUCCESS;
}

RDB_RESULT tiny_rdb_MoveNext( RDB_HANDLE dbhandle )
{
	RDB_INFO_T* rdbinfo;

    assert( dbhandle );

	rdbinfo = (RDB_INFO_T*)dbhandle;
	if( rdbinfo->dbflag.site == STORAGE_TYPE_FILE )
	{
		rdbinfo->crecindex++;

		if( rdbinfo->crecindex >= (long)rdbinfo->dbheader.reccount )
			return TINY_RDBE_OVER_RANGE;

	}
	else if( rdbinfo->dbflag.site == STORAGE_TYPE_RAM )
	{
	}

	return TINY_RDBE_SUCCESS;
}

RDB_RESULT tiny_rdb_MovePrev( RDB_HANDLE dbhandle )
{
	RDB_INFO_T* rdbinfo;

    assert( dbhandle );

	rdbinfo = (RDB_INFO_T*)dbhandle;
	if( rdbinfo->dbflag.site == STORAGE_TYPE_FILE )
	{
		rdbinfo->crecindex--;

		if( rdbinfo->crecindex < 0 )
			return TINY_RDBE_OVER_RANGE;

	}
	else if( rdbinfo->dbflag.site == STORAGE_TYPE_RAM )
	{
	}

	return TINY_RDBE_SUCCESS;
}


RDB_RESULT tiny_rdb_SetAbsolutePosition( RDB_HANDLE dbhandle, unsigned long position )
{
	RDB_INFO_T* rdbinfo;

    assert( dbhandle );

	rdbinfo = (RDB_INFO_T*)dbhandle;
	if( position >= rdbinfo->dbheader.reccount )
		return TINY_RDBE_OVER_RANGE;

    rdbinfo->crecindex = position;

	return TINY_RDBE_SUCCESS;
}

long tiny_rdb_GetAbsolutePosition( RDB_HANDLE dbhandle )
{
	RDB_INFO_T* rdbinfo;

    assert( dbhandle );

	rdbinfo = (RDB_INFO_T*)dbhandle;

	return rdbinfo->crecindex;
}


RDB_RESULT tiny_rdb_AppendRecord( RDB_HANDLE dbhandle, const void *record )
{
	RDB_INFO_T* rdbinfo;
	RDB_HEADER_T dbheader;
	long lastpos;
	size_t count;

    assert( dbhandle && record );

	rdbinfo = (RDB_INFO_T*)dbhandle;
	if( rdbinfo->dbflag.site == STORAGE_TYPE_FILE )
	{
		lastpos = sizeof(RDB_HEADER_T) + rdbinfo->dbheader.headersize + rdbinfo->dbheader.recsize*rdbinfo->dbheader.reccount;
		if (fseek(rdbinfo->dbhandle, lastpos, SEEK_SET))     //locate the current position in file
		    return TINY_RDBE_INVALID_RECORD;

		count = fwrite(record, rdbinfo->dbheader.recsize, 1, rdbinfo->dbhandle);
		if( count != 1 )
			return TINY_RDBE_FAILURE;

		dbheader = rdbinfo->dbheader;
		dbheader.reccount++;
		if( tiny_rdb_SetDBHeader( dbhandle, &dbheader ) == TINY_RDBE_SUCCESS )
		{
			rdbinfo->dbheader.reccount = dbheader.reccount;
		}
	}
	else if( rdbinfo->dbflag.site == STORAGE_TYPE_RAM )
	{

	}

	return TINY_RDBE_SUCCESS;
}

RDB_RESULT tiny_rdb_RemoveRecord( RDB_HANDLE dbhandle )
{
	RDB_INFO_T* rdbinfo;
	RDB_HEADER_T dbheader;
	long curpos;
	size_t count;
	unsigned long i;

    assert( dbhandle );

	if (tiny_rdb_IsBOF(dbhandle) || tiny_rdb_IsEOF(dbhandle))
		return TINY_RDBE_INVALID_RECORD;

	rdbinfo = (RDB_INFO_T*)dbhandle;
	if( rdbinfo->dbflag.site == STORAGE_TYPE_FILE )
	{
		void* record = malloc(rdbinfo->dbheader.recsize);
		if (record==NULL)
			return TINY_RDBE_FAILURE;

		for (i = rdbinfo->crecindex+1; i < rdbinfo->dbheader.reccount; i++)
		{
			curpos = sizeof(RDB_HEADER_T) + rdbinfo->dbheader.headersize + rdbinfo->dbheader.recsize*i;
		    if (fseek(rdbinfo->dbhandle, curpos, SEEK_SET))     //locate the current position in file
			{
				if (record)
				{
					free(record);
					record = NULL;
				}
		        return TINY_RDBE_INVALID_RECORD;
			}

			count = fread(record, rdbinfo->dbheader.recsize, 1, rdbinfo->dbhandle);
			if (count != 1)
			{
				if (record)
				{
					free(record);
					record = NULL;
				}
				return TINY_RDBE_FAILURE;
			}
			
			fseek(rdbinfo->dbhandle, -2*(int)rdbinfo->dbheader.recsize, SEEK_CUR);
			count = fwrite(record, rdbinfo->dbheader.recsize, 1, rdbinfo->dbhandle);
			if( count != 1 )
			{
				if (record)
				{
					free(record);
					record = NULL;
				}
                return TINY_RDBE_FAILURE;
			}
		}
		
		if (record)
		{
			free(record);
			record = NULL;
		}

		dbheader = rdbinfo->dbheader;
		dbheader.reccount--;
		if( tiny_rdb_SetDBHeader( dbhandle, &dbheader ) == TINY_RDBE_SUCCESS )
		{
			rdbinfo->dbheader.reccount = dbheader.reccount;
		}
	}
	else if( rdbinfo->dbflag.site == STORAGE_TYPE_RAM )
	{

	}

	return TINY_RDBE_SUCCESS;
}

long tiny_rdb_GetRecordCount( RDB_HANDLE dbhandle )
{
	RDB_INFO_T* rdbinfo;

    assert( dbhandle );

	rdbinfo = (RDB_INFO_T*)dbhandle;

	return rdbinfo->dbheader.reccount;
}

long tiny_rdb_GetHeaderSize( RDB_HANDLE dbhandle )
{
	RDB_INFO_T* rdbinfo;

    assert( dbhandle );

	rdbinfo = (RDB_INFO_T*)dbhandle;

	return rdbinfo->dbheader.headersize;
}

long tiny_rdb_GetRecordSize( RDB_HANDLE dbhandle )
{

	RDB_INFO_T* rdbinfo;

    assert( dbhandle );

	rdbinfo = (RDB_INFO_T*)dbhandle;

	return rdbinfo->dbheader.recsize;
}

RDB_BOOL tiny_rdb_IsBOF( RDB_HANDLE dbhandle )
{
	RDB_INFO_T* rdbinfo;

    assert( dbhandle );

	rdbinfo = (RDB_INFO_T*)dbhandle;
	if (rdbinfo->crecindex < 0 || rdbinfo->dbheader.reccount == 0)
		return TINY_RDB_TRUE;

	return TINY_RDB_FALSE;
}

RDB_BOOL tiny_rdb_IsEOF( RDB_HANDLE dbhandle )
{
	RDB_INFO_T* rdbinfo;

    assert( dbhandle );

	rdbinfo = (RDB_INFO_T*)dbhandle;
	if (rdbinfo->crecindex >= (long)rdbinfo->dbheader.reccount || rdbinfo->dbheader.reccount == 0)
		return TINY_RDB_TRUE;

	return TINY_RDB_FALSE;
}
