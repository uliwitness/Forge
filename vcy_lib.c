/*
 *  vcy_lib.c
 *  HyperCompiler
 *
 *  Created by Uli Kusterer on 14.12.08.
 *  Copyright 2008 The Void Software. All rights reserved.
 *
 */

// -----------------------------------------------------------------------------
//	Headers:
// -----------------------------------------------------------------------------

#include "vcy_lib.h"
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <stdarg.h>



void	vcy_alloc_empty( vcy_variant_t* theVal )	// Must not alloc anything, so we can use this to clear fields or pre-init fields where we might not always alloc something.
{
	theVal->type = VCY_VARIANT_TYPE_CONST_STRING;
	theVal->count = 0;
	theVal->value.string = "";
}


void	vcy_alloc_number( vcy_variant_t* theVal, double theNum )
{
	theVal->type = VCY_VARIANT_TYPE_NUMBER;
	theVal->count = 0;
	theVal->value.number = theNum;
}


void	vcy_alloc_boolean( vcy_variant_t* theVal, bool theBool )
{
	theVal->type = VCY_VARIANT_TYPE_BOOLEAN;
	theVal->count = 0;
	theVal->value.boolean = theBool;
}


void	vcy_alloc_const_string( vcy_variant_t* theVal, const char* theStr, size_t theLen )
{
	theVal->type = VCY_VARIANT_TYPE_CONST_STRING;
	theVal->count = theLen;
	theVal->value.string = (char*) theStr;
}


void	vcy_alloc_const_cstring( vcy_variant_t* theVal, const char* theStr )
{
	vcy_alloc_const_string( theVal, theStr, strlen(theStr) );
}


void	vcy_alloc_string( vcy_variant_t* theVal, const char* theStr, size_t theLen )
{
	theVal->type = VCY_VARIANT_TYPE_STRING;
	theVal->count = 0;
	theVal->value.string = malloc( theLen +1 );
	memcpy( theVal->value.string, theStr, theLen );
	theVal->value.string[theLen] = 0;
}


void	vcy_alloc_cstring( vcy_variant_t* theVal, const char* theStr )
{
	vcy_alloc_string( theVal, theStr, strlen(theStr) );
}


void	vcy_kill( vcy_variant_t* theVal )
{
	switch( theVal->type )
	{
		case VCY_VARIANT_TYPE_STRING:
			free( theVal->value.string );
			break;
		
		case VCY_VARIANT_TYPE_LIST:
			for( size_t x = 0; x < theVal->count; x++ )
				vcy_free( &theVal->value.variant[x] );
			free( theVal->value.variant );
			break;
	}
}


void	vcy_free( vcy_variant_t* theVal )
{
	vcy_kill( theVal );
	
	theVal->type = VCY_VARIANT_TYPE_INVALID;
	theVal->count = 0;
	theVal->value.string = NULL;
}


void	vcy_alloc_list( vcy_variant_t* theVal )
{
	theVal->type = VCY_VARIANT_TYPE_LIST;
	theVal->count = 0;
	theVal->value.variant = NULL;
}


void	vcy_overwrite_list( vcy_variant_t* theVal )
{
	vcy_kill(theVal);
	
	theVal->type = VCY_VARIANT_TYPE_LIST;
	theVal->count = 0;
	theVal->value.variant = NULL;
}


void	vcy_overwrite( vcy_variant_t* srcVal, vcy_variant_t* dstVal )
{
	vcy_kill( dstVal );
	vcy_alloc_value( srcVal, dstVal );
}


vcy_variant_t*	vcy_add_item( vcy_variant_t* theVal )
{
	if( theVal->count == 0 || theVal->value.variant == NULL )
	{
		theVal->count = 1;
		theVal->value.variant = malloc( sizeof(vcy_variant_t) );
		return theVal->value.variant;
	}
	else
	{
		void*	theData = realloc( theVal->value.variant, (theVal->count +1) * sizeof(vcy_variant_t) );
		if( theData )
		{
			theVal->count++;
			theVal->value.variant = theData;
			
			vcy_variant_t*	newEntry = theVal->value.variant +(theVal->count -1);
			vcy_alloc_empty( newEntry );	// Just to be safe.
			
			return newEntry;
		}
	}
	
	return NULL;
}


