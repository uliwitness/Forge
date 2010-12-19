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

CCodeBlock::CCodeBlock( LEOContextGroup * inGroup, LEOScript* inScript )
	: mGroup(NULL), mCurrentHandler(NULL), mScript(NULL)
{
	mScript = LEOScriptRetain( inScript );
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


void	CCodeBlock::GenerateFunctionPrologForName( const std::string& inName, size_t inNumVariables )
{
	// Create the handler:
	LEOHandlerID handlerID = LEOContextGroupHandlerIDForHandlerName( mGroup, inName.c_str() );
	mCurrentHandler = LEOScriptAddCommandHandlerWithID( mScript, handlerID );
	
	// Allocate stack space for our local variables:
	size_t	stringIndex = LEOScriptAddString( mScript, "" );
	for( size_t x = 0; x < inNumVariables; x++ )
		LEOHandlerAddInstruction( mCurrentHandler, PUSH_STR_FROM_TABLE_INSTR, 0, stringIndex );
}


void	CCodeBlock::GenerateFunctionEpilogForName( const std::string& inName, size_t inNumVariables )
{
	// Get rid of stack space allocated for our local variables:
	for( size_t x = 0; x < inNumVariables; x++ )
		LEOHandlerAddInstruction( mCurrentHandler, POP_VALUE_INSTR, BACK_OF_STACK, 0 );
	
	// Make sure we return, even if there's no return statement at the end of the handler:
	LEOHandlerAddInstruction( mCurrentHandler, RETURN_FROM_HANDLER_INSTR, 0, 0 );	// Make sure we return from this handler even if there's no explicit return statement.
	
	mCurrentHandler = NULL;	// Be paranoid. Don't want to accidentally add stuff to a finished handler.
}


void	CCodeBlock::GenerateFunctionCallInstruction( const std::string& inName )
{
	LEOHandlerID handlerID = LEOContextGroupHandlerIDForHandlerName( mGroup, inName.c_str() );
	LEOHandlerAddInstruction( mCurrentHandler, CALL_HANDLER_INSTR, 0, handlerID );
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


void	CCodeBlock::GeneratePushVariableInstruction( int16_t bpRelativeOffset )
{
	LEOHandlerAddInstruction( mCurrentHandler, PUSH_REFERENCE_INSTR, (*(uint16_t*)&bpRelativeOffset), 0 );
}


void	CCodeBlock::GeneratePopIntoVariableInstruction( int16_t bpRelativeOffset )
{
	LEOHandlerAddInstruction( mCurrentHandler, POP_VALUE_INSTR, (*(uint16_t*)&bpRelativeOffset), 0 );
}


void	CCodeBlock::GeneratePopValueInstruction()
{
	LEOHandlerAddInstruction( mCurrentHandler, POP_VALUE_INSTR, BACK_OF_STACK, 0 );
}


void	CCodeBlock::GeneratePrintValueInstruction()
{
	LEOHandlerAddInstruction( mCurrentHandler, PRINT_VALUE_INSTR, BACK_OF_STACK, 0 );
}


void	CCodeBlock::GeneratePrintVariableInstruction( int16_t bpRelativeOffset )
{
	LEOHandlerAddInstruction( mCurrentHandler, PRINT_VALUE_INSTR, (*(uint16_t*)&bpRelativeOffset), 0 );
}


void	CCodeBlock::GenerateAssignParamToVariableInstruction( int16_t bpRelativeOffset, size_t paramNum )
{
	LEOHandlerAddInstruction( mCurrentHandler, PARAMETER_INSTR, (*(uint16_t*)&bpRelativeOffset), paramNum +1 );
}

}

