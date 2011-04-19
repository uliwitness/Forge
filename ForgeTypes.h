//
//  ForgeTypes.h
//  Forge
//
//  Created by Uli Kusterer on 16.04.11.
//  Copyright 2011 Uli Kusterer. All rights reserved.
//

#ifndef FORGE_TYPES_H
#define FORGE_TYPES_H		1

#include "LEOInterpreter.h"


/* The various built-in identifiers the parser recognizes: */
typedef enum
{
	EFunctionIdentifier = 0,
	EEndIdentifier,
	EPlusOperator,
	EMinusOperator,
	EMultiplyOperator,
	EDivideOperator,
	ENewlineOperator,
	ELessThanOperator,
	EGreaterThanOperator,
	EOnIdentifier,
	ECommaOperator,
	EOpenBracketOperator,
	ECloseBracketOperator,
	EPutIdentifier,
	EIntoIdentifier,
	EAfterIdentifier,
	EBeforeIdentifier,
	EReturnIdentifier,
	ETheIdentifier,
	EResultIdentifier,
	ERepeatIdentifier,
	EWhileIdentifier,
	EForIdentifier,
	ETimesIdentifier,
	EWithIdentifier,
	EToIdentifier,
	EEqualsOperator,
	EAmpersandOperator,
	EIfIdentifier,
	EThenIdentifier,
	EElseIdentifier,
	EFromIdentifier,
	ETrueIdentifier,
	EFalseIdentifier,
	EEmptyIdentifier,
	EOpenSquareBracketOperator,
	ECloseSquareBracketOperator,
	EColonOperator,
	EPeriodOperator,
	EQuoteIdentifier,
	EDownIdentifier,
	EUntilIdentifier,
	EItemIdentifier,
	EOfIdentifier,
	ENewlineIdentifier,
	EAddIdentifier,
	ESubtractIdentifier,
	EMultiplyIdentifier,
	EDivideIdentifier,
	EByIdentifier,
	ECharacterIdentifier,
	ECharIdentifier,
	EWordIdentifier,
	ELineIdentifier,
	EEntryIdentifier,
	EParamIdentifier,
	EParameterIdentifier,
	EParamCountIdentifier,
	EUnsetIdentifier,
	EIsIdentifier,
	ENotIdentifier,
	EExponentOperator,
	EModIdentifier,
	EModuloIdentifier,
	EDeleteIdentifier,
	EItemDelIdentifier,
	EItemDelimIdentifier,
	EItemDelimiterIdentifier,
	ENumberIdentifier,
	ECharactersIdentifier,
	ECharsIdentifier,
	EWordsIdentifier,
	ELinesIdentifier,
	EItemsIdentifier,
	EAtSignOperator,
	EAndIdentifier,
	EOrIdentifier,
	EExitIdentifier,
	ENextIdentifier,
	EEachIdentifier,
	ECommaIdentifier,
	EColonIdentifier,
	ECrIdentifier,
	ELineFeedIdentifier,
	ENullIdentifier,
	ESpaceIdentifier,
	ETabIdentifier,
	EPiIdentifier,
	EGetIdentifier,
	EShortIdentifier,
	EAbbrIdentifier,
	EAbbrevIdentifier,
	EAbbreviatedIdentifier,
	ELongIdentifier,
	ENumIdentifier,
	ESetIdentifier,
	ENullPointerIdentifier,
	EIdIdentifier,
	EHandlerIdentifier,
	EMessageIdentifier,
	EGlobalIdentifier,
	EPrivateIdentifier,
	EPublicIdentifier,
	EDoubleAmpersandPseudoOperator,
	ENotEqualPseudoOperator,
	ELessThanEqualPseudoOperator,
	EGreaterThanEqualPseudoOperator,
	EThruIdentifier,
	EThroughIdentifier,
	EInIdentifier,
	EByteIdentifier,
	EBytesIdentifier,
	EGoIdentifier,
	ECursorIdentifier,
	EStackIdentifier,
	EBackgroundIdentifier,
	EBkgndIdentifier,
	EBkgdIdentifier,
	EBgIdentifier,
	ECardIdentifier,
	ECdIdentifier,
	EFieldIdentifier,
	EFldIdentifier,
	EButtonIdentifier,
	EBtnIdentifier,
	EPartIdentifier,
	EPreviousIdentifier,
	EPrevIdentifier,
	EFirstIdentifier,
	ELastIdentifier,
	EVisualIdentifier,
	EEffectIdentifier,
	ELastIdentifier_Sentinel	// Must be last. Used for array size and to mean "no system-defined identifier".
} TIdentifierSubtype;


typedef enum {
	EHostParamImmediateValue = 0,	// Just a value.
	EHostParamIdentifier,			// An identifier that gets passed as a string.
	EHostParamLabeledValue,			// A value preceded by an identifier labeling it.
	EHostParam_Sentinel				// If this value is specified, this is the last parameter.
} THostParameterType;


typedef enum
{
	EHostParameterOptional = 1,
	EHostParameterRequired = 0
} THostParameterOptional;


#define LEO_MAX_HOST_PARAMS		8


// *** An entry in our global property look-up table:
struct TGlobalPropertyEntry
{
	TIdentifierSubtype		mType;					// The identifier for this property.
	LEOInstructionID		mSetterInstructionID;	// Instruction for changing this property.
	LEOInstructionID		mGetterInstructionID;	// Instruction for retrieving this property's value.
};


// *** An entry for a parameter to a command in our host command look-up table:
struct THostParameterEntry
{
	THostParameterType		mType;				// One of the flags above.
	TIdentifierSubtype		mIdentifierType;	// The identifier (for the label if EHostParamLabeled, ignored if EHostParamImmediateValue).
	THostParameterOptional	mIsOptional;		// If not present, pass an empty string.
	LEOInstructionID		mInstructionID;		// If not INVALID_INSTR2, this instruction overrides the one in the command entry if this parameter is present. If mType is EHostParamIdentifier, no string will be passed as a parameter either.
	uint16_t				mInstructionParam1;	// If mInstructionID is not INVALID_INSTR2, these parameters will be assigned to the instruction.
	uint32_t				mInstructionParam2;	// If mInstructionID is not INVALID_INSTR2, these parameters will be assigned to the instruction.
};


// *** An entry in our host command look-up table:
struct THostCommandEntry
{
	TIdentifierSubtype			mType;							// The identifier that introduces this command.
	LEOInstructionID			mInstructionID;					// The instruction to execute after pushing this command's params & param count on the stack.
	uint16_t					mInstructionParam1;				// These parameters will be assigned to the instruction.
	uint32_t					mInstructionParam2;				// These parameters will be assigned to the instruction.
	struct THostParameterEntry	mParam[LEO_MAX_HOST_PARAMS +1];	// These are the parameters that get pushed on the stack. Indicate the last param by setting the type of the one following it to EHostParam_Sentinel.
};
	

#endif /*FORGE_TYPES_H*/
