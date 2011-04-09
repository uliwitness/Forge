//
//  ForgeFramework.h
//  ForgeFramework
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


// -----------------------------------------------------------------------------
//	Data types:
// -----------------------------------------------------------------------------

typedef struct LEOParseTree	LEOParseTree;	// Private internal data structure representing a parse tree.



// -----------------------------------------------------------------------------
//	Forge methods:
// -----------------------------------------------------------------------------

LEOParseTree*	LEOParseTreeCreateFromUTF8Characters( const char* inCode, size_t codeLength, const char* filename );
void			LEOCleanUpParseTree( LEOParseTree* inTree );

void			LEOScriptCompileAndAddParseTree( LEOScript* inScript, LEOContextGroup* inGroup, LEOParseTree* inTree );

const char*		LEOParserGetLastErrorMessage();	// Call this after LEOParseTreeCreateFromUTF8Characters or LEOScriptCompileAndAddParseTree to detect errors. If it returns NULL, everything was fine.