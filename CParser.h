/*
 *  CParser.h
 *  HyperC
 *
 *  Created by Uli Kusterer on 29.07.06.
 *  Copyright 2006 Uli Kusterer. All rights reserved.
 *
 */

/*!
	@header CParser (Private)
	
	The actual parser for the Hammer programming language is called Forge, and
	is implemented mainly by the CParser C++ class. See the Forge.h header for
	the minimalist public C API people who mainly want to use Hammer in a host
	application can use to parse and compile scripts, and register host-specific
	commands, functions and properties.
*/

#pragma once

#include <deque>
#include "CToken.h"
#include <sstream>
#include <ios>
#include <map>
#include <vector>


#include "CParseTree.h"
#include "CCodeBlockNode.h"
extern "C" {
#include "LEOInterpreter.h"
#include "ForgeTypes.h"
}


namespace Carlson
{
	class CFunctionDefinitionNode;
	class CCodeBlockNodeBase;
	class CFunctionCallNode;
	
	//! An entry in our chunk type look-up table.
	struct TChunkTypeEntry
	{
		TIdentifierSubtype		mType;						//!< The identifier for this chunk.
		TIdentifierSubtype		mPluralType;				//!< The identifier for this chunk when we're looking for its plural.
		TChunkType				mChunkTypeConstant;			//!< Constant to pass to get a range of this chunk type.
	};
	
	//! An entry in our constant look-up table.
	#define MAX_CONSTANT_IDENTS		3
	struct TConstantEntry
	{
		TIdentifierSubtype		mType[MAX_CONSTANT_IDENTS];	//!< The identifier for this constant.
		CValueNode*				mValue;						//!< Actual value this constant evaluates to.
		TIdentifierSubtype		mSetName;					//!< A one-word name for the group of identifiers this belongs to, used to reference it from host entries.
	};
	
	//! An entry in our ObjC -> Variant or Variant -> ObjC type conversion mapping tables. (no longer used, needs to be ported from old source code generator)
	struct TObjCTypeConversionEntry
	{
		const char*				mType;			// Type to map from or to.
		const char*				mPrefix;		// Prefix code to put before the value being converted.
		const char*				mSuffix;		// Suffix code to put after the value being converted.
		bool					mUsesObjC;		// TRUE if this code needs us to pull in ObjC.
	};
	
	//! An entry in our table of ObjC methods we know how to call.
	class CObjCMethodEntry
	{
	public:
		CObjCMethodEntry() {};
		CObjCMethodEntry( std::string& hdr, std::string& frm, std::string& sig ) : mHeaderName(hdr), mFrameworkName(frm), mMethodSignature(sig) {};
		
	public:
		std::string				mHeaderName;		//!< Name of framework umbrella header that declares this method as it's specified for an include statement.
		std::string				mFrameworkName;		//!< Name of framework as it's passed to -framework command line option.
		std::string				mMethodSignature;	//!< The return and parameter types of the method.
	};
	
	//! Warning/error that the script editor can show in-line or in some asynchronous place.
	class CMessageEntry
	{
	public:
		std::string		mMessage;
		std::string		mFileName;
		size_t			mLineNum;
		size_t			mOffset;
		long			mErrorCode;
		
		CMessageEntry( std::string inMessage, std::string inFileName, size_t inLineNum, size_t inOffset = SIZE_T_MAX, long inErrorCode = 0 )
			: mMessage(inMessage), mFileName(inFileName), mLineNum(inLineNum), mOffset(inOffset), mErrorCode(inErrorCode) {};
	};
	
	
	//! Documentation comments found in the script so you can show them in script editor or a help viewer:
	class CHandlerNotesEntry
	{
	public:
		std::string		mHandlerName;	//!< Name of the handler to which these notes apply.
		std::string		mNotes;			//!< Actual help text the author of the handler provided.
		
		CHandlerNotesEntry( std::string inHandlerName, std::string inNotes ) : mHandlerName(inHandlerName), mNotes(inNotes) {}
	};
	
	
	typedef enum
	{
		EAllVarsAreGlobals,
		EVarsAreLocals
	} TAllVarsAreGlobals;
	
