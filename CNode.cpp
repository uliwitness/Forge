/*
 *  CNode.cpp
 *  HyperCompiler
 *
 *  Created by Uli Kusterer on 10.05.07.
 *  Copyright 2007 M. Uli Kusterer. All rights reserved.
 *
 */

#include "CNode.h"
#include "CNodeTransformation.h"


using namespace Carlson;


void	CNode::Simplify()
{
	CNodeTransformationBase::Apply( this );
}