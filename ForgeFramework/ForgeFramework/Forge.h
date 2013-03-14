//
//  Forge.h
//  Forge
//
//  Created by Uli Kusterer on 09.04.11.
//  Copyright 2011 The Void Software. All rights reserved.
//

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
#include "LEOMsgInstructions.h"
#include "LEOPropertyInstructions.h"


// -----------------------------------------------------------------------------
//	Data types:
// -----------------------------------------------------------------------------

typedef struct LEOParseTree	LEOParseTree;	// Private internal data structure representing a parse tree.



// -----------------------------------------------------------------------------
//	Forge methods:
// -----------------------------------------------------------------------------

LEOParseTree*	LEOParseTreeCreateFromUTF8Characters( const char* inCode, size_t codeLength, uint16_t inFileID );
LEOParseTree*	LEOParseTreeCreateForCommandOrExpressionFromUTF8Characters( const char* inCode, size_t codeLength, uint16_t inFileID );

void			LEOCleanUpParseTree( LEOParseTree* inTree );

void			LEOScriptCompileAndAddParseTree( LEOScript* inScript, LEOContextGroup* inGroup, LEOParseTree* inTree, uint16_t inFileID );

const char*		LEOParserGetLastErrorMessage();	// Call this after LEOParseTreeCreateFromUTF8Characters or LEOScriptCompileAndAddParseTree to detect errors. If it returns NULL, everything was fine.

void	LEOAddGlobalPropertiesAndOffsetInstructions( struct TGlobalPropertyEntry* inEntries, size_t firstGlobalPropertyInstruction );

void	LEOAddHostCommandsAndOffsetInstructions( struct THostCommandEntry* inEntries, size_t firstHostCommandInstruction );
void	LEOAddHostFunctionsAndOffsetInstructions( struct THostCommandEntry* inEntries, size_t firstHostCommandInstruction );
