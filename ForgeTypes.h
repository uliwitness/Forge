//
//  ForgeTypes.h
//  Forge
//
//  Created by Uli Kusterer on 16.04.11.
//  Copyright 2011 Uli Kusterer. All rights reserved.
//

/*!
	@header ForgeTypes
	
	This header defines the C data types a host application needs to register its
	own syntax with the Forge parser, and to use the Forge parser to generate
	Leonie bytecode.
*/


#ifndef FORGE_TYPES_H
#define FORGE_TYPES_H		1

#include "LEOInterpreter.h"


/*! The various built-in identifiers the parser recognizes:
	Adding an identifier here defines the identifier type enum for it, as well
	as the matching string table entry (which must be all-lowercase so our fake
	case-insensitivity will work). Also, if you use the X2 macro instead of
	X1, you can also declare a token as synonymous to another token, e.g. to
	register a short form that is always remapped to the longer form before
	the parser sees it.
	
	If you want to use any identifiers for your host-specific commands that aren't
	in here, you must currently edit this file to add them. In the future, you
	will hopefully be able to just define your own identifiers elsewhere in addition
	to the ones here that are needed for built-in commands.
*/
#define X1(constName,constStr)	X2(constName,constName,constStr)
#define IDENTIFIERS		\
	X1(EFunctionIdentifier,"function") \
	X1(EEndIdentifier,"end") \
	X1(EPlusOperator,"+") \
	X1(EMinusOperator,"-") \
	X1(EMultiplyOperator,"*") \
	X1(EDivideOperator,"/") \
	X1(EDivideSymbolOperator,"÷") \
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
	X1(ENotEqualOperator,"≠") \
	X1(ELessThanEqualOperator,"≤") \
	X1(EGreaterThanEqualOperator,"≥") \
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
	X2(EPlyIdentifier,EPlayerIdentifier,"ply") \
	X1(EMyIdentifier,"my") \
	X1(EVersionIdentifier,"version") \
	X1(EMachineIdentifier,"machine") \
	X1(ECreateIdentifier,"create") \
	X1(ENewIdentifier,"new") \
	X1(EDebugIdentifier,"debug") \
	X1(ECheckpointIdentifier,"checkpoint") \
	X1(EDownloadIdentifier,"download") \
	X1(EDownloadsIdentifier,"downloads") \
	X1(EWhenIdentifier,"when") \
	X1(EDoneIdentifier,"done") \
	X1(EChunkIdentifier,"chunk") \
	X1(EPlatformIdentifier,"platform") \
	X1(ESystemVersionIdentifier,"systemversion") \
	X1(EPhysicalMemoryIdentifier,"physicalmemory") \
	X1(EButtonsIdentifier,"buttons") \
	X2(EBtnsIdentifier,EButtonsIdentifier,"btns") \
	X1(EFieldsIdentifier,"fields") \
	X2(EFldsIdentifier,EFieldsIdentifier,"flds") \
	X1(EPartsIdentifier,"parts") \
	X1(EPlayersIdentifier,"players") \
	X2(EPlysIdentifier,EPlayersIdentifier,"plys") \
	X1(ECardsIdentifier,"cards") \
	X1(EBackgroundsIdentifier,"backgrounds") \
	X2(ECdsIdentifier,ECardsIdentifier,"cds") \
	X2(EBgsIdentifier,EBackgroundsIdentifier,"bgs") \
	X2(EBkgdsIdentifier,EBackgroundsIdentifier,"bkgds") \
	X2(EBkgndsIdentifier,EBackgroundsIdentifier,"bkgnds") \
	X1(EStacksIdentifier,"stacks") \
	X1(EPropertyIdentifier,"property") \
	X2(EPropIdentifier,EPropertyIdentifier,"prop") \
	X1(ETargetIdentifier,"target") \
	X1(EPlayIdentifier,"play") \
	X1(ETimerIdentifier,"timer") \
	X1(ETimersIdentifier,"timers") \
	X1(EGraphicIdentifier,"graphic") \
	X2(EGrcIdentifier,EGraphicIdentifier,"grc") \
	X1(EGraphicsIdentifier,"graphics") \
	X2(EGrcsIdentifier,EGraphicsIdentifier,"grcs") \
	X1(EStartIdentifier,"start") \
	X1(EStopIdentifier,"stop") \
	X1(EBoxIdentifier,"box") \
	X2(EMsgIdentifier,EMessageIdentifier,"msg") \
	X1(EShowIdentifier,"show") \
	X1(EHideIdentifier,"hide") \
	X1(EWaitIdentifier,"wait") \
	X1(ETickIdentifier,"tick") \
	X1(ETicksIdentifier,"ticks") \
	X1(ESecondIdentifier,"second") \
	X1(ESecondsIdentifier,"seconds") \
	X1(EMinuteIdentifier,"minute") \
	X1(EMinutesIdentifier,"minutes") \
	X1(EHourIdentifier,"hour") \
	X1(EHoursIdentifier,"hours") \
	X1(EDayIdentifier,"day") \
	X1(EDaysIdentifier,"days") \
	X1(EWeekIdentifier,"week") \
	X1(EWeeksIdentifier,"weeks") \
	X1(EKilobyteIdentifier,"kilobyte") \
	X1(EKilobytesIdentifier,"kilobytes") \
	X1(EMegabyteIdentifier,"megabyte") \
	X1(EMegabytesIdentifier,"megabytes") \
	X1(EGigabyteIdentifier,"gigabyte") \
	X1(EGigabytesIdentifier,"gigabytes") \
	X1(ETerabyteIdentifier,"terabyte") \
	X1(ETerabytesIdentifier,"terabytes") \
	X1(ESoundIdentifier,"sound") \
	X1(EEditBackgroundIdentifier,"editbackground") \
	X2(EEditBkgndIdentifier,EEditBackgroundIdentifier,"editbkgnd") \
	X2(EEditBkgdIdentifier,EEditBackgroundIdentifier,"editbkgd") \
	X2(EEditBgIdentifier,EEditBackgroundIdentifier,"editbg") \
	X1(EWatcherIdentifier,"watcher") \
	X1(EChooseIdentifier,"choose") \
	X1(EToolIdentifier,"tool") \
	X1(EWindowIdentifier,"window") \
	X2(EWdIdentifier,EWindowIdentifier,"wd") \
	X2(EWindIdentifier,EWindowIdentifier,"wind") \
	X1(EBrowserIdentifier,"browser") \
	X1(EBrowsersIdentifier,"browsers") \
	X1(EPopupIdentifier,"popup") \
	X1(ENoteIdentifier,"note") \
	X1(ENotesIdentifier,"notes") \
	X1(ERowIdentifier,"row") \
	X1(EColumnIdentifier,"column") \
	X1(ERowsIdentifier,"rows") \
	X1(EColumnsIdentifier,"columns") \
	X2(EColIdentifier,EColumnIdentifier,"col") \
	X2(EColsIdentifier,EColumnsIdentifier,"cols") \
	X1(EBackIdentifier,"back") \
	X1(EMoveIdentifier,"move") \
	X1(EAlongIdentifier,"along") \
	X1(EScreensIdentifier,"screens") \
	X1(EScreenIdentifier,"screen") \
	X1(EIIdentifier,"i") \
	X1(EHaveIdentifier,"have") \
	X1(EHasIdentifier,"has") \
	X1(EAmongIdentifier,"among") \
	X1(EIsAmongOperator,"is among") \
	X1(EHasPropertyOperator,"has property") \
	X1(EWorkingIdentifier,"working") \
	X1(EEffectiveIdentifier,"effective")


