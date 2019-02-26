//
//  CForgeExceptions.h
//  Forge
//
//  Created by Uli Kusterer on 2013-12-19.
//
//

#ifndef __Forge__CForgeExceptions__
#define __Forge__CForgeExceptions__

#include <stdexcept>
#include <climits>
#include <string>


namespace Carlson
{

enum
{
	EForgeErrorCode_None = 0	// Success, or no error information available.
};
typedef long TForgeErrorCode;


class CForgeParseError : public std::runtime_error
{
public:
	CForgeParseError( const std::string& inWhatMsg, size_t inLineNum, size_t inOffset = SIZE_MAX, TForgeErrorCode inErrorCode = EForgeErrorCode_None ) throw() : std::runtime_error(inWhatMsg), mLineNum(inLineNum), mOffset(inOffset), mErrorCode(inErrorCode) {};
	~CForgeParseError() throw() {};
	
	size_t		GetLineNum() const		{ return mLineNum; };
	size_t		GetOffset() const		{ return mOffset; };
	size_t		GetErrorCode() const	{ return mErrorCode; };
	
protected:
	size_t			mLineNum;
	size_t			mOffset;
	TForgeErrorCode	mErrorCode;
};


class CForgeParseErrorProcessed : public CForgeParseError
{
public:
	CForgeParseErrorProcessed( const std::string& inWhatMsg, size_t inLineNum, size_t inOffset = SIZE_MAX, TForgeErrorCode inErrorCode = EForgeErrorCode_None )
		: CForgeParseError( inWhatMsg, inLineNum, inOffset, inErrorCode )	{};
	CForgeParseErrorProcessed( const CForgeParseError& inError ) : CForgeParseError( std::string(inError.what()), inError.GetLineNum(), inError.GetOffset(), inError.GetErrorCode() ) {};
};

}


#endif /* defined(__Forge__CForgeExceptions__) */
