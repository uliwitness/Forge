/*
 *  CToken.cpp
 *  Carlson
 *
 *  Created by Uli Kusterer on 15.04.06.
 *  Copyright 2006 Uli Kusterer. All rights reserved.
 *
 */

#include "CToken.h"
#include <stdexcept>
#include <ios>
#include <iostream>
#include <sstream>
#include <string>
#include "UTF8UTF32Utilities.h"
#include "CForgeExceptions.h"


#define VERBOSE_TOKEN_PARSING		0


namespace Carlson
{
#pragma mark Constant tokens

	const CToken&	CToken::KNewlineToken = CToken( EIdentifierToken, ENewlineOperator, 0, 0, std::string("") );	

#pragma mark Token type strings

	const char*		gTokenTypeStrings[ELastToken_Sentinel] =
	{
		"EInvalidToken",
		"EStringToken",
		"EIdentifierToken",
		"ENumberToken",
		"***ECommentPseudoToken",
		"***EMultilineCommentPseudoToken",
		"***ECurlyStringPseudoToken",
		"***EGuillemotsStringPseudoToken"
	};

#pragma mark Token identifier strings

	const char*		gIdentifierStrings[ELastIdentifier_Sentinel] =	// All strings in this array must already be "ToLowerString"ed, so our fake case insensitivity works.
	{
#define X2(constName,constSynonym,constStr)	constStr,
	IDENTIFIERS
#undef X2
	};

TIdentifierSubtype	gIdentifierSynonyms[ELastIdentifier_Sentinel +1] =
{
#define X2(constName,constSynonym,constStr)	constSynonym,
	IDENTIFIERS
#undef X2
	
	ELastIdentifier_Sentinel
};

#pragma mark -

	std::string	ToLowerString( const std::string& inUTF8String )
	{
		std::string		outStr;
		size_t			x = 0,
						theLen = inUTF8String.length();
		const char*		inUTF8Bytes = inUTF8String.c_str();
		
		while( x < theLen )
		{
			uint32_t	currUTF32Char = UTF8StringParseUTF32CharacterAtOffset( inUTF8Bytes, theLen, &x );
			char		outUTF8Bytes[6];
			size_t		bytesLength = 0;
			UTF8BytesForUTF32Character( UTF32CharacterToLower( currUTF32Char ), outUTF8Bytes, &bytesLength );
			outStr.append( outUTF8Bytes, bytesLength );
		}
		
		return outStr;
	}

#pragma mark -
	
	TIdentifierSubtype	CToken::IdentifierTypeFromText( const char* inLowercasedString )
	{
		size_t		x = (size_t) ELastIdentifier_Sentinel;
		
		while( true )
		{
			--x;
			
			if( strcmp( gIdentifierStrings[x], inLowercasedString ) == 0 )
				return (TIdentifierSubtype) x;
			
			if( x == 0 )
				return ELastIdentifier_Sentinel;
		}
		
		return ELastIdentifier_Sentinel;
	}
	
