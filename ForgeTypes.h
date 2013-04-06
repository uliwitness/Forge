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
	Adding an identifier here defines the identifier type enum for it, as well
	as the matching string table entry (which must be all-lowercase so our fake
	case-insensitivity will work). Also, if you use the X2 macro instead of
	X1, you can also declare a token as synonymous to another token. 
*/
#define X1(constName,constStr)	X2(constName,constName,constStr)
#define IDENTIFIERS		\
	X1(EFunctionIdentifier,"function") \
	X1(EEndIdentifier,"end") \
	X1(EPlusOperator,"+") \
	X1(EMinusOperator,"-") \
	X1(EMultiplyOperator,"*") \
	X1(EDivideOperator,"/") \
	X1(ENewlineOperator,"\n")	/* A line break in the file. */ \
	X1(ELessThanOperator,"<") \
	X1(EGreaterThanOperator,">") \
	X1(EOnIdentifier,"on") \
	X1(ECommaOperator,",") \
	X1(EOpenBracketOperator,"(") \
	X1(ECloseBracketOperator,")") \
	X1(EPutIdentifier,"put") \
	X1(EIntoIdentifier,"into") \
	X1(EAfterIdentifier,"after") \
	X1(EBeforeIdentifier,"before") \
	X1(EReturnIdentifier,"return") \
	X1(ETheIdentifier,"the") \
	X1(EResultIdentifier,"result") \
	X1(ERepeatIdentifier,"repeat") \
	X1(EWhileIdentifier,"while") \
	X1(EForIdentifier,"for") \
	X1(ETimesIdentifier,"times") \
	X1(EWithIdentifier,"with") \
	X1(EToIdentifier,"to") \
	X1(EEqualsOperator,"=") \
	X1(EAmpersandOperator,"&") \
	X1(EIfIdentifier,"if") \
	X1(EThenIdentifier,"then") \
	X1(EElseIdentifier,"else") \
	X1(EFromIdentifier,"from") \
	X1(ETrueIdentifier,"true") \
	X1(EFalseIdentifier,"false") \
	X1(EEmptyIdentifier,"empty") \
	X1(EOpenSquareBracketOperator,"[") \
	X1(ECloseSquareBracketOperator,"]") \
	X1(EColonOperator,":") \
	X1(EPeriodOperator,".") \
	X1(EQuoteIdentifier,"quote") \
	X1(EDownIdentifier,"down") \
	X1(EUntilIdentifier,"until") \
	X1(EItemIdentifier,"item") \
	X1(EOfIdentifier,"of") \
	X1(ENewlineIdentifier,"newline") /* The word "newline". */ \
	X1(EAddIdentifier,"add") \
	X1(ESubtractIdentifier,"subtract") \
	X1(EMultiplyIdentifier,"multiply") \
	X1(EDivideIdentifier,"divide") \
	X1(EByIdentifier,"by") \
	X1(ECharacterIdentifier,"character") \
	X2(ECharIdentifier,ECharacterIdentifier,"char") \
	X1(EWordIdentifier,"word") \
	X1(ELineIdentifier,"line") \
	X1(EEntryIdentifier,"entry") \
	X2(EParamIdentifier,EParameterIdentifier,"param") \
	X1(EParameterIdentifier,"parameter") \
	X1(EParametersIdentifier,"parameters") \
	X1(EParamCountIdentifier,"paramcount") \
	X1(EUnsetIdentifier,"unset") \
	X1(EIsIdentifier,"is") \
	X1(ENotIdentifier,"not") \
	X1(EExponentOperator,"^") \
	X2(EModIdentifier,EModuloIdentifier,"mod") \
	X1(EModuloIdentifier,"modulo") \
	X1(EDeleteIdentifier,"delete") \
	X2(EItemDelIdentifier,EItemDelimiterIdentifier,"itemdel") \
	X2(EItemDelimIdentifier,EItemDelimiterIdentifier,"itemdelim") \
	X2(EItemDelimiterIdentifier,EItemDelimiterIdentifier,"itemdelimiter") \
	X1(ENumberIdentifier,"number") \
	X1(ECharactersIdentifier,"characters") \
	X2(ECharsIdentifier,ECharactersIdentifier,"chars") \
	X1(EWordsIdentifier,"words") \
	X1(ELinesIdentifier,"lines") \
	X1(EItemsIdentifier,"items") \
	X1(EAtSignOperator,"@") \
	X1(EAndIdentifier,"and") \
	X1(EOrIdentifier,"or") \
	X1(EExitIdentifier,"exit") \
	X1(ENextIdentifier,"next") \
	X1(EEachIdentifier,"each") \
	X1(ECommaIdentifier,"comma") \
	X1(EColonIdentifier,"colon") \
	X1(ECrIdentifier,"cr") \
	X1(ELineFeedIdentifier,"linefeed") \
	X1(ENullIdentifier,"null") \
	X1(ESpaceIdentifier,"space") \
	X1(ETabIdentifier,"tab") \
	X1(EPiIdentifier,"pi") \
	X1(EGetIdentifier,"get") \
	X1(EShortIdentifier,"short") \
	X2(EAbbrIdentifier,EAbbreviatedIdentifier,"abbr") \
	X2(EAbbrevIdentifier,EAbbreviatedIdentifier,"abbrev") \
	X2(EAbbreviatedIdentifier,EAbbreviatedIdentifier,"abbreviated") \
	X1(ELongIdentifier,"long") \
	X2(ENumIdentifier,ENumberIdentifier,"num") \
	X1(ESetIdentifier,"set") \
	X1(ENullPointerIdentifier,"nullpointer") \
	X1(EIdIdentifier,"id") \
	X1(EHandlerIdentifier,"handler") \
	X1(EMessageIdentifier,"message") \
	X1(EGlobalIdentifier,"global") \
	X1(EPrivateIdentifier,"private") \
	X1(EPublicIdentifier,"public") \
	X1(EDoubleAmpersandPseudoOperator,"&&") \
	X1(ENotEqualPseudoOperator,"is not") \
	X1(ELessThanEqualPseudoOperator,"<=") \
	X1(EGreaterThanEqualPseudoOperator,">=") \
	X2(EThruIdentifier,EThroughIdentifier,"thru") \
	X1(EThroughIdentifier,"through") \
	X1(EInIdentifier,"in") \
	X1(EByteIdentifier,"byte") \
	X1(EBytesIdentifier,"bytes") \
	X1(EGoIdentifier,"go") \
	X1(ECursorIdentifier,"cursor") \
	X1(EStackIdentifier,"stack") \
	X1(EBackgroundIdentifier,"background") \
	X2(EBkgndIdentifier,EBackgroundIdentifier,"bkgnd") \
	X2(EBkgdIdentifier,EBackgroundIdentifier,"bkgd") \
	X2(EBgIdentifier,EBackgroundIdentifier,"bg") \
	X1(ECardIdentifier,"card") \
	X2(ECdIdentifier,ECardIdentifier,"cd") \
	X1(EFieldIdentifier,"field") \
	X2(EFldIdentifier,EFieldIdentifier,"fld") \
	X1(EButtonIdentifier,"button") \
	X2(EBtnIdentifier,EButtonIdentifier,"btn") \
	X1(EPartIdentifier,"part") \
	X1(EPreviousIdentifier,"previous") \
	X2(EPrevIdentifier,EPreviousIdentifier,"prev") \
	X1(EFirstIdentifier,"first") \
	X1(ELastIdentifier,"last") \
	X1(EVisualIdentifier,"visual") \
	X1(EEffectIdentifier,"effect") \
	X1(EMeIdentifier,"me") \
	X1(EThisIdentifier,"this") \
	X1(EIrisIdentifier,"iris") \
	X1(EZoomIdentifier,"zoom") \
	X1(EBarnIdentifier,"barn") \
	X1(EDoorIdentifier,"door") \
	X1(EWipeIdentifier,"wipe") \
	X1(EPushIdentifier,"push") \
	X1(EScrollIdentifier,"scroll") \
	X1(EShrinkIdentifier,"shrink") \
	X1(EStretchIdentifier,"stretch") \
	X1(EVenetianIdentifier,"venetian") \
	X1(EBlindsIdentifier,"blinds") \
	X1(ELeftIdentifier,"left") \
	X1(ERightIdentifier,"right") \
	X1(EUpIdentifier,"up") \
	X1(EOpenIdentifier,"open") \
	X1(ECloseIdentifier,"close") \
	X1(EOutIdentifier,"out") \
	X1(ETopIdentifier,"top") \
	X1(EBottomIdentifier,"bottom") \
	X1(ECenterIdentifier,"center") \
	X2(ECentreIdentifier,ECenterIdentifier,"centre") \
	X1(EAnswerIdentifier,"answer") \
	X1(EAskIdentifier,"ask") \
	X1(EPassIdentifier,"pass") \
	X1(EMovieIdentifier,"movie") \
	X1(EPlayerIdentifier,"player") \
	X1(EMyIdentifier,"my") \
	X1(EVersionIdentifier,"version") \
	X1(ECreateIdentifier,"create") \
	X1(ENewIdentifier,"new") \
	X1(EDebugIdentifier,"debug") \
	X1(ECheckpointIdentifier,"checkpoint") \
	X1(EDownloadIdentifier,"download") \
	X1(EDownloadsIdentifier,"downloads") \
	X1(EWhenIdentifier,"when") \
	X1(EDoneIdentifier,"done") \
	X1(EChunkIdentifier,"chunk")


typedef enum
{
#define X2(constName,constSynonym,constStr)	constName,
	IDENTIFIERS
#undef X2
	
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
	TIdentifierSubtype		mPrefixType;			// One of ELongIdentifier, EShortIdentifier or EAbbreviatedIdentifier for two-word properties. Otherwise, ELastIdentifier_Sentinel.
	LEOInstructionID		mSetterInstructionID;	// Instruction for changing this property.
	LEOInstructionID		mGetterInstructionID;	// Instruction for retrieving this property's value.
};


// *** An entry in our global property look-up table:
struct TBuiltInFunctionEntry
{
	TIdentifierSubtype		mType;			// The identifier for this property.
	LEOInstructionID		mInstructionID;	// Instruction for this function.
	uint16_t				mParam1;		// Parameter to set on the instruction.
	uint32_t				mParam2;		// Parameter to set on the instruction.
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
