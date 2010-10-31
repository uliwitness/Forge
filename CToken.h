/*
 *  CToken.h
 *  Carlson
 *
 *  Created by Uli Kusterer on 15.04.06.
 *  Copyright 2006 Uli Kusterer. All rights reserved.
 *
 */

#pragma once

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
		ELastToken_Sentinel
	} TTokenType;

	typedef enum
	{
		EFunctionIdentifier = 0,
		EEndIdentifier,
		EPlusOperator,
		EMinusOperator,
		EMultiplyOperator,
		EDivideOperator,
		ENewlineOperator,
		ELessThanOperator,
		EGreaterThanOperator,
		EOnIdentifier,
		ECommaOperator,
		EOpenBracketOperator,
		ECloseBracketOperator,
		EPutIdentifier,
		EIntoIdentifier,
		EAfterIdentifier,
		EBeforeIdentifier,
		EReturnIdentifier,
		ETheIdentifier,
		EResultIdentifier,
		ERepeatIdentifier,
		EWhileIdentifier,
		EForIdentifier,
		ETimesIdentifier,
		EWithIdentifier,
		EToIdentifier,
		EEqualsOperator,
		EAmpersandOperator,
		EIfIdentifier,
		EThenIdentifier,
		EElseIdentifier,
		EFromIdentifier,
		ETrueIdentifier,
		EFalseIdentifier,
		EEmptyIdentifier,
		EOpenSquareBracketOperator,
		ECloseSquareBracketOperator,
		EColonOperator,
		EPeriodOperator,
		EQuoteIdentifier,
		EDownIdentifier,
		EUntilIdentifier,
		EItemIdentifier,
		EOfIdentifier,
		ENewlineIdentifier,
		EAddIdentifier,
		ESubtractIdentifier,
		EMultiplyIdentifier,
		EDivideIdentifier,
		EByIdentifier,
		ECharacterIdentifier,
		ECharIdentifier,
		EWordIdentifier,
		ELineIdentifier,
		EEntryIdentifier,
		EParamIdentifier,
		EParameterIdentifier,
		EParamCountIdentifier,
		EUnsetIdentifier,
		EIsIdentifier,
		ENotIdentifier,
		EExponentOperator,
		EModIdentifier,
		EModuloIdentifier,
		EDeleteIdentifier,
		EItemDelIdentifier,
		EItemDelimIdentifier,
		EItemDelimiterIdentifier,
		ENumberIdentifier,
		ECharactersIdentifier,
		ECharsIdentifier,
		EWordsIdentifier,
		ELinesIdentifier,
		EItemsIdentifier,
		EAtSignOperator,
		EAndIdentifier,
		EOrIdentifier,
		EExitIdentifier,
		ENextIdentifier,
		EEachIdentifier,
		ECommaIdentifier,
		EColonIdentifier,
		ECrIdentifier,
		ELineFeedIdentifier,
		ENullIdentifier,
		ESpaceIdentifier,
		ETabIdentifier,
		EPiIdentifier,
		EGetIdentifier,
		EShortIdentifier,
		EAbbrIdentifier,
		EAbbrevIdentifier,
		EAbbreviatedIdentifier,
		ELongIdentifier,
		ENumIdentifier,
		ESetIdentifier,
		ENullPointerIdentifier,
		EIdIdentifier,
		EHandlerIdentifier,
		EMessageIdentifier,
		EGlobalIdentifier,
		EPrivateIdentifier,
		EPublicIdentifier,
		EDoubleAmpersandPseudoOperator,
		ENotEqualPseudoOperator,
		ELessThanEqualPseudoOperator,
		EGreaterThanEqualPseudoOperator,
		ELastIdentifier_Sentinel	// Must be last. Used for array size and to mean "no system-defined identifier".
	} TIdentifierSubtype;

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
		int						mNumberValue;	// Number representation of this token.
		
	public:
		CToken( TTokenType type, TIdentifierSubtype subtype, size_t offs, size_t lineN, const std::string str, int n = 0 )
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