/*
 *  CCodeBlock.h
 *  Forge
 *
 *  Created by Uli Kusterer on 31.10.10.
 *  Copyright 2010 Uli Kusterer. All rights reserved.
 *
 */

#include <string>


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
	
	void		GenerateFunctionPrologForName( bool isCommand, const std::string& inName, size_t inNumVariables );
	void		GenerateFunctionEpilogForName( bool isCommand, const std::string& inName, size_t inNumVariables );
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

	void		GenerateAssignParamToVariableInstruction( int16_t bpRelativeOffset, size_t paramNum );
	
protected:
	LEOScript*						mScript;
	LEOContextGroup*				mGroup;
	LEOHandler*						mCurrentHandler;
};

}