	std::deque<CToken>	CTokenizer::TokenListFromText( const char* str, size_t len )
	{
		size_t				x = 0,
							currStartOffs = 0;
		TTokenType			currType = EInvalidToken;	// We're in whitespace.
		std::string			currText;
		std::deque<CToken>	tokenList;
		size_t				currLineNum = 1;
		int					currNestingDepth = 0;	// Nesting depth for nestable quotes.
		
		while( x < len )
		{
			size_t			newX = x;
			uint32_t		currCh = UTF8StringParseUTF32CharacterAtOffset( str, len, &newX );
			size_t			nextNewX = newX;
			uint32_t		nextCh = (nextNewX < len) ? UTF8StringParseUTF32CharacterAtOffset( str, len, &nextNewX ) : '\0';
			
			switch( currType )
			{
				case EInvalidToken:
					if( currCh == ' ' || currCh == '\t' )
					{
						currStartOffs = x;
						x = newX;
						continue;
					}
					if( currCh == '\"' )
					{
						currType = EStringToken;
						currStartOffs = newX;
					}
					else if( currCh == 0x201C )	// “
					{
						currType = ECurlyStringPseudoToken;
						currStartOffs = newX;
					}
					else if( currCh == 0x00AB )	// «
					{
						currType = EGuillemotsStringPseudoToken;
						currStartOffs = newX;
					}
					else if( isdigit( currCh ) )
					{
						currType = ENumberToken;
						currStartOffs = x;
						currText.append( str +x, newX -x );
					}
					else if( currCh == '-' && nextCh == '-' )
						currType = ECommentPseudoToken;
					else if( currCh == '(' && nextCh == '*' )
						currType = EMultilineCommentPseudoToken;
					else
					{
						char		opstr[2] = { 0, 0 };
						opstr[0] = currCh;
						TIdentifierSubtype subtype = (isalnum(currCh)) ? ELastIdentifier_Sentinel : CToken::IdentifierTypeFromText(opstr);	// Don't interrupt a token on a short identifier like "a".
						if( subtype != ELastIdentifier_Sentinel )
						{
							tokenList.push_back( CToken( EIdentifierToken, subtype, x, currLineNum, std::string(opstr) ) );
							currText.clear();
							currType = EInvalidToken;
							currStartOffs = x;
						}
						else
						{
							currType = EIdentifierToken;
							currStartOffs = x;
							currText.append( str +x, newX -x );
						}
					}
					break;
				
				case ECommentPseudoToken:
					if( currCh == '\n' || currCh == '\r' )
					{
						tokenList.push_back( CToken( EIdentifierToken, ENewlineOperator, x, currLineNum, std::string("\n") ) );
						currText.clear();
						currType = EInvalidToken;
						currStartOffs = newX;
					}
					break;
				
				case EMultilineCommentPseudoToken:
					if( currCh == '*' && nextCh == ')' )
					{
						currText.clear();
						currType = EInvalidToken;
						newX++;
						currStartOffs = newX;
					}
					break;
				
				case ENumberToken:
					if( isdigit(currCh) )
						currText.append( str +x, newX -x );
					else
					{
						char*		endPtr = NULL;
						long long	num = strtoll( currText.c_str(), &endPtr, 10 );
						tokenList.push_back( CToken( ENumberToken, ELastIdentifier_Sentinel, currStartOffs, currLineNum, currText, num ) );
						currText.clear();
						currType = EInvalidToken;
						
						if( currCh == '\"' )
						{
							currText.clear();
							currType = EStringToken;
							currStartOffs = newX;
						}
						else if( currCh == '-' && nextCh == '-' )
						{
							currType = ECommentPseudoToken;
						}
						else if( currCh != ' ' && currCh != '\t' )
						{
							char		opstr[2] = { 0, 0 };
							opstr[0] = currCh;
							TIdentifierSubtype subtype = (isalnum(currCh)) ? ELastIdentifier_Sentinel : CToken::IdentifierTypeFromText(opstr);	// Don't interrupt a token on a short identifier like "a".
							if( subtype != ELastIdentifier_Sentinel )
							{
								tokenList.push_back( CToken( EIdentifierToken, subtype, x, currLineNum, std::string(opstr) ) );
								currText.clear();
								currType = EInvalidToken;
								currStartOffs = x;
							}
							else
							{
								currText.append( str +x, newX -x );
								currType = EIdentifierToken;
								currStartOffs = x;
							}
						}
					}
					break;
				
				case EIdentifierToken:
				{
					bool	endThisToken = (currCh == ' ' || currCh == '\t' || currCh == '\n' || currCh == '\r');
					char	opstr[2] = { 0, 0 };
					opstr[0] = currCh;
					TIdentifierSubtype subtype = (isalnum(currCh)) ? ELastIdentifier_Sentinel : CToken::IdentifierTypeFromText(opstr);	// Don't interrupt a token on a short identifier like "a".
					endThisToken = endThisToken || (subtype != ELastIdentifier_Sentinel) || (currCh == '-' && nextCh == '-');
					if( endThisToken )
					{
						tokenList.push_back( CToken( EIdentifierToken, CToken::IdentifierTypeFromText( ToLowerString( currText ).c_str() ), currStartOffs, currLineNum, currText ) );
						currType = EInvalidToken;
						
						if( currCh == '-' && nextCh == '-' )	// Comment!
							currType = ECommentPseudoToken;
						else if( subtype != ELastIdentifier_Sentinel )
							tokenList.push_back( CToken( EIdentifierToken, subtype, x, currLineNum, std::string(opstr) ) );
						
						currText.clear();
						currStartOffs = x;
					}
					else
						currText.append( str +x, newX -x );
					break;
				}
				
				case EStringToken:
					if( currCh == '\"' )
					{
						tokenList.push_back( CToken( EStringToken, ELastIdentifier_Sentinel, currStartOffs, currLineNum, currText ) );
						currText.clear();
						currStartOffs = x;
						currType = EInvalidToken;
					}
					else
						currText.append( str +x, newX -x );
					break;
				
				case ECurlyStringPseudoToken:
					if( currCh == 0x201C )	// “ Another open quote? Nest!
					{
						currNestingDepth++;
						currText.append( str +x, newX -x );
					}
					else if( currCh == 0x201D )	// ”
					{
						if( currNestingDepth > 0 )	// Nested closing quote? Un-nest.
						{
							currNestingDepth--;
							currText.append( str +x, newX -x );
						}
						else	// Final closing quote? End string.
						{
							tokenList.push_back( CToken( EStringToken, ELastIdentifier_Sentinel, currStartOffs, currLineNum, currText ) );
							currText.clear();
							currStartOffs = x;
							currType = EInvalidToken;
						}
					}
					else
						currText.append( str +x, newX -x );
					break;
				
				case EGuillemotsStringPseudoToken:
					if( currCh == 0x00AB )	// « Another open quote? Nest!
					{
						currNestingDepth++;
						currText.append( str +x, newX -x );
					}
					else if( currCh == 0x00BB )	// » 
					{
						if( currNestingDepth > 0 )	// Nested closing quote? Un-nest.
						{
							currNestingDepth--;
							currText.append( str +x, newX -x );
						}
						else	// Final closing quote? End string.
						{
							tokenList.push_back( CToken( EStringToken, ELastIdentifier_Sentinel, currStartOffs, currLineNum, currText ) );
							currText.clear();
							currStartOffs = x;
							currType = EInvalidToken;
						}
					}
					else
						currText.append( str +x, newX -x );
					break;
				
				case ELastToken_Sentinel:
					throw std::logic_error( "ELastToken_Sentinel token encountered. Should never happen." );
					break;
			}
			
			if( currCh == '\n' || currCh == '\r' )
				currLineNum++;
			
			x = newX;
		}
		
		if( currType != EInvalidToken )	// We have an unfinished token waiting to be ended!
		{
			long	num = 0;
			char*	endPtr = NULL;
			if( currType == ENumberToken )
				num = strtol( currText.c_str(), &endPtr, 10 );
			tokenList.push_back( CToken( currType, CToken::IdentifierTypeFromText( ToLowerString( currText ).c_str() ), currStartOffs, currLineNum, currText, num ) );
		}
		
		return tokenList;
	}

