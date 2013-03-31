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


/* The various built-in identifiers the parser recognizes:
	When adding an identifier here, remember to always also add an entry for it
	to gIdentifierStrings and gIdentifierSynonyms.
*/
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
	EParametersIdentifier,
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
	EMeIdentifier,
	EThisIdentifier,
	EIrisIdentifier,
	EZoomIdentifier,
	EBarnIdentifier,
	EDoorIdentifier,
	EWipeIdentifier,
	EPushIdentifier,
	EScrollIdentifier,
	EShrinkIdentifier,
	EStretchIdentifier,
	EVenetianIdentifier,
	EBlindsIdentifier,
	ELeftIdentifier,
	ERightIdentifier,
	EUpIdentifier,
	EOpenIdentifier,
	ECloseIdentifier,
	EOutIdentifier,
	ETopIdentifier,
	EBottomIdentifier,
	ECenterIdentifier,
	EAnswerIdentifier,
	EAskIdentifier,
	EPassIdentifier,
	EMovieIdentifier,
	EPlayerIdentifier,
	EMyIdentifier,
	EVersionIdentifier,
	ECreateIdentifier,
	ENewIdentifier,
	EDebugIdentifier,
	ECheckpointIdentifier,
	EDownloadIdentifier,
	EWhenIdentifier,
	EDoneIdentifier,
	EChunkIdentifier,
	
	ELastIdentifier_Sentinel	// Must be last. Used for array size and to mean "no system-defined identifier".
} TIdentifierSubtype;


typedef enum {
	EHostParamImmediateValue = 0,	// Just a value.
	EHostParamExpression,			// An entire expression, but no label.
	EHostParamIdentifier,			// An identifier that gets passed as a string.
	EHostParamLabeledValue,			// A value preceded by an identifier labeling it.
	EHostParamLabeledExpression,	// An expression preceded by an identifier labeling it.
	EHostParam_Sentinel				// If this value is specified, this is the last parameter.
} THostParameterType;


typedef enum
{
	EHostParameterOptional = 1,
	EHostParameterRequired = 0
} THostParameterOptional;


#define LEO_MAX_HOST_PARAMS		15


// *** An entry in our global property look-up table:
struct TGlobalPropertyEntry
{
	TIdentifierSubtype		mType;					// The identifier for this property.
	LEOInstructionID		mSetterInstructionID;	// Instruction for changing this property.
	LEOInstructionID		mGetterInstructionID;	// Instruction for retrieving this property's value.
};


// *** An entry in our global property look-up table:
struct TBuiltInFunctionEntry
{
	TIdentifierSubtype		mType;			// The identifier for this property.
	LEOInstructionID		mInstructionID;	// Instruction for this function.
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
	char					mModeRequired;		// If this isn't 0, only parse this if the current mode is this number. The mode can be used to group together certain parameters so they only match when a previous parameter matched.
	char					mModeToSet;			// If this parameter matches, and this isn't 0, change the current mode to this.
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