	// -------------------------------------------------------------------------
	
	typedef std::function<bool(const std::string& inFileName,const std::string& inRelativeToFileName,std::vector<char>&outContents)> CParserIncludeHandler;	//!< Fills outContents with the contents of the requested include file. Returns TRUE on success, FALSE otherwise.
	
	/*!
		@class CParser
		The object that keeps all state while parsing a token stream and
		generates a CParseTree from it.
	*/
	
	class CParser
	{
	protected:
		std::map<std::string,CVariableEntry>	mGlobals;	//!< List of globals so we can declare them.
		std::string					mFirstHandlerName;			//!< Name of the function implementing the first handler we parse (can be used by templates as main entry point).
		bool						mFirstHandlerIsFunction;	//!< TRUE if mFirstHandlerName is a function, FALSE if it's a message/command handler.
		bool						mWebPageEmbedMode;			//!< TRUE if we allow in-line content and top-level commands.
		const char*					mFileName;					//!< Name of file being parsed right now.
		const char*					mSupportFolderPath;			//!< Path to folder with support files.
		std::vector<CMessageEntry>	mMessages;					//!< Errors and warnings.
		std::vector<CHandlerNotesEntry>	mHandlerNotes;			//!< List of documentation comments found in the script.
		std::string					mCurrentNotes;				//!< The most recent "notes" text (documentation "comment") we encountered, so a handler definition can snarf it up and associate it with its handler name. Only valid while parsing.
		CParserIncludeHandler		mIncludeHandler;			//!< Lambda that is called (if present) to retrieve a file included using the "use" statement in web page mode.
		
	protected:
		static std::map<std::string,CObjCMethodEntry>	sObjCMethodTable;		//!< Populated from frameworkheaders.hhc file.
		static std::map<std::string,CObjCMethodEntry>	sCFunctionTable;		//!< Populated from frameworkheaders.hhc file.
		static std::map<std::string,CObjCMethodEntry>	sCFunctionPointerTable;	//!< Populated from frameworkheaders.hhc file.
		static std::map<std::string,std::string>		sSynonymToTypeTable;	//!< Populated from frameworkheaders.hhc file.
		static std::map<std::string,std::string>		sConstantToValueTable;	//!< Populated from frameworkheaders.hhc file.
		static LEOFirstNativeCallCallbackPtr			sFirstNativeCallCallback;
		
	public:
		CParser();
		
		void	Parse( const char* fname, std::deque<CToken>& tokens, CParseTree& parseTree, const char* scriptText );	//!< Parse a complete script consisting of handlers etc.
		void	ParseCommandOrExpression( const char* fname, std::deque<CToken>& tokens, CParseTree& parseTree, TAllVarsAreGlobals inAllVarsAreGlobals );	//!< Generates a handler named ":run" that returns the given expression.
		
