/*
 *  CCodeBlock.h
 *  Forge
 *
 *  Created by Uli Kusterer on 31.10.10.
 *  Copyright 2010 Uli Kusterer. All rights reserved.
 *
 */

#include <string>
#include "CVariableEntry.h"
#include <map>
extern "C" {
#include "LEOInterpreter.h"
}

struct LEOScript;
struct LEOHandler;
struct LEOContextGroup;

namespace Carlson
{

class CCodeBlock;

class CCodeBlock
{
public:
	CCodeBlock( LEOContextGroup * inGroup, LEOScript* inScript );
	virtual ~CCodeBlock();
	
	void		GenerateFunctionPrologForName( bool isCommand, const std::string& inName, const std::map<std::string,CVariableEntry>& inLocals, size_t lineNumber );
	void		PrepareToExitFunction( size_t lineNumber );
	void		GenerateFunctionEpilogForName( bool isCommand, const std::string& inName, const std::map<std::string,CVariableEntry>& inLocals, size_t lineNumber );	// Calls PrepareToExitFunction.
	void		GenerateFunctionCallInstruction( bool isCommand, const std::string& inName );
	
	void		GeneratePushIntInstruction( int inNumber );
	void		GeneratePushFloatInstruction( float inNumber );
	void		GeneratePushBoolInstruction( bool inBoolean );
	void		GeneratePushStringInstruction( const std::string& inString );
	void		GeneratePushVariableInstruction( int16_t bpRelativeOffset );

	void		GeneratePopValueInstruction();
	void		GeneratePopIntoVariableInstruction( int16_t bpRelativeOffset );	// Maintains references.
	void		GeneratePopSimpleValueIntoVariableInstruction( int16_t bpRelativeOffset );	// Follows references.

	void		GeneratePrintValueInstruction();
	void		GeneratePrintVariableInstruction( int16_t bpRelativeOffset );

	void		GenerateAssignParamValueToVariableInstruction( int16_t bpRelativeOffset, size_t paramNum );
	void		GenerateAssignParamToVariableInstruction( int16_t bpRelativeOffset, size_t paramNum );
	void		GenerateReturnInstruction();
	void		GenerateSetReturnValueInstruction();

	size_t		GetNextInstructionOffset();	// Offset that next instruction added will have.
	
	void		GenerateJumpRelativeInstruction( int32_t numInstructions );
	void		GenerateJumpRelativeIfFalseInstruction( int32_t numInstructions );
	void		SetJumpAddressOfInstructionAtIndex( size_t idx, int32_t offs );
	
	void		GenerateAddNumberInstruction( int16_t bpRelativeOffset, LEONumber inNumber );
	void		GenerateAddIntegerInstruction( int16_t bpRelativeOffset, LEOInteger inNumber );
	void		GenerateOperatorInstruction( LEOInstructionID inInstructionID );
	
	void		GenerateLineMarkerInstruction( size_t inLineNum );
	
	void		GeneratePushChunkRefInstruction( int16_t bpRelativeOffset, uint32_t inChunkType );
	void		GenerateAssignChunkArrayInstruction( int16_t bpRelativeOffset, uint32_t inChunkType );
	void		GenerateGetArrayItemCountInstruction( int16_t bpRelativeOffset );
	void		GenerateGetArrayItemInstruction( int16_t bpRelativeOffset );
	
	void		GenerateSetStringInstruction( int16_t bpRelativeOffset );
	
protected:
	LEOScript*				mScript;
	LEOContextGroup*		mGroup;
	LEOHandler*				mCurrentHandler;
	size_t					mNumLocals;
};

}