typedef enum
{
#define X2(constName,constSynonym,constStr)	constName,
	IDENTIFIERS
#undef X2
	
	ELastIdentifier_Sentinel	// Must be last. Used for array size and to mean "no system-defined identifier".
} TIdentifierSubtype;


/*! The THostParameterType enum is used by THostCommandEntry to indicate which
	part of a command is to be parsed in what way. */
typedef enum {
	EHostParamImmediateValue = 0,	//! Just a value.
	EHostParamExpression,			//! An entire expression, but no label.
	EHostParamContainer,			//! A container that something can be put into.
	EHostParamIdentifier,			//! An identifier that gets passed as a string.
	EHostParamInvisibleIdentifier,	//! An identifier that is simply used to switch modes, but doesn't cause a parameter.
	EHostParamLabeledValue,			//! A value preceded by an identifier labeling it.
	EHostParamLabeledExpression,	//! An expression preceded by an identifier labeling it.
	EHostParamLabeledContainer,		//! A container that something can be put into, preceded by an identifier labeling it.
	EHostParamExpressionOrIdentifiersTillLineEnd,	//! Either an expression, or a bunch of unquoted string literals all merged into one string parameter with a single space separating each from the next. Used e.g. for 'play' command's melody.
	EHostParam_Sentinel				//! If this value is specified, this is the last parameter.
} THostParameterType;


