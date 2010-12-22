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
	
	void		GenerateFunctionPrologForName( bool isCommand, const std::string& inName, const std::map<std::string,CVariableEntry>& inLocals );
	void		PrepareToExitFunction();
	void		GenerateFunctionEpilogForName( bool isCommand, const std::string& inName, const std::map<std::string,CVariableEntry>& inLocals );	// Calls PrepareToExitFunction.
	void		GenerateFunctionCallInstruction( bool isCommand, const std::string& inName );
	
	void		GeneratePushIntInstruction( int inNumber );
	void		GeneratePushFloatInstruction( float inNumber );
	void		GeneratePushBoolInstruction( bool inBoolean );
	void		GeneratePushStringInstruction( const std::string& inString );
	void		GeneratePushVariableInstruction( int16_t bpRelativeOffset );

	void		GeneratePopValueInstruction();
	void		GeneratePopIntoVariableInstruction( int16_t bpRelativeOffset );

	void		GeneratePrintValueInstruction();
	void		GeneratePrintVariableInstruction( int16_t bpRelativeOffset );

	void		GenerateAssignParamValueToVariableInstruction( int16_t bpRelativeOffset, size_t paramNum );
	void		GenerateAssignParamToVariableInstruction( int16_t bpRelativeOffset, size_t paramNum );
	void		GenerateReturnInstruction();
	void		GenerateSetReturnValueInstruction();

	void		GenerateOperatorInstruction( LEOInstructionID inInstructionID );
	
protected:
	LEOScript*				mScript;
	LEOContextGroup*		mGroup;
	LEOHandler*				mCurrentHandler;
	size_t					mNumLocals;
};

}