/*
 *  CToken.h
 *  Carlson
 *
 *  Created by Uli Kusterer on 15.04.06.
 *  Copyright 2006 Uli Kusterer. All rights reserved.
 *
 */

#pragma once

extern "C" {
#include "ForgeTypes.h"
}
#include <deque>
#include <string>


namespace Carlson
{

	typedef enum
	{
		EInvalidToken = 0,
		EStringToken,
		EIdentifierToken,
		ENumberToken,
		ECommentPseudoToken,
		EMultilineCommentPseudoToken,
		ECurlyStringPseudoToken,
		EGuillemotsStringPseudoToken,
		ELastToken_Sentinel
	} TTokenType;

	// These two need to be kept in sync with constants above:
	extern const char*		gTokenTypeStrings[ELastToken_Sentinel];
	extern const char*		gIdentifierStrings[ELastIdentifier_Sentinel];

	class CToken
	{
	// Class:
	public:
		static const CToken&		KNewlineToken;	
	
	public:
		static std::deque<CToken>	TokenListFromText( const char* str, size_t len );
		static TIdentifierSubtype	IdentifierTypeFromText( const char* str );
	
	// Instance:
	public:
		TTokenType				mType;			// Identifier, string, number?
		TIdentifierSubtype		mSubType;		// What kind of identifier is it?
		size_t					mOffset;		// Position of this token in text.
		size_t					mLineNum;		// Line where this token is in the text.
		std::string				mStringValue;	// String representation of this token.
		long					mNumberValue;	// Number representation of this token.
		
	public:
		CToken( TTokenType type, TIdentifierSubtype subtype, size_t offs, size_t lineN, const std::string str, long n = 0 )
			: mStringValue(str)
		{
			mType = type;
			mSubType = subtype;
			mOffset = offs;
			mNumberValue = n;
			mLineNum = lineN;
		}
		
		void			ExpectIdentifier( const std::string& inFileName, TIdentifierSubtype subType, TIdentifierSubtype precedingIdent = ELastIdentifier_Sentinel );
		
		std::string		GetDescription() const;			// All attributes of this token.
		std::string		GetShortDescription() const;	// The token, pretty much in the form the user would see it.

		// Operator overloads for use with std::map:
		//	ignores offset during comparisons, case-insensitively compares strings.
		bool	operator==( const CToken& other );
		bool	operator!=( const CToken& other );
		bool	operator>( const CToken& other );
		bool	operator<( const CToken& other );
		bool	operator>=( const CToken& other );
		bool	operator<=( const CToken& other );
		
		bool				IsIdentifier( TIdentifierSubtype subType ) const;
		const std::string	GetIdentifierText() const;			// Lowercased and otherwise normalised for easier compares.
		TIdentifierSubtype	GetIdentifierSubType() const;		// Like mSubType, but throws if this isn't an identifier.
		const std::string	GetOriginalIdentifierText() const;	// Original string as entered by user.
	
	public:
		static void	GoNextToken( const char* fname, std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens );
		static void	GoPrevToken( const char* fname, std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens );
	};


	std::string	ToLowerString( const std::string& str );

}	/* namespace Carlson */
