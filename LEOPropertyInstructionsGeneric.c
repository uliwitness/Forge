/*
 *  LEOMsgCommandsGeneric.c
 *  Leonie
 *
 *  Created by Uli Kusterer on 09.10.10.
 *  Copyright 2010 Uli Kusterer. All rights reserved.
 *
 */

/*!
	@header LEOPropertyInstructionsGeneric
	Instructions to implement property syntax for values only.
*/

#include "LEOPropertyInstructions.h"
#include <stdio.h>


void	LEOPushPropertyOfObjectInstruction( LEOContext* inContext );
void	LEOSetPropertyOfObjectInstruction( LEOContext* inContext );
void	LEOPushMeInstruction( LEOContext* inContext );
void	LEOHasPropertyInstruction( LEOContext* inContext );
void	LEOIHavePropertyInstruction( LEOContext* inContext );


/*!
	Push the value of a property of an object onto the stack, ready for use e.g.
	in an expression. Two parameters need to be pushed on the stack before
	calling this and will be popped off the stack by this instruction before
	the property value is pushed:
	
	propertyName -	The name of the property to retrieve, as a string or some
					value that converts to a string.
	
	object -		The object from which to retrieve the property, as a
					WILDObjectValue (i.e. isa = kLeoValueTypeWILDObject or isa = kLeoValueTypeObjectDescriptor).
	
	(PUSH_PROPERTY_OF_OBJECT_INSTR)
*/
void	LEOPushPropertyOfObjectInstruction( LEOContext* inContext )
{
	LEODebugPrintContext( inContext );
	
	LEOValuePtr		theKeyPath = inContext->stackEndPtr -2;
	LEOValuePtr		theObject = inContext->stackEndPtr -1;
	union LEOValue tmpKeyPath;
	LEOInitSimpleCopy(theKeyPath, &tmpKeyPath, kLEOInvalidateReferences, inContext );
	LEOCleanUpValue(theKeyPath, kLEOInvalidateReferences, inContext);
	
	long long numKeys = LEOGetKeyCount( &tmpKeyPath, inContext );
	for( long long x = 1; x <= numKeys; ++x )
	{
		union LEOValue tmpValue;
		char indexStr[256] = {0};
		snprintf( indexStr, sizeof(indexStr) -1, "%lld", x );
		LEOValuePtr thePropertyName = LEOGetValueForKey( &tmpKeyPath, indexStr, &tmpValue, kLEOInvalidateReferences, inContext );
		char		propNameStr[1024] = { 0 };
		LEOGetValueAsString( thePropertyName, propNameStr, sizeof(propNameStr), inContext );
		
		LEOValuePtr foundObject = LEOGetValueForKey( theObject, propNameStr, &tmpValue, kLEOInvalidateReferences, inContext );
		theObject = foundObject;
	}

	LEODebugPrintContext( inContext );

	union LEOValue tmp = *theKeyPath;
	*theKeyPath = *theObject;
	*theObject = tmp;
	LEOCleanUpStackToPtr( inContext, inContext->stackEndPtr -1 );

	LEODebugPrintContext( inContext );

	inContext->currentInstruction++;
}


