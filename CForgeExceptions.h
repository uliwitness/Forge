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


namespace Carlson
{

class CForgeParseError : public std::runtime_error
{
public:
	CForgeParseError( const std::string& inWhatMsg, size_t inLineNum, size_t inOffset = SIZE_T_MAX, long inErrorCode = 0 ) throw() : std::runtime_error(inWhatMsg), mLineNum(inLineNum) {};
	~CForgeParseError() throw() {};
	
	size_t		GetLineNum()	{ return mLineNum; };
	size_t		GetOffset()		{ return mOffset; };
	size_t		GetErrorCode()	{ return mErrorCode; };
	
protected:
	size_t		mLineNum;
	size_t		mOffset;
	long		mErrorCode;
};

}


#endif /* defined(__Forge__CForgeExceptions__) */
