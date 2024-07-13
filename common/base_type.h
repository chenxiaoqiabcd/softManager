/************************************************************************/
/*  编写：王岁明  
	时间：2010-12-15
	功能：定义一些常用数据类型 常用常量
*/
/************************************************************************/
#pragma once

#include <string>
#include <vector>
#include <set>
#include <map>
#include <bitset>
#include <list>
#include <queue>
#include <stack>
#include <assert.h>
//
//#if defined(WIN32)
//#include <winsock2.h>//之前包含winsock2.h 避免与winsock.h的定义冲突
//#include <TCHAR.h>
//#include <Windows.h>
//#else
//#endif

typedef char            	int8;
typedef unsigned char   	uint8;
typedef int             	int32;
typedef unsigned int    	uint32;
typedef short           	int16;
typedef unsigned short		uint16;
typedef __int64  			int64;
typedef unsigned __int64	uint64;

typedef unsigned long   	u_long;
typedef unsigned long   	ulong;
typedef long            	s_long;
typedef unsigned char   	u_char;
typedef unsigned char   	uchar;
typedef unsigned char   	byte;

typedef std::string			astring_t;
typedef std::wstring		wstring_t;

typedef const char*			castr_t;
typedef const wchar_t*		cwstr_t;

#define vector_t			std::vector
#define list_t				std::list
#define map_t				std::map
#define pair_t				std::pair
#define set_t				std::set
#define bitset_t            std::bitset
#define queue_t             std::queue

#define ARRSIZE(x)	(sizeof(x)/sizeof(x[0]))
#define SAFE_DELETE(x) {if (x) delete x;  x = 0; }
#define SAFE_DELETE_ARRAY(x) {if (x) delete[] x; x = 0; }

#if defined(UNICODE) || defined(_UNICODE)
	typedef wchar_t			char_t;
	typedef cwstr_t			cstr_t;
	typedef wstring_t		string_t;

	#ifndef _T
		#define _T(x)		L ## x
	#endif
#else
	typedef char			char_t;
	typedef castr_t			cstr_t;
	typedef astring_t		string_t;

	#ifndef _T
		#define _T(x)		x
	#endif
#endif
#define BASE_SUCCESSED      (1)
#define BASE_FAILED         (0)

#define BASE_MAX_LEN       (256)