void	vcy_alloc_value( vcy_variant_t* srcVal, vcy_variant_t* dstVal )
{
	switch( srcVal->type )
	{
		case VCY_VARIANT_TYPE_INVALID:
		case VCY_VARIANT_TYPE_CONST_STRING:
		case VCY_VARIANT_TYPE_NUMBER:
		case VCY_VARIANT_TYPE_BOOLEAN:
			*dstVal = *srcVal;
			break;
		
		case VCY_VARIANT_TYPE_STRING:
			dstVal->type = srcVal->type;
			dstVal->value.string = malloc( srcVal->count +1 );
			memmove( dstVal->value.string, srcVal->value.string, srcVal->count +1 );
			break;
		
		case VCY_VARIANT_TYPE_LIST:
			dstVal->type = srcVal->type;
			dstVal->value.variant = malloc( srcVal->count * sizeof(vcy_variant_t) );
			for( size_t x = 0; x < srcVal->count; x++ )
				vcy_alloc_value( &srcVal->value.variant[x], &dstVal->value.variant[x] );
			break;
		
		default:
			dstVal->type = VCY_VARIANT_TYPE_INVALID;
			dstVal->count = 0;
			dstVal->value.string = NULL;
			break;
	}
}


void	vcy_append( vcy_variant_t* theVal, vcy_variant_t* appendVal )
{
	if( theVal->type == VCY_VARIANT_TYPE_LIST )
	{
		if( !theVal->value.variant )
		{
			theVal->count = 1;
			theVal->value.variant = malloc( sizeof(vcy_variant_t) );
			vcy_alloc_value( appendVal, &theVal->value.variant[ theVal->count -1 ] );
		}
		else
		{
			++theVal->count;
			vcy_variant_t*	newPtr = realloc( theVal->value.variant, sizeof(vcy_variant_t) * theVal->count);
			if( newPtr )
			{
				theVal->value.variant = newPtr;
				vcy_alloc_value( appendVal, &theVal->value.variant[ theVal->count -1 ] );
			}
		}
	}
	else
		; // Error! Can't do this with other types!
}


void	vcy_cat( vcy_variant_t* firstVal, vcy_variant_t* lastVal, vcy_variant_t* dstVal )
{
	bool	firstIsStr = (firstVal->type == VCY_VARIANT_TYPE_STRING || firstVal->type == VCY_VARIANT_TYPE_CONST_STRING);
	char*	firstStr = firstIsStr ? firstVal->value.string : NULL;
	if( firstVal->type == VCY_VARIANT_TYPE_BOOLEAN )
	{
		firstIsStr = true;
		firstStr = firstVal->value.boolean ? "true" : "false";
	}
	bool	lastIsStr = (lastVal->type == VCY_VARIANT_TYPE_STRING || lastVal->type == VCY_VARIANT_TYPE_CONST_STRING);
	char*	lastStr = lastIsStr ? lastVal->value.string : NULL;
	if( lastVal->type == VCY_VARIANT_TYPE_BOOLEAN )
	{
		lastIsStr = true;
		lastStr = lastVal->value.boolean ? "true" : "false";
	}
	bool	firstIsNum = firstVal->type == VCY_VARIANT_TYPE_NUMBER;
	bool	lastIsNum = lastVal->type == VCY_VARIANT_TYPE_NUMBER;
	
	vcy_kill( dstVal );
	
	if( firstIsStr && lastIsStr )
	{
		dstVal->type = VCY_VARIANT_TYPE_STRING;
		dstVal->count = asprintf( &dstVal->value.string, "%s%s", firstStr, lastStr );
	}
	else if( firstIsNum && lastIsStr )
	{
		dstVal->type = VCY_VARIANT_TYPE_STRING;
		dstVal->count = asprintf( &dstVal->value.string, "%f%s", firstVal->value.number, lastStr );
	}
	else if( firstIsStr && lastIsNum )
	{
		dstVal->type = VCY_VARIANT_TYPE_STRING;
		dstVal->count = asprintf( &dstVal->value.string, "%s%f", firstStr, lastVal->value.number );
	}
	else	// Error: Can't do this with other types.
	{
		dstVal->type = VCY_VARIANT_TYPE_INVALID;
		dstVal->count = 0;
		dstVal->value.string = NULL;
	}
}