	/*static*/ bool		CTokenizer::NextTokensAreIdentifiers( const char* fname, std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens, TIdentifierSubtype inFirstType, ... )
	{
		if( tokenItty == tokens.end() )
		{
			return false;
		}

		std::deque<CToken>::iterator	originalTokenItty = tokenItty;
		bool							fullMatch = true;
		va_list							ap;
		
		va_start( ap, inFirstType );
			TIdentifierSubtype currType = inFirstType;
			while( currType != ELastIdentifier_Sentinel )
			{
				if( !tokenItty->IsIdentifier(currType) )
				{
					fullMatch = false;
					break;
				}
				
				++tokenItty;
				
				currType = (TIdentifierSubtype) va_arg( ap, std::underlying_type<TIdentifierSubtype>::type );
				if( currType == ELastIdentifier_Sentinel )
					break;
				
				if( tokenItty == tokens.end() )
				{
					fullMatch = false;
					break;
				}
			}
		va_end(ap);
		
		if( !fullMatch )
		{
			tokenItty = originalTokenItty;
		}
		
		return fullMatch;
	}
	
	std::string	CToken::GetDescription() const
	{
		char		numstr[256];
		std::string	str( gTokenTypeStrings[mType] );
		if( mSubType == ELastIdentifier_Sentinel )
		{
			str.append( ", \"" );
			str.append( mStringValue );
			str.append( "\"" );
		}
		else
		{
			str.append( ", '" );
			str.append( gIdentifierStrings[mSubType] );
			str.append( "'" );
		}
		sprintf( numstr,", %lu", (unsigned long) mLineNum );
		str.append( numstr );
		sprintf( numstr,", %lu", (unsigned long) mOffset );
		str.append( numstr );
		sprintf( numstr,", %lld", mNumberValue );
		str.append( numstr );
		
		return str;
	}
	
