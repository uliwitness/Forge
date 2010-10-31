/*
 *  vcy_lib.c
 *  HyperCompiler
 *
 *  Created by Uli Kusterer on 14.12.08.
 *  Copyright 2008 The Void Software. All rights reserved.
 *
 */

#include <vector>

typedef std::vector<CVariant*>	CVariantList;


class CVariant
{
public:
	explicit CVariant( const char* inStr ) : mNumber(0), mList(NULL)				{ mString = malloc( strlen(inStr) ); strcpy( mString, inStr ); };
	explicit CVariant( double inNum ) : mNumber(inNum), mString(NULL), mList(NULL)	{};
	~CVariant()	{ Clear(); };
	
	void			Clear()		{ if( mString ) free(mString); mString = NULL; if( mList ) delete mList; mList = NULL; };
	
	double			mNumber;
	char*			mString;
	CVariantList*	mList;
};

void	vcy_make_list( CVariant& theVariant )
{
	theVariant.Clear();
	theVariant.mList = new CVariantList;
}