void	vcy_cat_space( vcy_variant_t* firstVal, vcy_variant_t* lastVal, char theDelim, vcy_variant_t* dstVal )
{
	bool	firstIsStr = (firstVal->type == VCY_VARIANT_TYPE_STRING || firstVal->type == VCY_VARIANT_TYPE_CONST_STRING);
	char*	firstStr = firstIsStr ? firstVal->value.string : NULL;
	if( firstVal->type == VCY_VARIANT_TYPE_BOOLEAN )
	{
		firstIsStr = true;
		firstStr = firstVal->value.boolean ? "true" : "false";
	}
	bool	lastIsStr = (lastVal->type == VCY_VARIANT_TYPE_STRING || lastVal->type == VCY_VARIANT_TYPE_CONST_STRING);
	char*	lastStr = lastIsStr ? lastVal->value.string : NULL;
	if( lastVal->type == VCY_VARIANT_TYPE_BOOLEAN )
	{
		lastIsStr = true;
		lastStr = lastVal->value.boolean ? "true" : "false";
	}
	bool	firstIsNum = firstVal->type == VCY_VARIANT_TYPE_NUMBER;
	bool	lastIsNum = lastVal->type == VCY_VARIANT_TYPE_NUMBER;
	
	vcy_kill( dstVal );
	
	if( firstIsStr && lastIsStr )
	{
		dstVal->type = VCY_VARIANT_TYPE_STRING;
		dstVal->count = asprintf( &dstVal->value.string, "%s%c%s", firstStr, theDelim, lastStr );
	}
	else if( firstIsNum && lastIsStr )
	{
		dstVal->type = VCY_VARIANT_TYPE_STRING;
		dstVal->count = asprintf( &dstVal->value.string, "%f%c%s", firstVal->value.number, theDelim, lastStr );
	}
	else if( firstIsStr && lastIsNum )
	{
		dstVal->type = VCY_VARIANT_TYPE_STRING;
		dstVal->count = asprintf( &dstVal->value.string, "%s%c%f", firstStr, theDelim, lastVal->value.number );
	}
	else	// Error: Can't do this with other types.
	{
		dstVal->type = VCY_VARIANT_TYPE_INVALID;
		dstVal->count = 0;
		dstVal->value.string = NULL;
	}
}


void	vcy_list_getref( vcy_variant_t* dstVal, vcy_variant_t* list, size_t oneBasedIndex )
{
	if( list->type == VCY_VARIANT_TYPE_LIST )
	{
		if( list->count >= oneBasedIndex || oneBasedIndex < 1 )
		{
			vcy_kill( dstVal );
			vcy_alloc_value( &list->value.variant[oneBasedIndex -1], dstVal );
		}
		else // ERROR! List out of range!
		{
			vcy_kill( dstVal );
			vcy_alloc_empty( dstVal );
		}
	}
	else if( list->type == VCY_VARIANT_TYPE_STRING || list->type == VCY_VARIANT_TYPE_CONST_STRING )
	{
		char		theStr[2] = { 0 };
		if( list->count >= oneBasedIndex || oneBasedIndex < 1 )
		{
			theStr[0] = list->value.string[oneBasedIndex -1];
			vcy_kill( dstVal );
			vcy_alloc_string( dstVal, theStr, 1 );
		}
		else
		{
			vcy_kill( dstVal );
			vcy_alloc_empty( dstVal );
		}
	}
	else	// ERROR! Can't do this with other values!
	{
		vcy_kill( dstVal );
		vcy_alloc_empty( dstVal );
	}
}


vcy_variant_t	vcy_list_get_const( vcy_variant_t* list, size_t oneBasedIndex )
{
	vcy_variant_t	outVariant;
	vcy_alloc_empty( &outVariant );
	
	if( list->type == VCY_VARIANT_TYPE_LIST )
	{
		if( list->count >= oneBasedIndex || oneBasedIndex < 1 )
		{
			vcy_alloc_value( &list->value.variant[oneBasedIndex -1], &outVariant );
		}
	}
	else if( list->type == VCY_VARIANT_TYPE_STRING || list->type == VCY_VARIANT_TYPE_CONST_STRING )
	{
		char		theStr[2] = { 0 };
		if( list->count >= oneBasedIndex || oneBasedIndex < 1 )
		{
			theStr[0] = list->value.string[oneBasedIndex -1];
			vcy_alloc_string( &outVariant, theStr, 1 );
		}
	}
	
	return outVariant;
}