/*!
	Change the value of a particular property of an object. Three parameters must
	have been pushed on the stack before this instruction is called, and will be
	popped off the stack:
	
	propertyName -	An array containing names of the properties to change, as an array
					of string values or values that convert to a string. If more than
					one item is passed, the code will descend into the object and set
					the keys in the array.
					
	object -		The object to change the property on. This must be a
					WILDObjectValue (i.e. isa = kLeoValueTypeWILDObject or isa = kLeoValueTypeObjectDescriptor).
	
	value -			The new value to assign to the given property.
	
	(SET_PROPERTY_OF_OBJECT_INSTR)
*/
void	LEOSetPropertyOfObjectInstruction( LEOContext* inContext )
{
	LEOValuePtr		theValue = inContext->stackEndPtr -1;
	LEOValuePtr		theObject = inContext->stackEndPtr -2;
	LEOValuePtr		theKeyPath = inContext->stackEndPtr -3;
	
	LEODebugPrintContext( inContext );
	
	long long numKeys = LEOGetKeyCount( theKeyPath, inContext );
	for( long long x = 1; x <= numKeys; ++x )
	{
		union LEOValue tmpValue;
		char indexStr[256] = {0};
		snprintf( indexStr, sizeof(indexStr) -1, "%lld", x );
		LEOValuePtr thePropertyName = LEOGetValueForKey( theKeyPath, indexStr, &tmpValue, kLEOInvalidateReferences, inContext );
		char		propNameStr[1024] = { 0 };
		LEOGetValueAsString( thePropertyName, propNameStr, sizeof(propNameStr), inContext );
		
		LEOValuePtr foundObject = LEOGetValueForKey( theObject, propNameStr, &tmpValue, kLEOInvalidateReferences, inContext );
		if( !foundObject )
		{
			inContext->flags |= kLEOContextKeepRunning;
			inContext->errMsg[0] = 0;
			
			union LEOValue emptyValue;
			LEOInitStringConstantValue( &emptyValue, "", kLEOInvalidateReferences, inContext );
			LEOSetValueForKey( theObject, propNameStr, &emptyValue, inContext );
			LEOCleanUpValue( &emptyValue, kLEOInvalidateReferences, inContext );
			foundObject = LEOGetValueForKey( theObject, propNameStr, &tmpValue, kLEOInvalidateReferences, inContext );
		}
		theObject = foundObject;
	}
	
	LEOPutValueIntoValue( theValue, theObject, inContext );
	
	LEOCleanUpStackToPtr( inContext, inContext->stackEndPtr -3 );
	
	inContext->currentInstruction++;

	LEODebugPrintContext( inContext );
}


/*!
	This instruction pushes a reference to the object owning the current script
	onto the stack. It implements the 'me' object specifier for Hammer.
	
	(PUSH_ME_INSTR)
*/

void	LEOPushMeInstruction( LEOContext* inContext )
{
	inContext->stackEndPtr++;
	
	LEOInitUnsetValue( inContext->stackEndPtr -1, kLEOInvalidateReferences, inContext );
	
	inContext->currentInstruction++;
}


/*!
	This instruction pushes a boolean that indicates whether the given object
	has the given property.
	
	(HAS_PROPERTY_INSTR)
*/

void	LEOHasPropertyInstruction( LEOContext* inContext )
{
	LEOCleanUpStackToPtr( inContext, inContext->stackEndPtr -1 );
	LEOCleanUpValue( inContext->stackEndPtr -1, kLEOInvalidateReferences, inContext );
	LEOInitBooleanValue( inContext->stackEndPtr -1, false, kLEOInvalidateReferences, inContext );
	
	inContext->currentInstruction++;
}


/*
	This instruction pushes a boolean that indicates whether the object
	owning the script has the given property.
 
	(I_HAVE_PROPERTY_INSTRUCTION)
*/

void	LEOIHavePropertyInstruction( LEOContext* inContext )
{
	LEODebugPrintContext( inContext );
	
	LEOCleanUpValue( inContext->stackEndPtr -1, kLEOInvalidateReferences, inContext );
	LEOInitBooleanValue( inContext->stackEndPtr -1, false, kLEOInvalidateReferences, inContext );
	
	inContext->currentInstruction++;
}


LEOINSTR_START(Property,LEO_NUMBER_OF_PROPERTY_INSTRUCTIONS)
LEOINSTR(LEOPushPropertyOfObjectInstruction)
LEOINSTR(LEOSetPropertyOfObjectInstruction)
LEOINSTR(LEOPushMeInstruction)
LEOINSTR(LEOHasPropertyInstruction)
LEOINSTR_LAST(LEOIHavePropertyInstruction)
