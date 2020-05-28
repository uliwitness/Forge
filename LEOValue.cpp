//
//  LEOValue.cpp
//  Stacksmith
//
//  Created by Uli Kusterer on 18/12/2016.
//  Copyright © 2016 Uli Kusterer. All rights reserved.
//

#include "LEOValue.hpp"
extern "C" {
#include "LEOInstructions.h"
#include "LEOContextGroup.h"
}


#pragma mark - CppVariantBase

void	CppVariantBase::SetAsInteger( LEOInteger n, LEOUnit inUnit, LEOContext * inContext )
{
	this->~CppVariantBase();
	new (this) CppVariantInteger(n, inUnit);
}


void	CppVariantBase::SetAsNumber( LEONumber n, LEOUnit inUnit, LEOContext * inContext )
{
	this->~CppVariantBase();
	new (this) CppVariantNumber(n,inUnit);
}


template<class T>
CppVariantBase*	CppVariantBase::FollowReferencesAndReturnValueOfType( struct LEOContext* inContext )
{
	return dynamic_cast<T>( FollowReferencesAndReturnValue(inContext) );
}


CppVariantBase*	CppVariantBase::FollowReferencesAndReturnValue( struct LEOContext* inContext )
{
	return this;
}


//void	CppVariantBase::GetAsRangeOfString( LEOChunkType inType,
//											size_t inRangeStart, size_t inRangeEnd,
//											string& outBuf, struct LEOContext* inContext )
//{
//	outBuf = GetAsString( inContext );
//	size_t	outChunkStart = 0,
//			outChunkEnd = 0,
//			outDelChunkStart = 0,
//			outDelChunkEnd = 0;
//	LEOGetChunkRanges( outBuf.c_str(), inType, inRangeStart, inRangeEnd, &outChunkStart, &outChunkEnd, &outDelChunkStart, &outDelChunkEnd, inContext->itemDelimiter );
//	outBuf = outBuf.substr( inRangeStart, inRangeEnd - inRangeStart );
//}


void	CppVariantBase::DetermineChunkRangeOfSubstring( size_t *ioBytesStart, size_t *ioBytesEnd,
														size_t *ioBytesDelStart, size_t *ioBytesDelEnd,
														LEOChunkType inType, size_t inRangeStart, size_t inRangeEnd,
														struct LEOContext* inContext )
{
	string outBuf = GetAsString( inContext );
	LEOGetChunkRanges( outBuf.c_str(), inType, inRangeStart, inRangeEnd, ioBytesStart, ioBytesEnd, ioBytesDelStart, ioBytesDelEnd, inContext->itemDelimiter );
}


#pragma mark - CppVariantInteger

bool	CppVariantInteger::GetAsBoolean( struct LEOContext* inContext )
{
	size_t lineNo = SIZE_T_MAX;
	uint16_t fileID = 0;
	LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
	LEOContextStopWithError( inContext, lineNo, SIZE_T_MAX, fileID, "Expected boolean, found %s.", GetDisplayTypeName().c_str() );
	return false;
}


//void	CppVariantInteger::SetAsString( const string inBuf, struct LEOContext* inContext )
//{
//	const char* str = inBuf.c_str();
//	const char* desiredEndPtr = str +strlen(str);
//	char* endPtr = NULL;
//	LEOInteger n = strtoll( str, &endPtr, 10 );
//
//	if( endPtr == desiredEndPtr )
//		mInteger = n;
//	else
//	{
//		size_t lineNo = SIZE_T_MAX;
//		uint16_t fileID = 0;
//		LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
//		LEOContextStopWithError( inContext, lineNo, SIZE_T_MAX, fileID, "Expected integer here." );
//	}
//}


void    CppVariantInteger::InitCopy( LEOValue& dest, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
    ((CppVariantBase*)dest.mBuf)->~CppVariantBase();
    new (dest.mBuf) CppVariantInteger( mInteger, mUnit );
    
    if( keepReferences == kLEOInvalidateReferences )
        dest.refObjectID = kLEOObjectIDINVALID;
}


void    CppVariantInteger::InitSimpleCopy( LEOValue& dest, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
	((CppVariantBase*)dest.mBuf)->~CppVariantBase(); new (dest.mBuf) CppVariantInteger( mInteger, mUnit );
    
    if( keepReferences == kLEOInvalidateReferences )
        dest.refObjectID = kLEOObjectIDINVALID;
}


void    CppVariantInteger::PutValueIntoValue( LEOValue& dest, struct LEOContext* inContext )
{
    dest->SetAsInteger( mInteger, mUnit, inContext );
}


#pragma mark - CppVariantNumber

bool	CppVariantNumber::GetAsBoolean( struct LEOContext* inContext )
{
	size_t lineNo = SIZE_T_MAX;
	uint16_t fileID = 0;
	LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
	LEOContextStopWithError( inContext, lineNo, SIZE_T_MAX, fileID, "Expected boolean, found %s.", GetDisplayTypeName().c_str() );
	return false;
}


void    CppVariantNumber::InitCopy( LEOValue& dest, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
    ((CppVariantBase*)dest.mBuf)->~CppVariantBase();
    new (dest.mBuf) CppVariantNumber( mFloat, mUnit );
    
    if( keepReferences == kLEOInvalidateReferences )
        dest.refObjectID = kLEOObjectIDINVALID;
}


void    CppVariantNumber::InitSimpleCopy( LEOValue& dest, LEOKeepReferencesFlag keepReferences, struct LEOContext* inContext )
{
	((CppVariantBase*)dest.mBuf)->~CppVariantBase(); new (dest.mBuf) CppVariantNumber( mFloat, mUnit );
    
    if( keepReferences == kLEOInvalidateReferences )
        dest.refObjectID = kLEOObjectIDINVALID;
}


void    CppVariantNumber::PutValueIntoValue( LEOValue& dest, struct LEOContext* inContext )
{
    dest->SetAsNumber( mFloat, mUnit, inContext );
}


#pragma mark - CppVariantReference


CppVariantBase* __nullable	CppVariantReference::FollowReferencesAndReturnValue( struct LEOContext* inContext )
{
	CppVariantBase*	theValue = static_cast<CppVariantBase*>( LEOContextGroupGetPointerForObjectIDAndSeed( inContext->group, mObjectID, mObjectSeed ));
	if( theValue == NULL )
	{
		size_t		lineNo = SIZE_T_MAX;
		uint16_t	fileID = 0;
		LEOInstructionsFindLineForInstruction( inContext->currentInstruction, &lineNo, &fileID );
		LEOContextStopWithError( inContext, lineNo, SIZE_T_MAX, fileID, "The referenced value doesn't exist anymore." );
		
		return NULL;
	}
	else
		return theValue->FollowReferencesAndReturnValue( inContext );
}
