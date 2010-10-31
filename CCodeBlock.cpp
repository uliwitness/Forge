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
}

namespace Carlson
{

CCodeBlock::CCodeBlock( LEOScript* inScript, CCodeBlockProgressDelegate * progressDelegate )
	: mProgressDelegate(progressDelegate)
{
	mScript = LEOScriptRetain(inScript);
}


CCodeBlock::~CCodeBlock()
{
	LEOScriptRelease( mScript );
	mScript = NULL;
}


}