/*! The THostParameterOptional enum is used by THostCommandEntry to indicate whether
	a part of a command is required or optional. */
typedef enum
{
	EHostParameterOptional = 1,
	EHostParameterRequired = 0
} THostParameterOptional;


#define LEO_MAX_HOST_PARAMS		15


//! An entry in our operator look-up table.
struct TOperatorEntry
{
	TIdentifierSubtype		mType;				//!< The identifier for this operator.
	TIdentifierSubtype		mSecondType;		//!< The second identifier if this operator consists of two tokens.
	int						mPrecedence;		//!< Precedence, with higher number taking precedence over lower numbers (i.e. * > +).
	LEOInstructionID		mInstructionID;		//!< Name of function to call for this operator.
	TIdentifierSubtype		mTypeToReturn;		//!< The identifier to return for this operator.
};


//! An entry in our unary operator look-up table.
struct TUnaryOperatorEntry
{
	TIdentifierSubtype		mType;				//!< The identifier for this operator.
	LEOInstructionID		mInstructionID;		//!< Instruction that implements this operator.
};


//! An entry in our global property look-up table:
struct TGlobalPropertyEntry
{
	TIdentifierSubtype		mType;					//! The identifier for this property (i.e. its name).
	TIdentifierSubtype		mPrefixType;			//! One of ELongIdentifier, EShortIdentifier, EAbbreviatedIdentifier, EWorkingIdentifier or EEffectiveIdentifier for two-word properties. Otherwise, ELastIdentifier_Sentinel.
	LEOInstructionID		mSetterInstructionID;	//! Instruction for changing this property.
	LEOInstructionID		mGetterInstructionID;	//! Instruction for retrieving this property's value.
};


//! An entry in our global property look-up table:
struct TBuiltInFunctionEntry
{
	TIdentifierSubtype		mType;			//! The identifier for this function (i.e. its name).
	LEOInstructionID		mInstructionID;	//! Instruction for this function.
	uint16_t				mParam1;		//! Parameter to set on the instruction.
	uint32_t				mParam2;		//! Parameter to set on the instruction.
};


/*! An entry for a parameter to a THostCommandEntry in our host command look-up table: */
struct THostParameterEntry
{
	THostParameterType		mType;				//! The type of parameter to parse, or EHostParam_Sentinel if this is the end of the list of host command entries.
	TIdentifierSubtype		mIdentifierType;	//! The identifier (for the label or identifier, depending on mType).
	THostParameterOptional	mIsOptional;		//! Is this parameter required or optional? If not present and optional, we, pass an empty string unless mType indicates otherwise, or mInstructionID is not INVALID_INSTR2.
	LEOInstructionID		mInstructionID;		//! If not INVALID_INSTR2, this instruction overrides the one in the command entry if this parameter is present. If mType is EHostParamIdentifier, no string will be passed as a parameter either.
	uint16_t				mInstructionParam1;	//! If mInstructionID is not INVALID_INSTR2, these parameters will be assigned to the instruction if this parameter is parsed.
	uint32_t				mInstructionParam2;	//! If mInstructionID is not INVALID_INSTR2, these parameters will be assigned to the instruction if this parameter is parsed.
	char					mModeRequired;		//! If this isn't 0, only parse this if the current mode is this number. The mode can be used to group together certain parameters so they only match when a previous parameter matched.
	char					mModeToSet;			//! If this parameter matches, and this isn't 0, change the current mode to this. The mode can be used to only look for certain parameters when a previous parameter matched.
};


//! An entry in our host command look-up table:
struct THostCommandEntry
{
	TIdentifierSubtype			mType;							//! The identifier that introduces this command.
	LEOInstructionID			mInstructionID;					//! The instruction to execute after pushing this command's params & param count on the stack.
	uint16_t					mInstructionParam1;				//! These parameters will be assigned to the instruction.
	uint32_t					mInstructionParam2;				//! These parameters will be assigned to the instruction.
	char						mTerminalMode;					//! If not 0, this is the mModeToSet that must have been set for this by one of the parameters to be considered a successful match.
	struct THostParameterEntry	mParam[LEO_MAX_HOST_PARAMS +1];	//! These are the parameters that get pushed on the stack. Indicate the last param by setting the type of the one following it to EHostParam_Sentinel.
};



typedef void	(*LEOFirstNativeCallCallbackPtr)( void );


#endif /*FORGE_TYPES_H*/