void	vcy_alloc_list_chunks( vcy_variant_t* destVal, vcy_variant_t* srcVal, int chunkType )
{
	char			theDelim = ',';
	char			theDelim2 = ',';
	char			theDelim3 = ',';
	char			theDelim4 = ',';
	size_t			len = 0;
	const char*		theStr = vcy_as_string( srcVal, &len );
	
	if( chunkType == TChunkTypeCharacter )
	{
		vcy_alloc_string( destVal, theStr, len );
	}
	else
	{
		if( chunkType == TChunkTypeWord )
		{
			theDelim = ' ';
			theDelim2 = '\t';
			theDelim3 = '\n';
			theDelim4 = '\r';
		}
		else if( chunkType == TChunkTypeLine )
		{
			theDelim = '\n';
			theDelim2 = '\n';
			theDelim3 = '\r';
			theDelim4 = '\r';
		}
		
		size_t	startOffs = 0;
		size_t	endOffs = 0;
		
		vcy_alloc_list( destVal );
		for( size_t x = 0; x < len; x++ )
		{
			bool	isDelim = theStr[x] == theDelim || theStr[x] == theDelim2
							|| theStr[x] == theDelim3 || theStr[x] == theDelim4;
			if( isDelim || x == (len -1) )
			{
				endOffs = isDelim ? x : x +1;	// If this is last char, include it in last item. +++ Probably miss last item in string ending in delimiter.
				size_t		itemLen = endOffs -startOffs;
				if( itemLen > 0 || chunkType != TChunkTypeWord )
				{
					vcy_variant_t*	newItem = vcy_add_item( destVal );
					vcy_alloc_string( newItem, theStr +startOffs, itemLen );
				}
				startOffs = x;
			}
		}
	}
}


size_t	vcy_chunk_count( vcy_variant_t* srcVal, int chunkType )
{
	char			theDelim = ',';
	char			theDelim2 = ',';
	char			theDelim3 = ',';
	char			theDelim4 = ',';
	size_t			len = 0;
	const char*		theStr = vcy_as_string( srcVal, &len );
	size_t			itemCount = 0;
	
	if( chunkType == TChunkTypeCharacter )
	{
		return len;
	}
	else
	{
		if( chunkType == TChunkTypeWord )
		{
			theDelim = ' ';
			theDelim2 = '\t';
			theDelim3 = '\n';
			theDelim4 = '\r';
		}
		else if( chunkType == TChunkTypeLine )
		{
			theDelim = '\n';
			theDelim2 = '\n';
			theDelim3 = '\r';
			theDelim4 = '\r';
		}
		
		size_t	startOffs = 0;
		size_t	endOffs = 0;
		
		for( size_t x = 0; x < len; x++ )
		{
			bool	isDelim = theStr[x] == theDelim || theStr[x] == theDelim2
							|| theStr[x] == theDelim3 || theStr[x] == theDelim4;
			if( isDelim || x == (len -1) )
			{
				endOffs = isDelim ? x : x +1;	// If this is last char, include it in last item. +++ Probably miss last item in string ending in delimiter.
				size_t		itemLen = endOffs -startOffs;
				if( itemLen > 0 || chunkType != TChunkTypeWord )
					itemCount++;
				startOffs = x;
			}
		}
	}
	
	return itemCount;
}


void	vcy_clear_value( vcy_variant_t* theVal )
{
	vcy_kill( theVal );
	vcy_alloc_empty( theVal );
}


void	vcy_list_assign_items( vcy_variant_t* dstVal, size_t numItems, ... )
{
	va_list	args;
	
	va_start( args, numItems );
	for( size_t x = 0; x < numItems; x++ )
	{
		vcy_variant_t*	currArg = va_arg( args, vcy_variant_t* );
		vcy_variant_t*	currItem = vcy_add_item( dstVal );
		vcy_alloc_value( currArg, currItem );
	}
	va_end( args );
}


size_t	vcy_list_count( vcy_variant_t* theVal )
{
	if( theVal->type == VCY_VARIANT_TYPE_STRING
		|| theVal->type == VCY_VARIANT_TYPE_CONST_STRING )
		return theVal->count;
	else if( theVal->type == VCY_VARIANT_TYPE_LIST )
		return theVal->count;
	
	return 0;
}


