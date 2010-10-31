/*
 *  CWhileLoopNode.cpp
 *  HyperCompiler
 *
 *  Created by Uli Kusterer on 19.05.07.
 *  Copyright 2007 M. Uli Kusterer. All rights reserved.
 *
 */

#include "CWhileLoopNode.h"

namespace Carlson
{


void	CWhileLoopNode::GenerateCpp( CppBlock& codeBlock )
{
	codeBlock.generate_while_loop_start();
	
	mCondition->GenerateCpp( codeBlock );
	
	codeBlock.generate_while_loop_middle();
	
	CCodeBlockNodeBase::GenerateCpp( codeBlock );
	
	codeBlock.generate_while_loop_end();
}


} /*Carlson*/