	std::string	CToken::GetShortDescription() const
	{
		if( mType == ENumberToken )
		{
			char		numstr[256];
			sprintf( numstr,"%lld", mNumberValue );
			return std::string(numstr);
		}
		else if( mType == EStringToken )
		{
			std::string		str("\"");
			str.append( mStringValue );
			str.append( "\"" );
			
			return str;
		}
		else if( mSubType == ELastIdentifier_Sentinel )
			return mStringValue;
		else
			return std::string(gIdentifierStrings[mSubType]);
	}
	
	
	bool	CToken::IsIdentifier( TIdentifierSubtype subType ) const
	{
		return( mType == EIdentifierToken && gIdentifierSynonyms[mSubType] == subType );
	}
	
	TIdentifierSubtype	CToken::GetIdentifierSubType() const
	{
		if( mType != EIdentifierToken )
			throw CForgeParseError( "Expected identifier here.", mLineNum, mOffset );
		
		return gIdentifierSynonyms[mSubType];
	}
	
	const std::string	CToken::GetIdentifierText() const
	{
		if( mType != EIdentifierToken )
			throw CForgeParseError( "Expected identifier here.", mLineNum, mOffset );
		
		if( mSubType == ELastIdentifier_Sentinel )
			return ToLowerString(mStringValue);
		else
			return std::string(gIdentifierStrings[mSubType]);
	}
	
	const std::string	CToken::GetOriginalIdentifierText() const
	{
		if( mType != EIdentifierToken )
			throw CForgeParseError( "Expected identifier here.", mLineNum, mOffset );
		
		return mStringValue;
	}
	
	// Operator overloads so we can use this in std::map & co.:
	bool	CToken::operator==( const CToken& other )
	{
		bool		same = (mType == other.mType && mSubType == other.mSubType);
		
		if( !same )
			return false;
		
		if( mType == EStringToken )
		{
			std::string	myStr( ToLowerString(mStringValue) );
			std::string	otherStr( ToLowerString(other.mStringValue) );
			same = same && ( otherStr.compare(myStr) == 0 );
		}
		else if( mType == EIdentifierToken && mSubType == ELastIdentifier_Sentinel )
			same = same && ( other.mStringValue.compare(mStringValue) == 0 );
		else if( mType == ENumberToken )
			same = same && ( mNumberValue == other.mNumberValue );
		
		return( same );
	}
	
	bool	CToken::operator!=( const CToken& other )
	{
		return !(*this == other);
	}

	bool	CToken::operator>( const CToken& other )
	{
		bool		same = (mType == other.mType && mSubType == other.mSubType);
		
		if( !same )
			return (mType == other.mType && mSubType > other.mSubType) || (mType > other.mType);
		
		if( mType == EStringToken )
		{
			std::string	myStr( ToLowerString(mStringValue) );
			std::string	otherStr( ToLowerString(other.mStringValue) );
			same = same && ( otherStr.compare(myStr) == 1 );
		}
		else if( mType == EIdentifierToken && mSubType == ELastIdentifier_Sentinel )
			same = same && ( other.mStringValue.compare(mStringValue) == 1 );
		else if( mType == ENumberToken )
			same = same && ( mNumberValue > other.mNumberValue );
		
		return( same );
	}
	
