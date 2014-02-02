//
//  Forge.h
//  Forge
//
//  Created by Uli Kusterer on 09.04.11.
//  Copyright 2011 The Void Software. All rights reserved.
//

/*!
	@header Forge
	
	This is the public C API people who mainly want to use Hammer in a host
	application can use to parse and compile scripts, and register host-specific
	commands, functions and properties. For the actual C++ implementation of the
	parser, see the CParser class.
*/

#ifndef FORGE_H
#define FORGE_H

// -----------------------------------------------------------------------------
//	Headers:
// -----------------------------------------------------------------------------

// Leonie headers needed by Forge to compile code:
#include "LEOScript.h"
#include "LEOContextGroup.h"

// Leonie headers you may need to run Forge code:
#include "LEOValue.h"
#include "LEOInterpreter.h"
#include "LEOInstructions.h"
#include "LEODebugger.h"
#include "LEORemoteDebugger.h"
#include "LEOChunks.h"
#include "LEOContextGroup.h"
#include "LEOHandlerID.h"
#include "LEOPropertyInstructions.h"
#include "LEODownloadInstructions.h"
#include "LEOObjCCallInstructions.h"


#if __cplusplus
extern "C" {
#endif


// -----------------------------------------------------------------------------
//	Data types:
// -----------------------------------------------------------------------------

/*! LEOParseTree is a private, internal data structure representing a parse tree,
	i.e. it is a representation of a script's code in a more easily machine
	processible form than plain text. Internally it is equivalent to the
	CParseTree C++ class, but do not rely on that from your host application. */
typedef struct LEOParseTree	LEOParseTree;


typedef struct LEOLineIndentTable LEOLineIndentTable;


struct TGlobalPropertyEntry;
struct THostCommandEntry;


// -----------------------------------------------------------------------------
//	Forge methods:
// -----------------------------------------------------------------------------

/*! Create an abstract syntax tree from the given script text specified by <tt>inCode</tt>
	and <tt>codeLength</tt>. You can then call <tt>LEOScriptCompileAndAddParseTree</tt> to fill a <tt>LEOScript</tt> with the corresponding byte code. Once you are done with the parse tree, call <tt>LEOCleanUpParseTree</tt> to free the memory associated with it. Provide a file name (or other unique display name that debuggers should show to the user on script errors) for <tt>inFileID</tt> by calling <tt>LEOFileIDForFileName</tt>.
	@seealso //leo_ref/c/func/LEOParseTreeCreateForCommandOrExpressionFromUTF8Characters	LEOParseTreeCreateForCommandOrExpressionFromUTF8Characters
	@seealso //leo_ref/c/func/LEOScriptCompileAndAddParseTree	LEOScriptCompileAndAddParseTree
	@seealso //leo_ref/c/func/LEOFileIDForFileName	LEOFileIDForFileName
	@seealso //leo_ref/c/func/LEOParserGetLastErrorMessage	LEOParserGetLastErrorMessage
	@seealso //leo_ref/c/func/LEOCleanUpParseTree	LEOCleanUpParseTree */
LEOParseTree*	LEOParseTreeCreateFromUTF8Characters( const char* inCode, size_t codeLength, uint16_t inFileID );

/*! Create an abstract syntax tree from the given program fragment specified by <tt>inCode</tt>
	and <tt>codeLength</tt>. You can then call <tt>LEOScriptCompileAndAddParseTree</tt> to fill a <tt>LEOScript</tt> with the corresponding byte code. This can be used for evaluating a few commands, for example in the message box. Once you are done with the parse tree, call <tt>LEOCleanUpParseTree</tt> to free the memory associated with it. Provide a file name (or other unique display name that debuggers should show to the user on script errors) for <tt>inFileID</tt> by calling <tt>LEOFileIDForFileName</tt>.
	@note	If you want to parse a complete script, use <tt>LEOParseTreeCreateFromUTF8Characters</tt>.
	@seealso //leo_ref/c/func/LEOParseTreeCreateFromUTF8Characters	LEOParseTreeCreateFromUTF8Characters
	@seealso //leo_ref/c/func/LEOScriptCompileAndAddParseTree	LEOScriptCompileAndAddParseTree
	@seealso //leo_ref/c/func/LEOFileIDForFileName	LEOFileIDForFileName
	@seealso //leo_ref/c/func/LEOParserGetLastErrorMessage	LEOParserGetLastErrorMessage
	@seealso //leo_ref/c/func/LEOCleanUpParseTree	LEOCleanUpParseTree */
LEOParseTree*	LEOParseTreeCreateForCommandOrExpressionFromUTF8Characters( const char* inCode, size_t codeLength, uint16_t inFileID );

/*! Free the memory for a parse tree created by <tt>LEOParseTreeCreateFromUTF8Characters</tt> or <tt>LEOParseTreeCreateForCommandOrExpressionFromUTF8Characters</tt>.
	@seealso //leo_ref/c/func/LEOParseTreeCreateFromUTF8Characters	LEOParseTreeCreateFromUTF8Characters
	@seealso //leo_ref/c/func/LEOParseTreeCreateForCommandOrExpressionFromUTF8Characters	LEOParseTreeCreateForCommandOrExpressionFromUTF8Characters */
void			LEOCleanUpParseTree( LEOParseTree* inTree );


LEOLineIndentTable*	LEOLineIndentTableCreateForParseTree( LEOParseTree* inTree );

void				LEOCleanUpLineIndentTable( LEOLineIndentTable* inTable );


/*! Take a parse tree created by <tt>LEOParseTreeCreateFromUTF8Characters</tt> or <tt>LEOParseTreeCreateForCommandOrExpressionFromUTF8Characters</tt> and compile it into Leonie bytecode. The given script, <tt>inScript</tt> will be filled with the command and function handlers, strings etc. defined in the script. Handler IDs will be generated in the given context group. Provide the same file ID in <tt>inFileID</tt> that you generated using <tt>LEOFileIDForFileName</tt> when you created the parse tree.
	@seealso //leo_ref/c/func/LEOParseTreeCreateFromUTF8Characters	LEOParseTreeCreateFromUTF8Characters
	@seealso //leo_ref/c/func/LEOParseTreeCreateForCommandOrExpressionFromUTF8Characters	LEOParseTreeCreateForCommandOrExpressionFromUTF8Characters
	@seealso //leo_ref/c/func/LEOFileIDForFileName	LEOFileIDForFileName */
void			LEOScriptCompileAndAddParseTree( LEOScript* inScript, LEOContextGroup* inGroup, LEOParseTree* inTree, uint16_t inFileID );

/*! Call this after each call to <tt>LEOParseTreeCreateFromUTF8Characters</tt>, <tt>LEOParseTreeCreateForCommandOrExpressionFromUTF8Characters</tt> and <tt>LEOScriptCompileAndAddParseTree</tt> to detect errors. If it returns NULL, everything was fine.
	@seealso //leo_ref/c/func/LEOParseTreeCreateFromUTF8Characters	LEOParseTreeCreateFromUTF8Characters
	@seealso //leo_ref/c/func/LEOParseTreeCreateForCommandOrExpressionFromUTF8Characters	LEOParseTreeCreateForCommandOrExpressionFromUTF8Characters
	@seealso //leo_ref/c/func/LEOScriptCompileAndAddParseTree	LEOScriptCompileAndAddParseTree
*/
const char*		LEOParserGetLastErrorMessage( void );	

/*! You may call this after a call to <tt>LEOParserGetLastErrorMessage</tt> to determine the line in the script an error occurred on. If this returns SIZE_T_MAX, no line number information is available.
	@seealso //leo_ref/c/func/LEOParserGetLastErrorMessage	LEOParserGetLastErrorMessage
	@seealso //leo_ref/c/func/LEOParserGetLastErrorOffset	LEOParserGetLastErrorOffset
*/
size_t		LEOParserGetLastErrorLineNum( void );

/*! You may call this after a call to <tt>LEOParserGetLastErrorMessage</tt> to determine the character offset in the script an error occurred on. If this returns SIZE_T_MAX, no offset is available. Character offset information is usually only available for errors that happen early on in parsing. If you can't get an offset, you may want to try calling <tt>LEOParserGetLastErrorLineNum</tt>, which will often still at least be able to give you the line number for an error.
	@seealso //leo_ref/c/func/LEOParserGetLastErrorMessage	LEOParserGetLastErrorMessage
	@seealso //leo_ref/c/func/LEOParserGetLastErrorLineNum	LEOParserGetLastErrorLineNum
*/
size_t		LEOParserGetLastErrorOffset( void );

/*! Register the global property names and their corresponding instructions in <tt>inEntries</tt> with the Forge parser. The property array passed in is copied into Forge's internal tables, and its end detected by an entry with identifier type ELastIdentifier_Sentinel. You must have registered all instructions referenced here using the same call to <tt>LEOAddInstructionsToInstructionArray</tt>, and you must pass in the index of the first instruction as returned by that call in <tt>firstGlobalPropertyInstruction</tt>. If you want to specify an invalid instruction (e.g. to indicate a read-only or write-only property), you <i>must</i> use <tt>INVALID_INSTR2</tt>, as <tt>INVALID_INSTR</tt> is 0 and would thus be undistinguishable from your first instruction. */
void	LEOAddGlobalPropertiesAndOffsetInstructions( struct TGlobalPropertyEntry* inEntries, size_t firstGlobalPropertyInstruction );


/*! Register the built-in function names and their corresponding instructions in <tt>inEntries</tt> with the Forge parser. The array passed in is copied into Forge's internal tables, and its end detected by an entry with identifier type ELastIdentifier_Sentinel. You must have registered all instructions referenced here using the same call to <tt>LEOAddInstructionsToInstructionArray</tt>, and you must pass in the index of the first instruction as returned by that call in <tt>firstBuiltInFunctionInstruction</tt>. If you want to specify an invalid instruction (e.g. to indicate a read-only or write-only property), you <i>must</i> use <tt>INVALID_INSTR2</tt>, as <tt>INVALID_INSTR</tt> is 0 and would thus be undistinguishable from your first instruction. */

void	LEOAddBuiltInFunctionsAndOffsetInstructions( struct TBuiltInFunctionEntry* inEntries, size_t firstBuiltInFunctionInstruction );


/*! Register the syntax and the corresponding instructions for host-specific commands in <tt>inEntries</tt> with the Forge parser. The array passed in is copied into Forge's internal tables, and its end detected by an entry with identifier type ELastIdentifier_Sentinel.

	You must have registered all instructions referenced here using the same call to
	<tt>LEOAddInstructionsToInstructionArray</tt>, and you must pass in the index of
	the first instruction as returned by that call in <tt>firstGlobalPropertyInstruction</tt>.
	
	If you want to specify an invalid instruction (e.g. to indicate a read-only or write-only property), you <i>must</i> use <tt>INVALID_INSTR2</tt>, as <tt>INVALID_INSTR</tt> is 0 and would thus be undistinguishable from your first instruction. */
void	LEOAddHostCommandsAndOffsetInstructions( struct THostCommandEntry* inEntries, size_t firstHostCommandInstruction );

/*! Register the syntax and the corresponding instructions for host-specific functions in <tt>inEntries</tt> with the Forge parser (anything that can be a term in an expression, really, it doesn't have to be a function in the traditional sense.). The array passed in is copied into Forge's internal tables, and its end detected by an entry with identifier type ELastIdentifier_Sentinel.

	You must have registered all instructions referenced here using the same call to
	<tt>LEOAddInstructionsToInstructionArray</tt>, and you must pass in the index of the
	first instruction as returned by that call in <tt>firstGlobalPropertyInstruction</tt>.
	
	If you want to specify an invalid instruction (e.g. to indicate a read-only or write-only property), you <i>must</i> use <tt>INVALID_INSTR2</tt>, as <tt>INVALID_INSTR</tt> is 0 and would thus be undistinguishable from your first instruction. */
void	LEOAddHostFunctionsAndOffsetInstructions( struct THostCommandEntry* inEntries, size_t firstHostCommandInstruction );

/*! Load syntax for 'native calls' (i.e. native operating system APIs as you would call them from C, Objective C, C# or Pascal) from the specified .hhc header file, so the functions and methods in it become available to the parser as if they were handlers (just that they can return pointers and you might have to manage memory handed into or returned from them).
	@seealso //leo_ref/c/func/LEOSetFirstNativeCallCallback	LEOSetFirstNativeCallCallback
*/
void	LEOLoadNativeHeadersFromFile( const char* filepath );


/*! Since loading the headers for 'native calls' (i.e. native operating system APIs as you would call them from C, Objective C, C# or Pascal) may take a while, this callback is provided, which gets called the first time Forge needs to parse a native call, so you can lazily load the headers only when a script actually uses native calls and not incur the overhead otherwise.
	@seealso //leo_ref/c/func/LEOLoadNativeHeadersFromFile	LEOLoadNativeHeadersFromFile
*/
void	LEOSetFirstNativeCallCallback( LEOFirstNativeCallCallbackPtr inCallback );

#if __cplusplus
}
#endif

#endif /*FORGE_H*/
