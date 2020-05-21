#include "catch_with_main.hpp"
#include "LEOValue.hpp"
#include "LEOInterpreter.h"
#include "LEOContextGroup.h"

TEST_CASE( "Create an Integer Variant", "[variants] [integer-variants]" ) {
	LEOContextGroup* group = LEOContextGroupCreate(NULL, NULL);
	LEOContext * context = LEOContextCreate(group, NULL, NULL);
	LEOContextGroupRelease(group);

    LEOValue testValue(42LL, kLEOUnitNone);
    
	REQUIRE( testValue->GetAsInteger(NULL, context) );
	
	LEOContextRelease(context);
}