	bool	CToken::operator<( const CToken& other )
	{
		bool		same = (mType == other.mType && mSubType == other.mSubType);
		
		if( !same )
			return (mType == other.mType && mSubType < other.mSubType) || (mType < other.mType);
		
		if( mType == EStringToken )
		{
			std::string	myStr( ToLowerString(mStringValue) );
			std::string	otherStr( ToLowerString(other.mStringValue) );
			same = same && ( otherStr.compare(myStr) == -1 );
		}
		else if( mType == EIdentifierToken && mSubType == ELastIdentifier_Sentinel )
			same = same && ( other.mStringValue.compare(mStringValue) == -1 );
		else if( mType == ENumberToken )
			same = same && ( mNumberValue < other.mNumberValue );
		
		return( same );
	}

	bool	CToken::operator>=( const CToken& other )
	{
		bool		same = (mType == other.mType && mSubType == other.mSubType);
		
		if( !same )
			return (mType == other.mType && mSubType >= other.mSubType) || (mType >= other.mType);
		
		if( mType == EStringToken )
		{
			std::string	myStr( ToLowerString(mStringValue) );
			std::string	otherStr( ToLowerString(other.mStringValue) );
			same = same && ( otherStr.compare(myStr) >= 0 );
		}
		else if( mType == EIdentifierToken && mSubType == ELastIdentifier_Sentinel )
			same = same && ( other.mStringValue.compare(mStringValue) >= 0 );
		else if( mType == ENumberToken )
			same = same && ( mNumberValue >= other.mNumberValue );
		
		return( same );
	}
	
	bool	CToken::operator<=( const CToken& other )
	{
		bool		same = (mType == other.mType && mSubType == other.mSubType);
		
		if( !same )
			return (mType == other.mType && mSubType <= other.mSubType) || (mType <= other.mType);
		
		if( mType == EStringToken )
		{
			std::string	myStr( ToLowerString(mStringValue) );
			std::string	otherStr( ToLowerString(other.mStringValue) );
			same = same && ( otherStr.compare(myStr) <= 0 );
		}
		else if( mType == EIdentifierToken && mSubType == ELastIdentifier_Sentinel )
			same = same && ( other.mStringValue.compare(mStringValue) <= 0 );
		else if( mType == ENumberToken )
			same = same && ( mNumberValue <= other.mNumberValue );
		
		return( same );
	}


	void	CTokenizer::GoNextToken( const char* fname, std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens )
	{
		if( tokenItty == tokens.end() )
		{
			std::stringstream	errMsg;
			errMsg << fname << ":" << tokenItty->mLineNum << ": error: Premature end of file.";
			throw CForgeParseError( errMsg.str(), tokenItty->mLineNum, tokenItty->mOffset );
		}
		else
			tokenItty++;
		
		#if VERBOSE_TOKEN_PARSING
		if( tokenItty == tokens.end() )
			std::cout << "Advanced to: <no more tokens>" << std::endl;
		else
			std::cout << "Advanced to: " << tokenItty->GetShortDescription() << std::endl;
		#endif
	}
	
	
	void	CTokenizer::GoPreviousToken( const char* fname, std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens )
	{
		if( tokenItty == tokens.begin() )
		{
			std::stringstream	errMsg;
			errMsg << fname << ":" << tokenItty->mLineNum << ": error: Parser backtracking beyond start of file.";
			throw CForgeParseError( errMsg.str(), tokenItty->mLineNum, tokenItty->mOffset );
		}
		else
			tokenItty--;
		
		#if VERBOSE_TOKEN_PARSING
		std::cout << "Reversed to: " << tokenItty->GetShortDescription() << std::endl;
		#endif
	}

	void	CToken::ExpectIdentifier( const std::string& inFileName, TIdentifierSubtype subType, TIdentifierSubtype precedingIdent )
	{
		if( !IsIdentifier( subType ) )
		{
			std::stringstream		errMsg;
			errMsg << inFileName << ":" << mLineNum << ": error: Expected \"";
			if( precedingIdent != ELastIdentifier_Sentinel )
				errMsg << gIdentifierStrings[precedingIdent] << " ";
			errMsg << gIdentifierStrings[subType];
			errMsg << "\" here, found \"";
			if( precedingIdent != ELastIdentifier_Sentinel )
				errMsg << gIdentifierStrings[precedingIdent] << " ";
			errMsg << GetShortDescription() << "\".";
			
			throw CForgeParseError( errMsg.str(), mLineNum, mOffset );
		}
	}


} /* namespace Carlson */