		void	ParseTopLevelConstruct( std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens, CParseTree& parseTree, const char* scriptText );
		void	ParseDocumentation( std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens, CParseTree& parseTree, const char* scriptText );
		CFunctionDefinitionNode*	StartParsingFunctionDefinition( const std::string& handlerName, const std::string& userHandlerName, bool isCommand, size_t fcnLineNum, std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens, CParseTree& parseTree );	// Called by ParseFunctionDefinition.
		void	ParseFunctionDefinition( bool isCommand, std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens, CParseTree& parseTree );
		CValueNode	*	ParseFunctionCall( CParseTree& parseTree, CCodeBlockNodeBase* currFunction, bool isMessagePassing, std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens );
		void	ParsePassStatement( CParseTree& parseTree, CCodeBlockNodeBase* currFunction, std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens );
		void	ParseHandlerCall( CParseTree& parseTree, CCodeBlockNodeBase* currFunction, bool isMessagePassing, std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens );
		void	FindPrintHostCommand( LEOInstructionID* printInstrID, uint16_t* param1, uint32_t* param2 );
		void	ParsePutStatement( CParseTree& parseTree, CCodeBlockNodeBase* currFunction,
									std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens );
		void	ParseGetStatement( CParseTree& parseTree,
									CCodeBlockNodeBase* currFunction,
									std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens );
		void	ParseSetStatement( CParseTree& parseTree,
									CCodeBlockNodeBase* currFunction,
									std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens );
		void	ParseWebPageContentToken( CParseTree& parseTree, CCodeBlockNodeBase* currFunction,
									std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens );
		void	ParseHostCommand( CParseTree& parseTree, CCodeBlockNodeBase* currFunction,
									std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens );
		CValueNode*	ParseHostFunction( CParseTree& parseTree, CCodeBlockNodeBase* currFunction,
										std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens );
		CValueNode*	ParseHostEntityWithTable( CParseTree& parseTree, CCodeBlockNodeBase* currFunction,
										std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens,
										THostCommandEntry* inHostTable );
		void	ParseGlobalStatement( CParseTree& parseTree, CCodeBlockNodeBase* currFunction,
										std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens );
		void	ParseReturnStatement( CParseTree& parseTree,
										CCodeBlockNodeBase* currFunction,
										std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens );
		void	ParseDownloadStatement( const std::string& userHandlerName, CParseTree& parseTree,
										CCodeBlockNodeBase* currFunction,
										std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens );
		void	ParseRepeatForEachStatement( const std::string& userHandlerName, CParseTree& parseTree,
										CCodeBlockNodeBase* currFunction,
										std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens );
		void	ParseRepeatStatement( const std::string& userHandlerName, CParseTree& parseTree,
										CCodeBlockNodeBase* currFunction,
										std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens );
		void	ParseIfStatement( const std::string& userHandlerName, CParseTree& parseTree,
										CCodeBlockNodeBase* currFunction,
										std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens );
		CValueNode*	ParseContainer( bool asPointer, bool initWithName, CParseTree& parseTree,
										CCodeBlockNodeBase* currFunction,
										std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens,
										TIdentifierSubtype inEndToken );
		CValueNode*	ParseArrayItem( CParseTree& parseTree, CCodeBlockNodeBase* currFunction,
									std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens );
		void	ParseOneLine( const std::string& userHandlerName,
										CParseTree& parseTree,
										CCodeBlockNodeBase* currFunction,
										std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens,
										bool dontSwallowReturn = false );
		void	ParseFunctionBody( std::string& userHandlerName,
									CParseTree& parseTree, CCodeBlockNodeBase* currFunction,
									std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens,
									size_t *outEndLineNum = NULL, TIdentifierSubtype endIdentifier = EEndIdentifier,
									bool parseFirstLineAsReturnExpression = false );
		void	ParseParamList( TIdentifierSubtype identifierToEndOn,
								CParseTree& parseTree, CCodeBlockNodeBase* currFunction,
								std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens,
								CFunctionCallNode* inFCallToAddTo );
		CValueNode*	ParseObjCMethodCall( CParseTree& parseTree, CCodeBlockNodeBase* currFunction,
										std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens );
		CValueNode*	ParseNativeFunctionCallStartingAtParams( std::string& methodName, CObjCMethodEntry& methodInfo,
										CParseTree& parseTree, CCodeBlockNodeBase* currFunction,
										std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens );
		CValueNode* ParseNumberOfExpression( CParseTree& parseTree, CCodeBlockNodeBase* currFunction,
												std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens,
												TIdentifierSubtype inEndIdentifier );
		CValueNode*	ParseTerm( CParseTree& parseTree, CCodeBlockNodeBase* currFunction,
										std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens,
										TIdentifierSubtype inEndIdentifier );
		CValueNode*	ParseAnyFollowingArrayDefinitionWithKey(CValueNode* theTerm, CParseTree& parseTree, CCodeBlockNodeBase* currFunction,
								std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens,
								TIdentifierSubtype inEndIdentifier);	//!< Returns theTerm if it didn't find an array definition after this potential key in theTerm.
		CValueNode*	ParseAnyPostfixOperatorForTerm( CValueNode* theTerm, CParseTree& parseTree, CCodeBlockNodeBase* currFunction,
								std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens,
								TIdentifierSubtype inEndIdentifier );
		void	OutputExpressionStack( CParseTree& parseTree, CCodeBlockNodeBase* currFunction,
										std::deque<std::string>	&terms, std::deque<const char*>	&operators );
		void	CreateVariable( const std::string& varName, const std::string& realVarName, bool initWithName,
								CCodeBlockNodeBase* currFunction, bool isGlobal = false );
		TIdentifierSubtype	ParseOperator( std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens, int *outPrecedence, LEOInstructionID *outOpName );
		CValueNode*	ParseChunkExpression( TChunkType typeConstant, CParseTree& parseTree, CCodeBlockNodeBase* currFunction,
										std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens );
		CValueNode*	ParseConstantChunkExpression( TChunkType typeConstant, CParseTree& parseTree, CCodeBlockNodeBase* currFunction,
										std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens );
		CValueNode*	ParseExpression( CParseTree& parseTree, CCodeBlockNodeBase* currFunction,
										std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens, TIdentifierSubtype inEndToken );
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
		CValueNode*	ParseColumnRowExpression( CParseTree& parseTree, CCodeBlockNodeBase* currFunction,
								std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens, TIdentifierSubtype inEndToken );
		
