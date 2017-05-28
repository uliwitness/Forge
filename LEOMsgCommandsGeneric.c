/*
 *  LEOMsgCommandsGeneric.c
 *  Leonie
 *
 *  Created by Uli Kusterer on 09.10.10.
 *  Copyright 2010 Uli Kusterer. All rights reserved.
 *
 */

/*!
	@header LEOMsgCommandsGeneric
	Syntax for 'put' without destination and 'delete' without support for objects
	that map to the generic 'print value' and 'delete value' instructions.
*/

#include "LEOMsgCommandsGeneric.h"


struct THostCommandEntry		gMsgCommands[] =
{
	{
		EPutIdentifier, PRINT_VALUE_INSTR, BACK_OF_STACK, 0, '\0', 'X',
		{
			{ EHostParamExpression, ELastIdentifier_Sentinel, EHostParameterRequired, INVALID_INSTR2, 0, 0, '\0', 'X' },
			{ EHostParam_Sentinel, ELastIdentifier_Sentinel, EHostParameterOptional, INVALID_INSTR2, 0, 0, '\0', '\0' }
		}
	},
	{
		EOutputIdentifier, PRINT_VALUE_INSTR, BACK_OF_STACK, 0, '\0', 'X',
		{
			{ EHostParamExpression, ELastIdentifier_Sentinel, EHostParameterRequired, INVALID_INSTR2, 0, 0, '\0', 'X' },
			{ EHostParam_Sentinel, ELastIdentifier_Sentinel, EHostParameterOptional, INVALID_INSTR2, 0, 0, '\0', '\0' }
		}
	},
	{
		EDeleteIdentifier, DELETE_VALUE_INSTR, 0, 0, '\0', 'X',
		{
			{ EHostParamContainer, ELastIdentifier_Sentinel, EHostParameterRequired, INVALID_INSTR2, 0, 0, '\0', 'X' },
			{ EHostParam_Sentinel, ELastIdentifier_Sentinel, EHostParameterOptional, INVALID_INSTR2, 0, 0, '\0', '\0' }
		}
	},
	{
		ELastIdentifier_Sentinel, INVALID_INSTR2, 0, 0, '\0', '\0',
		{
			{ EHostParam_Sentinel, ELastIdentifier_Sentinel, EHostParameterOptional, INVALID_INSTR2, 0, 0, '\0', '\0' }
		}
	}
};
