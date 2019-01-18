#ifndef __TINY_RDB_H__
#define __TINY_RDB_H__

#ifdef __cplusplus
extern "C" {
#endif

/*
|-------------DB Header(RDB_HEADER_T)----|
|----------------------------------------|
|----------------User Header-------------|
|----------------------------------------|
|----------------Record1-----------------|
|----------------------------------------|
|----------------Record2-----------------|
|----------------------------------------|
|----------------Record3-----------------|
|----------------------------------------|
|----------------Record4-----------------|
|----------------------------------------|
|----------------Record5-----------------|
|----------------------------------------|
|----------------Record6-----------------|
|----------------------------------------|
*/

//constant and macro definitions
#define TINY_RDB_FILENAME_MAX	          (256)
#define TINY_RDB_RAMPATH_MAX	          (32)
#define TINY_RDB_SITENAME_MAX	          (16)

//error code definitions
#define TINY_RDBE_SUCCESS                 ( 0 )
#define TINY_RDBE_FAILURE                 (-1 )
#define TINY_RDBE_OVERFLOW                (-2 )
#define TINY_RDBE_NOT_INIT                (-3 )
#define TINY_RDBE_DB_EXIST                (-4 )
#define TINY_RDBE_INVALID_HANDLE          (-5 )
#define TINY_RDBE_OVER_RANGE              (-6 )
#define TINY_RDBE_INVALID_RECORD          (-7 )


//data type definitions
typedef int RDB_RESULT;
typedef void* RDB_HANDLE; 

typedef enum
{
	TINY_RDB_FALSE = 0,
    TINY_RDB_TRUE = 1

}RDB_BOOL;


/***********RAM PATH map talble**********/
/*
( "Name1", addr1 )
( "Name2", addr2 )
( "Name3", addr3 )
( "Name4", addr4 )
*/


typedef enum
{
    STORAGE_TYPE_FILE = 0,
	STORAGE_TYPE_RAM
   
}RDB_STORAGE_TYPE_T;

typedef struct
{
    RDB_STORAGE_TYPE_T site:1;
	unsigned long reserve:31;
	
}RDB_FLAG_T;

typedef struct
{
    char   path[TINY_RDB_RAMPATH_MAX+1];
	void*  block;

}RDB_RAMPATH_MAP_T;

typedef  struct __NODE
{
    RDB_RAMPATH_MAP_T data;
    struct __NODE  *next;

}RDB_RAMPATH_LISTNODE_T;


typedef struct
{
    unsigned long headersize;  
    unsigned long reccount;
    unsigned long recsize;     

}RDB_HEADER_T;

typedef struct
{
   	RDB_FLAG_T    dbflag;
	RDB_HEADER_T  dbheader;
	void*         dbhandle;
    long          crecindex;  //current record index

}RDB_INFO_T;


RDB_RESULT tiny_rdb_Create( const char* dbpath, unsigned long headersize, unsigned long recsize, unsigned long reccount );
RDB_RESULT tiny_rdb_Delete( const char* dbpath );
RDB_BOOL   tiny_rdb_IsExist( const char* dbpath );


RDB_HANDLE tiny_rdb_Open( const char* dbpath, RDB_FLAG_T flag );
RDB_RESULT tiny_rdb_Close( RDB_HANDLE dbhandle );

RDB_RESULT tiny_rdb_GetHeader( RDB_HANDLE dbhandle, void* header );
RDB_RESULT tiny_rdb_SetHeader( RDB_HANDLE dbhandle, const void *header );

RDB_RESULT tiny_rdb_MoveFirst( RDB_HANDLE dbhandle );
RDB_RESULT tiny_rdb_MoveLast( RDB_HANDLE dbhandle );
RDB_RESULT tiny_rdb_MoveNext( RDB_HANDLE dbhandle );
RDB_RESULT tiny_rdb_MovePrev( RDB_HANDLE dbhandle );

RDB_RESULT tiny_rdb_SetAbsolutePosition( RDB_HANDLE dbhandle, unsigned long position );
long tiny_rdb_GetAbsolutePosition( RDB_HANDLE dbhandle );


RDB_BOOL tiny_rdb_IsBOF( RDB_HANDLE dbhandle );
RDB_BOOL tiny_rdb_IsEOF( RDB_HANDLE dbhandle );

RDB_RESULT tiny_rdb_GetRecord( RDB_HANDLE dbhandle, void* record );
RDB_RESULT tiny_rdb_SetRecord( RDB_HANDLE dbhandle, const void *record );
RDB_RESULT tiny_rdb_AppendRecord( RDB_HANDLE dbhandle, const void *record );
RDB_RESULT tiny_rdb_RemoveRecord( RDB_HANDLE dbhandle );

long tiny_rdb_GetRecordCount( RDB_HANDLE dbhandle );
long tiny_rdb_GetHeaderSize( RDB_HANDLE dbhandle );
long tiny_rdb_GetRecordSize( RDB_HANDLE dbhandle );


#ifdef __cplusplus
}
#endif

#endif
