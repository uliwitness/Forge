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


void	CCodeBlock::GenerateFunctionPrologForName( bool isCommand, const std::string& inName, const std::map<std::string,CVariableEntry>& inLocals )
{
	// Create the handler:
	LEOHandlerID handlerID = LEOContextGroupHandlerIDForHandlerName( mGroup, inName.c_str() );
	if( isCommand )
		mCurrentHandler = LEOScriptAddCommandHandlerWithID( mScript, handlerID );
	else
		mCurrentHandler = LEOScriptAddFunctionHandlerWithID( mScript, handlerID );
	
	// Allocate stack space for our local variables:
	size_t	emptyStringIndex = LEOScriptAddString( mScript, "" );
	std::map<std::string,CVariableEntry>::const_iterator		itty;
	for( itty = inLocals.begin(); itty != inLocals.end(); itty++ )
	{
		size_t	stringIndex = itty->second.mInitWithName ? LEOScriptAddString( mScript, itty->second.mRealName.c_str() ) : emptyStringIndex;
		LEOHandlerAddInstruction( mCurrentHandler, PUSH_STR_FROM_TABLE_INSTR, 0, stringIndex );
	}
	
	mNumLocals = inLocals.size();
}


void	CCodeBlock::PrepareToExitFunction()
{
	// Get rid of stack space allocated for our local variables:
	std::map<std::string,CVariableEntry>::const_iterator		itty;
	for( size_t	x = 0; x < mNumLocals; x++ )
		LEOHandlerAddInstruction( mCurrentHandler, POP_VALUE_INSTR, BACK_OF_STACK, 0 );
}


void	CCodeBlock::GenerateFunctionEpilogForName( bool isCommand, const std::string& inName, const std::map<std::string,CVariableEntry>& inLocals )
{
	PrepareToExitFunction();
	
	// Make sure we return an empty result, even if there's no return statement at the end of the handler:
	size_t	emptyStringIndex = LEOScriptAddString( mScript, "" );
	LEOHandlerAddInstruction( mCurrentHandler, PUSH_STR_FROM_TABLE_INSTR, 0, emptyStringIndex );
	LEOHandlerAddInstruction( mCurrentHandler, RETURN_FROM_HANDLER_INSTR, BACK_OF_STACK, 0 );	// Make sure we return from this handler even if there's no explicit return statement.
	
	mCurrentHandler = NULL;	// Be paranoid. Don't want to accidentally add stuff to a finished handler.
	mNumLocals = 0;
}


void	CCodeBlock::GenerateFunctionCallInstruction( bool isCommand, const std::string& inName )
{
	LEOHandlerID handlerID = LEOContextGroupHandlerIDForHandlerName( mGroup, inName.c_str() );
	LEOHandlerAddInstruction( mCurrentHandler, CALL_HANDLER_INSTR, (isCommand ? 0 : 1), handlerID );
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


void	CCodeBlock::GenerateAssignParamValueToVariableInstruction( int16_t bpRelativeOffset, size_t paramNum )
{
	LEOHandlerAddInstruction( mCurrentHandler, PARAMETER_INSTR, (*(uint16_t*)&bpRelativeOffset), paramNum +1 );
}


void	CCodeBlock::GenerateAssignParamToVariableInstruction( int16_t bpRelativeOffset, size_t paramNum )
{
	LEOHandlerAddInstruction( mCurrentHandler, PARAMETER_KEEPREFS_INSTR, (*(uint16_t*)&bpRelativeOffset), paramNum +1 );
}


void	CCodeBlock::GenerateReturnInstruction()
{
	LEOHandlerAddInstruction( mCurrentHandler, RETURN_FROM_HANDLER_INSTR, 0, 0 );
}


void	CCodeBlock::GenerateSetReturnValueInstruction()
{
	LEOHandlerAddInstruction( mCurrentHandler, SET_RETURN_VALUE_INSTR, 0, 0 );
}


void	CCodeBlock::GenerateOperatorInstruction( LEOInstructionID inInstructionID )
{
	LEOHandlerAddInstruction( mCurrentHandler, inInstructionID, 0, 0 );
}


size_t	CCodeBlock::GetNextInstructionOffset()
{
	return mCurrentHandler->numInstructions;
}


void	CCodeBlock::GenerateJumpRelativeInstruction( int32_t numInstructions )
{
	LEOHandlerAddInstruction( mCurrentHandler, JUMP_RELATIVE_INSTR, BACK_OF_STACK,
								(*(uint32_t*)&numInstructions) );
}


void	CCodeBlock::GenerateJumpRelativeIfFalseInstruction( int32_t numInstructions )
{
	LEOHandlerAddInstruction( mCurrentHandler, JUMP_RELATIVE_IF_FALSE_INSTR, BACK_OF_STACK,
								(*(uint32_t*)&numInstructions) );
}


void	CCodeBlock::SetJumpAddressOfInstructionAtIndex( size_t idx, int32_t offs )
{
	assert( mCurrentHandler->instructions[idx].instructionID == JUMP_RELATIVE_INSTR
			|| mCurrentHandler->instructions[idx].instructionID == JUMP_RELATIVE_IF_FALSE_INSTR );
	
	mCurrentHandler->instructions[idx].param2 = (*(uint32_t*)&offs);
}


void	CCodeBlock::GenerateAddNumberInstruction( int16_t bpRelativeOffset, LEONumber inNumber )
{
	LEOHandlerAddInstruction( mCurrentHandler, ADD_NUMBER_INSTR, (*(uint16_t*)&bpRelativeOffset), (*(uint32_t*)&inNumber) );
}


void	CCodeBlock::GenerateAddIntegerInstruction( int16_t bpRelativeOffset, LEOInteger inNumber )
{
	LEOHandlerAddInstruction( mCurrentHandler, ADD_INTEGER_INSTR, (*(uint16_t*)&bpRelativeOffset), (*(uint32_t*)&inNumber) );
}


void	CCodeBlock::GenerateLineMarkerInstruction( size_t inLineNum )
{
	LEOHandlerAddInstruction( mCurrentHandler, LINE_MARKER_INSTR, 0, inLineNum );
}


void	CCodeBlock::GenerateAssignChunkArrayInstruction( int16_t bpRelativeOffset, uint32_t inChunkType )
{
	LEOHandlerAddInstruction( mCurrentHandler, ASSIGN_CHUNK_ARRAY_INSTR, bpRelativeOffset, inChunkType );
}


void	CCodeBlock::GenerateGetArrayItemCountInstruction( int16_t bpRelativeOffset )
{
	LEOHandlerAddInstruction( mCurrentHandler, GET_ARRAY_ITEM_COUNT_INSTR, bpRelativeOffset, 0 );
}


void	CCodeBlock::GenerateGetArrayItemInstruction( int16_t bpRelativeOffset )
{
	LEOHandlerAddInstruction( mCurrentHandler, GET_ARRAY_ITEM_INSTR, bpRelativeOffset, 0 );
}

}