void	vcy_print_atdepth( vcy_variant_t* theVal, size_t theDepth )
{
	switch( theVal->type )
	{
		case VCY_VARIANT_TYPE_INVALID:
			for( size_t x = 0; x < theDepth; x++ )
				printf( "\t" );
			printf( "<invalid>" );
			break;
		
		case VCY_VARIANT_TYPE_STRING:
		case VCY_VARIANT_TYPE_CONST_STRING:
			for( size_t x = 0; x < theDepth; x++ )
				printf( "\t" );
			printf( "\"%s\"", theVal->value.string );
			break;

		case VCY_VARIANT_TYPE_NUMBER:
			for( size_t x = 0; x < theDepth; x++ )
				printf( "\t" );
			printf( "%f", theVal->value.number );
			break;

		case VCY_VARIANT_TYPE_BOOLEAN:
			for( size_t x = 0; x < theDepth; x++ )
				printf( "\t" );
			printf( theVal->value.boolean ? "true" : "false" );
			break;

		case VCY_VARIANT_TYPE_LIST:
			for( size_t x = 0; x < theDepth; x++ )
				printf( "\t" );
			printf( "(\n" );
			for( size_t x = 0; x < theVal->count; x++ )
				vcy_print_atdepth( &theVal->value.variant[x], theDepth +1 );
			printf( ")" );
			break;
	}
	
	if( theDepth > 0 )
		printf( "\n" );
}


void	vcy_print( vcy_variant_t* theVal )
{
	vcy_print_atdepth( theVal, 0 );
}


double		vcy_as_number( vcy_variant_t* argA )
{
	double			numA = 0;
	
	if( argA->type == VCY_VARIANT_TYPE_NUMBER )
		numA = argA->value.number;
	else if( argA->type == VCY_VARIANT_TYPE_CONST_STRING || argA->type == VCY_VARIANT_TYPE_STRING )
		numA = strtod( argA->value.string, NULL );
	
	return numA;
}


const char*		vcy_as_string( vcy_variant_t* argA, size_t *outLen )
{
	static char		theNumStr[256] = { 0 };	// +++ Not thread-safe!
	const char*		theStr = theNumStr;
	
	if( argA->type == VCY_VARIANT_TYPE_NUMBER )
	{
		snprintf( theNumStr, 255, "%f", argA->value.number );
		*outLen = strlen( theNumStr );
	}
	else if( argA->type == VCY_VARIANT_TYPE_CONST_STRING || argA->type == VCY_VARIANT_TYPE_STRING )
	{
		theStr = argA->value.string;
		*outLen = argA->count;
	}
	else if( argA->type == VCY_VARIANT_TYPE_BOOLEAN )
	{
		theStr = argA->value.boolean ? "true" : "false";
		*outLen = strlen( theStr );
	}
	else
		*outLen = 0;
	
	return theStr;
}


bool	vcy_as_boolean( vcy_variant_t* argA )
{
	if( argA->type == VCY_VARIANT_TYPE_NUMBER )
		return false;
	else if( argA->type == VCY_VARIANT_TYPE_CONST_STRING || argA->type == VCY_VARIANT_TYPE_STRING )
		return strcmp(argA->value.string,"true") == 0;
	else if( argA->type == VCY_VARIANT_TYPE_BOOLEAN )
		return argA->value.boolean;
	
	return false;
}


vcy_variant_t	vcy_add( vcy_variant_t* argA, vcy_variant_t* argB )
{
	vcy_variant_t	theResult;

	double			numA = vcy_as_number(argA), numB = vcy_as_number(argB);
	vcy_alloc_number( &theResult, numA + numB );
	
	return theResult;
}


