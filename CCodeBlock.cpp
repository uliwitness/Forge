/*
 *  CCodeBlock.cpp
 *  Forge
 *
 *  Created by Uli Kusterer on 31.10.10.
 *  Copyright 2010 Uli Kusterer. All rights reserved.
 *
 */

#include "CCodeBlock.h"
extern "C"
{
#include "LEOScript.h"
#include "LEOContextGroup.h"
#include "LEOInstructions.h"
#include "LEOMsgInstructions.h"
#include "LEOPropertyInstructions.h"
}

#include <vector>

namespace Carlson
{

CCodeBlock::CCodeBlock( LEOContextGroup * inGroup, LEOScript* inScript, uint16_t inFileID )
	: mGroup(NULL), mCurrentHandler(NULL), mScript(NULL), mFileID(inFileID)
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


static bool CompareBPOffsets( const std::pair<std::string,CVariableEntry> &a, const std::pair<std::string,CVariableEntry> &b )
{
	return a.second.mBPRelativeOffset < b.second.mBPRelativeOffset;
}


void	CCodeBlock::GenerateFunctionPrologForName( bool isCommand, const std::string& inName, const std::map<std::string,CVariableEntry>& inLocals, size_t lineNumber )
{
	// Create the handler:
	LEOHandlerID handlerID = LEOContextGroupHandlerIDForHandlerName( mGroup, inName.c_str() );
	if( isCommand )
		mCurrentHandler = LEOScriptAddCommandHandlerWithID( mScript, handlerID );
	else
		mCurrentHandler = LEOScriptAddFunctionHandlerWithID( mScript, handlerID );
	
	// Sort all variables in their BP-relative order so we can just push values
	//	to allocate space and initial values in order:
	std::map<std::string,CVariableEntry>::const_iterator		itty2;
	std::vector< std::pair<std::string,CVariableEntry> >		locals;
	
	for( itty2 = inLocals.begin(); itty2 != inLocals.end(); itty2++ )
		locals.push_back( *itty2 );
	std::sort( locals.begin(), locals.end(), CompareBPOffsets );
	
	// Actually generate the code now that we have the proper order:
	LEOHandlerAddInstruction( mCurrentHandler, LINE_MARKER_INSTR, mFileID, (uint32_t)lineNumber );
	size_t	emptyStringIndex = LEOScriptAddString( mScript, "" );
	mNumLocals = 0;
	std::vector< std::pair<std::string,CVariableEntry> >::const_iterator		itty;
	
	for( itty = locals.begin(); itty != locals.end(); itty++ )
	{
		if( itty->second.mBPRelativeOffset != LONG_MAX )
		{
			//printf( "%s: %s BP offset %ld\n", inName.c_str(), itty->second.mRealName.c_str(), itty->second.mBPRelativeOffset );
			if( itty->second.mIsGlobal )
			{
				size_t	stringIndex = LEOScriptAddString( mScript, itty->second.mRealName.c_str() );
				LEOHandlerAddInstruction( mCurrentHandler, PUSH_STR_VARIANT_FROM_TABLE_INSTR, 0, (uint32_t)stringIndex );
				LEOHandlerAddInstruction( mCurrentHandler, PUSH_GLOBAL_REFERENCE_INSTR, 0, 0 );
			}
			else
			{
				size_t	stringIndex = itty->second.mInitWithName ? LEOScriptAddString( mScript, itty->second.mRealName.c_str() ) : emptyStringIndex;
				LEOHandlerAddInstruction( mCurrentHandler, PUSH_STR_VARIANT_FROM_TABLE_INSTR, 0, (uint32_t)stringIndex );
			}
			LEOHandlerAddVariableNameMapping( mCurrentHandler, itty->first.c_str(), itty->second.mRealName.c_str(), itty->second.mBPRelativeOffset );
			mNumLocals++;
		}
		else
			; //printf( "Variable %s unused.\n", itty->second.mRealName.c_str() );
	}
}


void	CCodeBlock::PrepareToExitFunction( size_t lineNumber )
{
	LEOHandlerAddInstruction( mCurrentHandler, LINE_MARKER_INSTR, mFileID, (uint32_t)lineNumber );
	// Get rid of stack space allocated for our local variables:
	std::map<std::string,CVariableEntry>::const_iterator		itty;
	for( size_t	x = 0; x < mNumLocals; x++ )
		LEOHandlerAddInstruction( mCurrentHandler, POP_VALUE_INSTR, BACK_OF_STACK, 0 );
}


void	CCodeBlock::GenerateFunctionEpilogForName( bool isCommand, const std::string& inName, const std::map<std::string,CVariableEntry>& inLocals, size_t lineNumber )
{
	PrepareToExitFunction( lineNumber );
	
	// Make sure we return an empty result, even if there's no return statement at the end of the handler:
	size_t	emptyStringIndex = LEOScriptAddString( mScript, "" );
	LEOHandlerAddInstruction( mCurrentHandler, PUSH_STR_FROM_TABLE_INSTR, 0, (uint32_t)emptyStringIndex );
	GenerateSetReturnValueInstruction();
	LEOHandlerAddInstruction( mCurrentHandler, RETURN_FROM_HANDLER_INSTR, BACK_OF_STACK, 0 );	// Make sure we return from this handler even if there's no explicit return statement.
	
	mCurrentHandler = NULL;	// Be paranoid. Don't want to accidentally add stuff to a finished handler.
	mNumLocals = 0;
}


void	CCodeBlock::GenerateFunctionCallInstruction( bool isCommand, bool isMessagePassing, const std::string& inName )
{
	LEOHandlerID handlerID = LEOContextGroupHandlerIDForHandlerName( mGroup, inName.c_str() );
	LEOHandlerAddInstruction( mCurrentHandler, CALL_HANDLER_INSTR, (isCommand ? kLEOCallHandler_IsCommandFlag : kLEOCallHandler_IsFunctionFlag) | (isMessagePassing ? kLEOCallHandler_PassMessage : 0), handlerID );
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
	LEOHandlerAddInstruction( mCurrentHandler, PUSH_STR_FROM_TABLE_INSTR, 0, (uint32_t)stringIndex );
}


void	CCodeBlock::GeneratePushVariableInstruction( int16_t bpRelativeOffset )
{
	LEOHandlerAddInstruction( mCurrentHandler, PUSH_REFERENCE_INSTR, (*(uint16_t*)&bpRelativeOffset), 0 );
}


void	CCodeBlock::GeneratePopSimpleValueIntoVariableInstruction( int16_t bpRelativeOffset )
{
	LEOHandlerAddInstruction( mCurrentHandler, POP_SIMPLE_VALUE_INSTR, (*(uint16_t*)&bpRelativeOffset), 0 );
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
	LEOHandlerAddInstruction( mCurrentHandler, PRINT_VALUE_INSTR +kFirstMsgInstruction, BACK_OF_STACK, 0 );
}


void	CCodeBlock::GeneratePrintVariableInstruction( int16_t bpRelativeOffset )
{
	LEOHandlerAddInstruction( mCurrentHandler, PRINT_VALUE_INSTR +kFirstMsgInstruction, (*(uint16_t*)&bpRelativeOffset), 0 );
}


void	CCodeBlock::GenerateAssignParamValueToVariableInstruction( int16_t bpRelativeOffset, uint32_t paramNum )
{
	LEOHandlerAddInstruction( mCurrentHandler, PARAMETER_INSTR, (*(uint16_t*)&bpRelativeOffset), paramNum +1 );
}


void	CCodeBlock::GenerateAssignParamToVariableInstruction( int16_t bpRelativeOffset, uint32_t paramNum )
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


void	CCodeBlock::GenerateOperatorInstruction( LEOInstructionID inInstructionID, uint16_t inParam1, uint32_t inParam2 )
{
	LEOHandlerAddInstruction( mCurrentHandler, inInstructionID, inParam1, inParam2 );
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


void	CCodeBlock::GenerateLineMarkerInstruction( uint32_t inLineNum )
{
	LEOHandlerAddInstruction( mCurrentHandler, LINE_MARKER_INSTR, mFileID, inLineNum );
}


void	CCodeBlock::GenerateAssignChunkArrayInstruction( int16_t bpRelativeOffset, uint32_t inChunkType )
{
	LEOHandlerAddInstruction( mCurrentHandler, ASSIGN_CHUNK_ARRAY_INSTR, bpRelativeOffset, inChunkType );
}


void	CCodeBlock::GeneratePushChunkRefInstruction( int16_t bpRelativeOffset, uint32_t inChunkType )
{
	LEOHandlerAddInstruction( mCurrentHandler, PUSH_CHUNK_REFERENCE_INSTR, bpRelativeOffset, inChunkType );
}


void	CCodeBlock::GeneratePushChunkConstInstruction( int16_t bpRelativeOffset, uint32_t inChunkType )
{
	LEOHandlerAddInstruction( mCurrentHandler, PUSH_CHUNK_INSTR, bpRelativeOffset, inChunkType );
}


void	CCodeBlock::GenerateSetChunkPropertyInstruction( int16_t bpRelativeOffset, uint32_t inChunkType )
{
	LEOHandlerAddInstruction( mCurrentHandler, SET_CHUNK_PROPERTY_INSTR, bpRelativeOffset, inChunkType );
}


void	CCodeBlock::GeneratePushChunkPropertyInstruction( int16_t bpRelativeOffset, uint32_t inChunkType )
{
	LEOHandlerAddInstruction( mCurrentHandler, PUSH_CHUNK_PROPERTY_INSTR, bpRelativeOffset, inChunkType );
}


void	CCodeBlock::GenerateGetArrayItemCountInstruction( int16_t bpRelativeOffset )
{
	LEOHandlerAddInstruction( mCurrentHandler, GET_ARRAY_ITEM_COUNT_INSTR, bpRelativeOffset, 0 );
}


void	CCodeBlock::GenerateGetArrayItemInstruction( int16_t bpRelativeOffset )
{
	LEOHandlerAddInstruction( mCurrentHandler, GET_ARRAY_ITEM_INSTR, bpRelativeOffset, 0 );
}


void	CCodeBlock::GenerateSetStringInstruction( int16_t bpRelativeOffset )
{
	LEOHandlerAddInstruction( mCurrentHandler, SET_STRING_INSTR, bpRelativeOffset, 0 );
}


void	CCodeBlock::GeneratePutValueIntoValueInstruction()
{
	LEOHandlerAddInstruction( mCurrentHandler, PUT_VALUE_INTO_VALUE_INSTR, 0, 0 );
}


void	CCodeBlock::GeneratePushPropertyOfObjectInstruction()
{
	LEOHandlerAddInstruction( mCurrentHandler, kFirstPropertyInstruction +PUSH_PROPERTY_OF_OBJECT_INSTR, 0, 0 );
}


void	CCodeBlock::GenerateSetPropertyOfObjectInstruction()
{
	LEOHandlerAddInstruction( mCurrentHandler, kFirstPropertyInstruction +SET_PROPERTY_OF_OBJECT_INSTR, 0, 0 );
}


void	CCodeBlock::GeneratePushMeInstruction()
{
	LEOHandlerAddInstruction( mCurrentHandler, kFirstPropertyInstruction +PUSH_ME_INSTR, 0, 0 );
}

}

