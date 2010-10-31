/*
 *  CParser.h
 *  HyperC
 *
 *  Created by Uli Kusterer on 29.07.06.
 *  Copyright 2006 Uli Kusterer. All rights reserved.
 *
 */

#pragma once

#include <deque>
#include "CToken.h"
#include <sstream>
#include <ios>
#include <map>


#include "CParseTree.h"
#include "CCodeBlockNode.h"


namespace Carlson
{
	class CFunctionDefinitionNode;
	class CCodeBlockNodeBase;
	
	// *** An entry in our operator look-up table:
	struct TOperatorEntry
	{
		TIdentifierSubtype		mType;				// The identifier for this operator.
		TIdentifierSubtype		mSecondType;		// The second identifier if this operator consists of two tokens.
		int						mPrecedence;		// Precedence, with higher number taking precedence over lower numbers (i.e. * > +).
		char					mOperationName[30];	// Name of function to call for this operator.
		TIdentifierSubtype		mTypeToReturn;		// The identifier to return for this operator.
	};
	
	// *** An entry in our unary operator look-up table:
	struct TUnaryOperatorEntry
	{
		TIdentifierSubtype		mType;						// The identifier for this operator.
		char					mOperatorCommandName[30];	// Name of function that implements this operator.
	};
	
	// *** An entry in our global property look-up table:
	struct TGlobalPropertyEntry
	{
		TIdentifierSubtype		mType;						// The identifier for this property.
		char					mGlobalPropertyVarName[30];	// Name of global variable that holds this global property.
	};
	
	// *** An entry in our chunk type look-up table:
	struct TChunkTypeEntry
	{
		TIdentifierSubtype		mType;						// The identifier for this chunk.
		TIdentifierSubtype		mPluralType;				// The identifier for this chunk when we're looking for its plural.
		TChunkType				mChunkTypeConstant;			// Constant to pass to get a range of this chunk type.
	};
	
	// *** An entry in our constant look-up table:
	struct TConstantEntry
	{
		TIdentifierSubtype		mType;	// The identifier for this constant.
		CValueNode*				mValue;	// TVariant holding this constant.
	};
	
	// *** an entry in our ObjC -> Variant or Variant -> ObjC type conversion mapping tables:
	struct TObjCTypeConversionEntry
	{
		const char*				mType;			// Type to map from or to.
		const char*				mPrefix;		// Prefix code to put before the value being converted.
		const char*				mSuffix;		// Suffix code to put after the value being converted.
		bool					mUsesObjC;		// TRUE if this code needs us to pull in ObjC.
	};
	
	// *** an entry in our table of ObjC methods we know how to call:
	class CObjCMethodEntry
	{
	public:
		CObjCMethodEntry() {};
		CObjCMethodEntry( std::string& hdr, std::string& frm, std::string& sig ) : mHeaderName(hdr), mFrameworkName(frm), mMethodSignature(sig) {};
		
	public:
		std::string				mHeaderName;		// Name of framework umbrella header that declares this method as it's specified for an include statement.
		std::string				mFrameworkName;		// Name of framework as it's passed to -framework command line option.
		std::string				mMethodSignature;	// The return and parameter types of the method.
	};
	
	// -------------------------------------------------------------------------
	//	MAIN CLASS:
	// -------------------------------------------------------------------------
	
	class CParser
	{
	protected:
		std::map<std::string,CVariableEntry>	mGlobals;	// List of globals so we can declare them.
		std::string				mFirstHandlerName;			// Name of the function implementing the first handler we parse (can be used by templates as main entry point).
		bool					mFirstHandlerIsFunction;	// TRUE if mFirstHandlerName is a function, FALSE if it's a message/command handler.
		bool					mUsesObjCCall;				// Flag that gets set if we need to include the ObjC-support library.
		const char*				mFileName;					// Name of file being parsed right now.
		const char*				mSupportFolderPath;			// Path to folder with support files.
		
	protected:
		static std::map<std::string,CObjCMethodEntry>	sObjCMethodTable;		// Populated from frameworkheaders.hhc file.
		static std::map<std::string,CObjCMethodEntry>	sCFunctionTable;		// Populated from frameworkheaders.hhc file.
		static std::map<std::string,CObjCMethodEntry>	sCFunctionPointerTable;	// Populated from frameworkheaders.hhc file.
		static std::map<std::string,std::string>		sSynonymToTypeTable;	// Populated from frameworkheaders.hhc file.
		static std::map<std::string,int>				sConstantToValueTable;	// Populated from frameworkheaders.hhc file.
		
	public:
		CParser() : mUsesObjCCall(false)	{};
		
		void	Parse( const char* fname, std::deque<CToken>& tokens, CParseTree& parseTree );
		