		CValueNode*	CollapseExpressionStack( CParseTree& parseTree, std::deque<CValueNode*> &terms, std::deque<LEOInstructionID> &operators );
		
		std::string	GetFirstHandlerName()								{ return mFirstHandlerName; };
		const char*	GetSupportFolderPath()								{ return mSupportFolderPath; };
		void		SetSupportFolderPath( const char* spfp )			{ mSupportFolderPath = spfp; };
		
		void		GenerateObjCTypeToVariantCode( std::string type, std::string &prefix, std::string &suffix );
		void		GenerateVariantToObjCTypeCode( std::string type, std::string &prefix, std::string &suffix, std::string& ioValue );
		
		void		LoadNativeHeaders();
		void		SetWebPageEmbedMode( bool inState )	{ mWebPageEmbedMode = inState; }
		void		SetIncludeHandler( CParserIncludeHandler inIncludeHandler ) { mIncludeHandler = inIncludeHandler; };
		
		const std::vector<CMessageEntry>&		GetMessages()		{ return mMessages; };
		const std::vector<CHandlerNotesEntry>&	GetHandlerNotes()	{ return mHandlerNotes; };
		
	// statics:
		static TBuiltInFunctionEntry* GetBuiltInFunctionWithName( const std::string& inName );
		static void		LoadNativeHeadersFromFile( const char* filepath );	//!< Used to load OS-native API signatures and names from the frameworkheaders.hhc file.
		static void		SetFirstNativeCallCallback( LEOFirstNativeCallCallbackPtr inCallback );	//!< Callback to be invoked when the user actually triggers execution of the first OS-native API. Allows lazy-loading some parts of the system headers.
		static void		AddOperatorsAndOffsetInstructions( TOperatorEntry* inEntries, size_t firstOperatorInstruction );
		static void		AddUnaryOperatorsAndOffsetInstructions( TUnaryOperatorEntry* inEntries, size_t firstUnaryOperatorInstruction );
		static void		AddPostfixOperatorsAndOffsetInstructions( TUnaryOperatorEntry* inEntries, size_t firstUnaryOperatorInstruction );
		static void		AddBuiltInFunctionsAndOffsetInstructions( TBuiltInFunctionEntry* inEntries, size_t firstBuiltInFunctionInstruction );	//!< Register functions that take no params and can be called as "foo()" or "the foo" and map them to a bunch of instructions previously registered.
		static void		AddGlobalPropertiesAndOffsetInstructions( TGlobalPropertyEntry* inEntries, size_t firstGlobalPropertyInstruction );
		static void		AddHostCommandsAndOffsetInstructions( THostCommandEntry* inEntries, size_t firstHostCommandInstruction );
		static void		AddHostFunctionsAndOffsetInstructions( THostCommandEntry* inEntries, size_t firstHostCommandInstruction );
		static void		AddStringConstants( TStringConstantEntry* inEntries );
		static void		AddNumberConstants( TNumberConstantEntry* inEntries );
		static void		AddBuiltInVariables( TBuiltInVariableEntry* inEntries );
	};
}
