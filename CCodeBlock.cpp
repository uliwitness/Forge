/*
 *  CCodeBlock.cpp
 *  Forge
 *
 *  Created by Uli Kusterer on 31.10.10.
 *  Copyright 2010 Uli Kusterer. All rights reserved.
 *
 */

#include "CCodeBlock.h"
extern "C" {
#include "LEOScript.h"
#include "LEOContextGroup.h"
#include "LEOInstructions.h"
}

namespace Carlson
{

CCodeBlock::CCodeBlock( LEOContextGroup * inGroup, LEOScript* inScript, CCodeBlockProgressDelegate * progressDelegate )
	: mGroup(inGroup), mProgressDelegate(progressDelegate), mCurrentHandler(NULL)
{
	mScript = LEOScriptRetain(inScript);
	mGroup = LEOContextGroupRetain( inGroup );
}


CCodeBlock::~CCodeBlock()
{
	LEOScriptRelease( mScript );
	mCurrentHandler = NULL;
	mScript = NULL;
	LEOContextGroupRelease( mGroup );
	mGroup = NULL;
}


void	CCodeBlock::GenerateFunctionPrologForName( const std::string& inName )
{
	mProgressDelegate->CodeBlockAddingFunction( this, inName );
	
	LEOHandlerID handlerID = LEOContextGroupHandlerIDForHandlerName( mGroup, inName.c_str() );
	mCurrentHandler = LEOScriptAddCommandHandlerWithID( mScript, handlerID );
	
	//LEOHandlerAddInstruction( handler, LEOInstructionID instructionID, uint16_t param1, uint32_t param2);
}


void	CCodeBlock::GenerateFunctionEpilogForName( const std::string& inName )
{
	LEOHandlerAddInstruction( mCurrentHandler, RETURN_FROM_HANDLER_INSTR, 0, 0 );	// Make sure we return from this handler even if there's no explicit return statement.
	
	mCurrentHandler = NULL;
}


void	CCodeBlock::GeneratePushIntInstruction( int inNumber )
{
	LEOHandlerAddInstruction( mCurrentHandler, PUSH_INTEGER_INSTR, 0, (*(uint32_t*)&inNumber) );
}


void	CCodeBlock::GeneratePushFloatInstruction( float inNumber )
{
	LEOHandlerAddInstruction( mCurrentHandler, PUSH_NUMBER_INSTR, 0, (*(uint32_t*)&inNumber) );
}


void	CCodeBlock::GeneratePushBoolInstruction( bool inBoolean )
{
	LEOHandlerAddInstruction( mCurrentHandler, PUSH_BOOLEAN_INSTR, 0, inBoolean );
}


void	CCodeBlock::GeneratePushStringInstruction( const std::string& inString )
{
	size_t	stringIndex = LEOScriptAddString( mScript, inString.c_str() );
	LEOHandlerAddInstruction( mCurrentHandler, PUSH_STR_FROM_TABLE_INSTR, 0, stringIndex );
}


void	CCodeBlock::GeneratePushVariableInstruction( size_t bpRelativeOffset )
{
	
}


void	CCodeBlock::GeneratePopIntoVariableInstruction( size_t bpRelativeOffset )
{
	
}

}