vcy_variant_t	vcy_fun_date( vcy_variant_t* paramList )
{
	vcy_variant_t	theResult = { VCY_VARIANT_TYPE_INVALID };
	char			theDateStr[256];
	time_t			rawtime = 0L;
	struct tm	*	timeinfo;
	const char	*	theFmt = "%d/%m/%y";	// short date, the default
	vcy_variant_t	dstVal = { VCY_VARIANT_TYPE_INVALID };
	
	vcy_list_getref( &dstVal, paramList, 1 );
	if( dstVal.type == VCY_VARIANT_TYPE_CONST_STRING
		|| dstVal.type == VCY_VARIANT_TYPE_CONST_STRING )
	{
		if( strcmp( dstVal.value.string, "abbreviated" ) == 0
			|| strcmp( dstVal.value.string, "abbrev" ) == 0
			|| strcmp( dstVal.value.string, "abbr" ) == 0 )
			theFmt = "%a, %b %d, %Y";
		else if( strcmp( dstVal.value.string, "long" ) == 0 )
			theFmt = "%A, %B %d, %Y";
	}
	time( &rawtime );
	timeinfo = localtime( &rawtime );
	strftime( theDateStr, sizeof(theDateStr), theFmt, timeinfo );
	
	vcy_alloc_cstring( &theResult, theDateStr );
	
	return theResult;
}


void	test_vcy_lib( void )
{
	vcy_variant_t		varA, varB, varC;
	
	// Test string manipulation:
	vcy_alloc_const_cstring( &varA, "My name is " );
	vcy_alloc_const_cstring( &varB, "Slim Shady" );
	vcy_alloc_empty( &varC );
	
	printf("varA = ");
	vcy_print( &varA );
	printf("\nvarB = ");
	vcy_print( &varB );
	printf("\nvarC = ");
	vcy_print( &varC );

	vcy_cat( &varA, &varB, &varC );
	
	printf("\nvarA = ");
	vcy_print( &varA );
	printf("\nvarB = ");
	vcy_print( &varB );
	printf("\nvarC = ");
	vcy_print( &varC );
	
	vcy_free( &varA );
	vcy_free( &varB );
	vcy_free( &varC );

	printf("\nvarA = ");
	vcy_print( &varA );
	printf("\nvarB = ");
	vcy_print( &varB );
	printf("\nvarC = ");
	vcy_print( &varC );
	printf("\n");

	vcy_alloc_number( &varA, 42 );
	vcy_alloc_boolean( &varB, true );
	vcy_alloc_empty( &varC );

	printf("\nvarA = ");
	vcy_print( &varA );
	printf("\nvarB = ");
	vcy_print( &varB );
	printf("\nvarC = ");
	vcy_print( &varC );
	printf("\n");
	
	vcy_cat( &varA, &varB, &varC );

	printf("\nvarA = ");
	vcy_print( &varA );
	printf("\nvarB = ");
	vcy_print( &varB );
	printf("\nvarC = ");
	vcy_print( &varC );
	printf("\n");
	
	vcy_cat( &varB, &varA, &varC );

	printf("\nvarA = ");
	vcy_print( &varA );
	printf("\nvarB = ");
	vcy_print( &varB );
	printf("\nvarC = ");
	vcy_print( &varC );
	printf("\n");

	vcy_free( &varA );
	vcy_free( &varB );
	vcy_free( &varC );

	printf("\nvarA = ");
	vcy_print( &varA );
	printf("\nvarB = ");
	vcy_print( &varB );
	printf("\nvarC = ");
	vcy_print( &varC );
	printf("\n");

	// Test list manipulation:
	vcy_alloc_const_cstring( &varA, "The Quick" );
	vcy_alloc_const_cstring( &varB, "Brown Fox" );
	vcy_alloc_list( &varC );
	
	printf("varA = ");
	vcy_print( &varA );
	printf("\nvarB = ");
	vcy_print( &varB );
	printf("\nvarC = ");
	vcy_print( &varC );

	vcy_append( &varC, &varA );
	
	printf("\nvarA = ");
	vcy_print( &varA );
	printf("\nvarB = ");
	vcy_print( &varB );
	printf("\nvarC = ");
	vcy_print( &varC );
	
	vcy_append( &varC, &varB );
	
	printf("\nvarA = ");
	vcy_print( &varA );
	printf("\nvarB = ");
	vcy_print( &varB );
	printf("\nvarC = ");
	vcy_print( &varC );
	
	vcy_free( &varA );
	vcy_free( &varB );
	vcy_free( &varC );

	printf("\nvarA = ");
	vcy_print( &varA );
	printf("\nvarB = ");
	vcy_print( &varB );
	printf("\nvarC = ");
	vcy_print( &varC );
	printf("\n");
}