		void	ParseTopLevelConstruct( std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens, CParseTree& parseTree );
		void	ParseFunctionDefinition( const std::string& prefix, std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens, CParseTree& parseTree );
		void	ParseHandlerCall( CParseTree& parseTree, CCodeBlockNodeBase* currFunction,
									std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens );
		void	ParsePutStatement( CParseTree& parseTree, CCodeBlockNodeBase* currFunction,
									std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens );
		void	ParseGetStatement( CParseTree& parseTree,
									CCodeBlockNodeBase* currFunction,
									std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens );
		void	ParseSetStatement( CParseTree& parseTree,
									CCodeBlockNodeBase* currFunction,
									std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens );
		void	ParseGlobalStatement( bool isPublic, CParseTree& parseTree,
										CCodeBlockNodeBase* currFunction,
										std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens );
		void	ParseReturnStatement( CParseTree& parseTree,
										CCodeBlockNodeBase* currFunction,
										std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens );
		void	ParseRepeatForEachStatement( std::string& userHandlerName, CParseTree& parseTree,
										CCodeBlockNodeBase* currFunction,
										std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens );
		void	ParseRepeatStatement( std::string& userHandlerName, CParseTree& parseTree,
										CCodeBlockNodeBase* currFunction,
										std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens );
		void	ParseIfStatement( std::string& userHandlerName, CParseTree& parseTree,
										CCodeBlockNodeBase* currFunction,
										std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens );
		CValueNode*	ParseContainer( bool asPointer, bool initWithName, CParseTree& parseTree,
										CCodeBlockNodeBase* currFunction,
										std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens );
		CValueNode*	ParseArrayItem( CParseTree& parseTree, CCodeBlockNodeBase* currFunction,
									std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens );
		void	ParseOneLine( std::string& userHandlerName,
										CParseTree& parseTree,
										CCodeBlockNodeBase* currFunction,
										std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens,
										bool dontSwallowReturn = false );
		void	ParseFunctionBody( std::string& userHandlerName,
									CParseTree& parseTree, CCodeBlockNodeBase* currFunction,
									std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens );
		CValueNode*	ParseParamList( TIdentifierSubtype identifierToEndOn,
										CParseTree& parseTree, CCodeBlockNodeBase* currFunction,
										std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens );
		CValueNode*	ParseObjCMethodCall( CParseTree& parseTree, CCodeBlockNodeBase* currFunction,
										std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens );
		CValueNode*	ParseNativeFunctionCallStartingAtParams( std::string& methodName, CObjCMethodEntry& methodInfo,
										CParseTree& parseTree, CCodeBlockNodeBase* currFunction,
										std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens );
		CValueNode*	ParseTerm( CParseTree& parseTree, CCodeBlockNodeBase* currFunction,
										std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens );
		void	OutputExpressionStack( CParseTree& parseTree, CCodeBlockNodeBase* currFunction,
										std::deque<std::string>	&terms, std::deque<const char*>	&operators );
		void	CreateVariable( const std::string& varName, const std::string& realVarName, bool initWithName,
								CCodeBlockNodeBase* currFunction, bool isGlobal = false );
		TIdentifierSubtype	ParseOperator( std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens, int *outPrecedence, const char* *outOpName );
		CValueNode*	ParseChunkExpression( TChunkType typeConstant, CParseTree& parseTree, CCodeBlockNodeBase* currFunction,
										std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens );
		CValueNode*	ParseConstantChunkExpression( TChunkType typeConstant, CParseTree& parseTree, CCodeBlockNodeBase* currFunction,
										std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens );
		CValueNode*	ParseExpression( CParseTree& parseTree, CCodeBlockNodeBase* currFunction,
										std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens );
		void	ParseAddStatement( CParseTree& parseTree, CCodeBlockNodeBase* currFunction,
										std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens );
		void	ParseSubtractStatement( CParseTree& parseTree, CCodeBlockNodeBase* currFunction,
										std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens );
		void	ParseMultiplyStatement( CParseTree& parseTree, CCodeBlockNodeBase* currFunction,
										std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens );
		void	ParseDivideStatement( CParseTree& parseTree, CCodeBlockNodeBase* currFunction,
										std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens );
		TChunkType	GetChunkTypeNameFromIdentifierSubtype( TIdentifierSubtype identifierToCheck );
		void	FillArrayWithComponentsSeparatedBy( const char* typesStr, char delimiter, std::deque<std::string> &destTypesList );
		void	CreateHandlerTrampolineForFunction( const std::string &handlerName, const std::string& procPtrName,
														const char* typesStr,
														std::stringstream& theCode, std::string &outTrampolineName );
		
		CValueNode*	CollapseExpressionStack( std::deque<CValueNode*> &terms, std::deque<const char*> &operators );
		
		bool		GetUsesObjCCall()									{ return mUsesObjCCall; };
		std::string	GetFirstHandlerName()								{ return mFirstHandlerName; };
		const char*	GetSupportFolderPath()								{ return mSupportFolderPath; };
		void		SetSupportFolderPath( const char* spfp )			{ mSupportFolderPath = spfp; };
		
		void		GenerateObjCTypeToVariantCode( std::string type, std::string &prefix, std::string &suffix );
		void		GenerateVariantToObjCTypeCode( std::string type, std::string &prefix, std::string &suffix, std::string& ioValue );
		void		LoadNativeHeaders();
		void		LoadNativeHeadersFromFile( const char* filepath );
	};
}