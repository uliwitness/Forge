#!/usr/bin/php
<?php

/*
	headerimport:
	This PHP script uses some regular expressions and a hand-grown parser in one
	spot to extract information about all classes, their methods, their protocols
	and their categories from the headers of the frameworks whose names have
	been specified as command-line arguments (or a list of default frameworks if none
	were specified).
	
	It writes the output as easily machine-parseable pseudo-text to a file
	named "frameworkheaders.hhc" from where they can be transferred anywhere
	you want.
	
	The framework headers are passed through the GCC preprocessor to ensure
	only classes and objects appropriate for the current platform are extracted.
*/

// The following is a comment printed at the top of the output file:
$directions =
"# Generated by Forge's headerimport.php file based on the following\n".
"# framework headers:\n";	// Continues below.

// -----------------------------------------------------------------------------
//	Determine list of frameworks to extract data from:
//

$frameworks = array_slice( $_SERVER['argv'], 1 );	// Remove name of executable.
if( sizeof($frameworks) == 0 )	// No other params? Fallback!
{
	$frameworks = array( "CoreFoundation", "ApplicationServices", "Carbon", "Foundation", "AppKit", "WebKit", "AddressBook", "QTKit", "ScreenSaver", "AVFoundation", "QuartzCore", "AVKit", "EventKit" );
	echo "note: No frameworks specified. Using defaults.\n";
}

$frameworkheaders = array();

// Include some standard runtime files:
$codestr .= "#pragma HEADERIMPORT FRAMEWORK: \"\"\n";
$codestr .= "#import <objc/NSObjCRuntime.h>\n";
$codestr .= "#import <stdio.h>\n";
$codestr .= "#import <stddef.h>\n";
$codestr .= "#import <string.h>\n";
$codestr .= "#import <stdint.h>\n";

// Actually generate a minimal application that imports all the frameworks:
echo "note: Preparing framework".(sizeof($frameworks) > 1 ? "s" : "").": ";

for( $x = 0; $x < sizeof($frameworks); $x++ )
{
	$fmwk = $frameworks[$x];
	echo ($x > 0 ? ", " : "").$fmwk;
	$slashpos = strpos($fmwk,"/");
	if( $slashpos !== false )
	{
		$codestr .= "#pragma HEADERIMPORT FRAMEWORK: \"".substr($fmwk, 0, $slashpos)."\"\n";
		$codestr .= "#import <".$fmwk.">\n";
		array_push( $frameworkheaders, $fmwk );
	}
	else
	{
		$codestr .= "#pragma HEADERIMPORT FRAMEWORK: \"".$fmwk."\"\n";
		$codestr .= "#import <".$fmwk."/".$fmwk.".h>\n";
		array_push( $frameworkheaders, $fmwk."/".$fmwk.".h" );
	}
	$directions .= "#\t$fmwk\n";	// Add framework names to directions.
}
$codestr .= "void main()\n{\n}";
echo "\n";

// Write rest of directions:
$directions .= "# \n".
"# The format of this file is easy: The first char of each line indicates what\n".
"# is on that line. An 'F' marks which framework the subsequent items come from,\n".
"# an 'H' what header was included to parse these items. An asterisk (*) indicates\n".
"# this is the name of the class the following lines and categories belong to.\n".
"# A colon (:) indicates the superclass of the current class. An opening bracket\n".
"# indicates that the following methods are from a category with that particular name.\n".
"# A less-than sign (<) indicates a protocol that the current class/category implements.\n".
"# There may be several protocols, each specified on its own line.\n# \n".
"# Lines beginning with a minus sign (-) specify instance methods for the\n".
"# current class, those with a plus (+) specify class methods. Method lines\n".
"# contain a comma-separated list of first the method name, followed by the\n".
"# return type and the types of any parameters.\n".
"# \n".
"# Function lines are like method lines and start with an equals sign (=).\n".
"# \n".
"# Typedefs start with a tilde character (~) and contain the name of the new\n".
"# type, then a comma and the name of the type it is an alternate name for.\n".
"# An ampersand (&) indicates a typedef for a procedure pointer, with the\n".
"# return type and parameter types following as a comma-separated list.\n".
"# \n# \n".
"# If you want to parse this file yourself, please skip any lines that start\n".
"# with a character that is none of the above or that are empty to allow for\n".
"# future expansion.\n# \n".
"# Created on ".date("Y-m-d H:i")."\n\n";

// -----------------------------------------------------------------------------
//	Get preprocessed framework headers concatenated in one file:
//

$srcfilename = tempnam(sys_get_temp_dir(), "hyperc_headerimport.mm");
$headerdatafilename = tempnam(sys_get_temp_dir(), "preparedheaders.h");
$fd = fopen( $srcfilename, "w" );
fwrite( $fd, $codestr );
fclose( $fd );
system( "gcc -x objective-c++ -E \"$srcfilename\" > \"$headerdatafilename\"" );
unlink( $srcfilename );

$headerstr = file_get_contents( $headerdatafilename );	// Read combined header from temp file.
unlink( $headerdatafilename );	// Get rid of temp file.

$output = "";

// -----------------------------------------------------------------------------
//	Split it up into several headers:
//

// Delete junk up to first included framework's name.
$pattern = "#pragma HEADERIMPORT FRAMEWORK: \"";
$nextframework = strpos( $headerstr, $pattern );
if( $nextframework === false )
{
	echo "\nError: Couldn't find framework boundaries.\n";
	return;
}
$startpos = $nextframework + strlen($pattern);
$len = strlen($headerstr) -$nextframework -strlen($pattern);
$headerstr = substr( $headerstr, $startpos, $len );

//echo "START: $startpos LENGTH: $len\n";
$fmwkNum = 0;
$knowntypes = array();	// Array of type name -> array( prefix,suffix ) mappings for registering typedefs with the same info as the original.

while( strlen($headerstr) > 0 )
{
	$fmwknameend = strpos( $headerstr, "\"" );
	$currfmwkname = substr( $headerstr, 0, $fmwknameend );
	
	$nextframework = strpos( $headerstr, $pattern );
	if( $nextframework === false )
		$nextframework = strlen($headerstr);
	$currheaderstr = substr( $headerstr, $fmwknameend +1, $nextframework -$fmwknameend -1 );
	$currheaderstr = removeCommentsAndPreprocessorJunk($currheaderstr);
	$output .= "F".$currfmwkname."\nH".$frameworkheaders[$fmwkNum++]."\n";
	if( $currfmwkname == "Carbon" )
	{
		//$output .= "~UInt32,unsigned long\n~SInt32,long\n~UInt16,unsigned short\n~SInt16,short\n~UInt8,unsigned charn~SInt8,char\n";
	}
	$output .= parseTypedefsFromString( $currheaderstr, $currfmwkname );
	$output .= parseEnumTypedefsFromString( $currheaderstr, $currfmwkname );
	$output .= parseProcPtrsFromString( $currheaderstr, $currfmwkname );
	$output .= parseFunctionsFromString( $currheaderstr, $currfmwkname );
	$output .= parseHeadersFromString( $currheaderstr, $currfmwkname );
	//echo "#pragma mark $currfmwkname\n$currheaderstr\n";
	$startpos = $nextframework + strlen($pattern);
	$len = strlen($headerstr) -$nextframework -strlen($pattern);
	//echo "START: $startpos LENGTH: $len\n";
	$headerstr = substr( $headerstr, $startpos, $len );
}


function removeCommentsAndPreprocessorJunk( $str )
{
	$outstr = "";
	$str = str_replace("\r","\n",$str);
	$lines = explode("\n",$str);
	echo sizeof($lines)."\n";
	for( $x = 0; $x < sizeof($lines); $x++ )
	{
		if( $lines[$x][0] == '#' && ($lines[$x][1] == ' ' || $lines[$x][1] == '\t') )
			$outstr .= "\n";
		else if( $lines[$x][0] == '/' && $lines[$x][1] == '/' )
			$outstr .= "\n";
		else
			$outstr .= $lines[$x]."\n";
	}
	
	return $outstr;
}


// -----------------------------------------------------------------------------
//	Now that we have all headers, we can use this function to find all classes
//	in them:
//

function parseHeadersFromString( $headerstr, $currfmwkname )
{
	$output = "";
	$treffer = array();

	// Build our honkin' huge regexp that finds classes and categories and returns
	//	their methods and ivars as one string:
	$re_classname = "([A-Za-z0-9_]*)";
	$re_whitespace = "([ \t\r\n]*)";
	$re_catname = "(\([ ]*([A-Za-z0-9_]*)[ ]*\))";
	$re_colonandsuper = "([:]$re_whitespace([A-Za-z0-9_]+))";
	$re_proto = "([A-Za-z0-9_]*)";
	$re_additionalproto = "($re_whitespace,$re_whitespace$re_proto)";
	$re_protocols = "(<$re_whitespace$re_proto$re_additionalproto*$re_whitespace>$re_whitespace){0,1}";
	$re_super = "($re_colonandsuper$re_whitespace$re_protocols)";
	$re_cat = "($re_catname$re_whitespace$re_protocols)";
	$re_superorcatname = "($re_super|$re_cat|$re_protocols)";
	$re_allchars_nocurly = "!$&'%\\\\?|~\]\[+\-*\/\(\)#a-zA-Z0-9\n\r\t^ @;,_:<>=.\"";
	$re_allchars = "$re_allchars_nocurly{}";
	$re_methodsnstuff = "([$re_allchars]*?)";
	$fullregexp = "/@interface$re_whitespace$re_classname$re_whitespace$re_superorcatname$re_whitespace$re_methodsnstuff$re_whitespace@end/";

	// Warn if headers contain characters our regexp isn't prepared for:
	$matchcount = preg_match_all( "([^$re_allchars])", $headerstr, $treffer );
	if( $matchcount > 0 )
	{
		echo "warning: Invalid characters: \"";
		for( $x = 0; $x < sizeof($treffer[0]); $x++ )
			echo $treffer[0][$x];
		echo "\"\n";
	}

	// Actually run our regexp:
	$treffer = array();
	$matchcount = preg_match_all( $fullregexp, $headerstr, $treffer );

	// -----------------------------------------------------------------------------
	//	Now that we have all classes, extract the exact info from them:
	//
	
	if( $matchcount == 0 )
		return "";
	echo "note: ".$currfmwkname.": $matchcount classes found\n";
	for( $y = 0; $y < sizeof($treffer[0]); $y++ )
	{
		// "Outside" info:
		$output .= "*".$treffer[2][$y]."\n";	// Class name.
		if( trim( $treffer[20][$y] ) != "" )	// Is a category on that class?
		{
			$output .= "(".$treffer[21][$y]."\n";	// Remember category name.
			$protos = $treffer[23][$y];				// Remember what protocols it implements (this is in a different place in the results dictionary than for classes).
		}
		else	// Otherwise it's a class.
		{
			$output .= ":".$treffer[8][$y]."\n";	
			$protos = $treffer[34][$y];
		}
		$methodstr = $treffer[42][$y];
		if( trim( $protos ) != "" )
		{
			$protos = str_replace(" ","",$protos);
			$protos = str_replace("\t","",$protos);
			$protos = str_replace("\r","",$protos);
			$protos = str_replace("\n","",$protos);
			$protos = explode(",",trim($protos,"<>"));
			for( $x = 0; $x < sizeof($protos); $x++ )
				$output .= "<".$protos[$x]."\n";
		}
		
		$methodstr = preg_replace( "/(#|\/\/)([$re_allchars]*)[\n\r]/", "", $methodstr );	// Remove comments and leftover preprocessor stuff.
		$methodstr = preg_replace( "/(\/\*)([$re_allchars]*?)(\*\/)/", "", $methodstr );	// Remove multi-line comments.
		$methodstr = str_replace("\t", " ", $methodstr);
		$methodstr = str_replace("\r", "\n", $methodstr);
		$methodstr = str_replace("\n", " ", $methodstr);
		while( strpos( $methodstr, "  " ) !== false )
			$methodstr = str_replace("  ", " ", $methodstr);
		$methodstr = trim($methodstr);
		
		// Now extract the ivar list. We have to do this manually as there's no way
		//	to write a regexp that can handle nested brackets, which can occur if
		//	someone uses an anonymous enum, struct or union in the ivar section:
		if( $methodstr[0] == "{" )	// Have ivars! We can do this since we just trimmed and normalised the methodstr.
		{
			$ivars = "";
			$bracecount = 1;
			for( $x = 1; $bracecount > 0 && $x < strlen($methodstr); $x++ )	// Start at 1 since we already scanned the starting bracket.
			{
				if( $methodstr[$x] == "{" )
				{
					$ivars .= "{";
					$bracecount++;
				}
				else if( $methodstr[$x] == "}" && $bracecount > 1 )	// Add all but last, enclosing bracket.
				{
					$ivars .= "}";
					$bracecount--;
				}
				else if( $methodstr[$x] == "}" )					// Balance for last bracket so we can exit the loop.
					$bracecount--;
				else
					$ivars .= $methodstr[$x];
			}
			if( $x < (strlen($methodstr)-1) )	// Still data to go?
				$methods = substr($methodstr,$x+1,strlen($methodstr)-$x);
			else
				$methods = "";
		}
		else
		{
			$ivars = "";
			$methods = $methodstr;
		}
		
		// Now loop over the methods:
		$ivars = $ivars;
		$methods = explode(";",$methods);
		for( $x = 0; $x < sizeof($methods); $x++ )
		{
			$currline = trim($methods[$x]);
			if( $currline == "" )
				continue;
			else if( $currline[0] == '#' )
				continue;
			else if( $currline[0] == "-" )
				$output .= "-";
			else if( $currline[0] == "+" )
				$output .= "+";
			else if( $currline[0] == "@" )
			{
				//echo "$currline\n";
				$identifier = "[A-Za-z_][A-Za-z_0-9]*";
				$typename = "[A-Za-z_][A-Za-z_0-9]*[ \t\n\r]*(<([A-Za-z_][A-Za-z_0-9]*)>)?[ \t\n\r]*\**";
				$identorgetter = "([gs]etter=)?[A-Za-z_][A-Za-z_0-9]*";
				$onebehaviour = "[ \t\n\r]*($identorgetter)[ \t\n\r]*";
				$nextbehaviour = ",($onebehaviour)";
				$matchcount = preg_match("/\@property[ \t\n\r]*(\($onebehaviour($nextbehaviour)*\))?[ \t\n\r]*($typename)[ \t\n\r]*($identifier)[ \t\n\r]*(.*)/",$currline, $propparts);
				
				if( $matchcount > 0 )
				{
					//print_r($propparts);
					
					$attributes = $propparts[1];
					$type = trim(str_replace("\t"," ",$propparts[8]));
					$type = trim(str_replace(" *","*",$type));
					$type = trim(str_replace(" <","<",$type));
					$name = $propparts[11];
					
					$settername = "set".ucfirst($name).":";
					$gettername = $name;
					
					if( preg_match("/(getter)=($identifier)/",$attributes,$getterparts) > 0 )
					{
						$gettername = $getterparts[2];
						//echo $gettername."\n";
					}
					if( preg_match("/(setter)=($identifier)/",$attributes,$setterparts) > 0 )
					{
						$settername = $setterparts[2];
						//echo $settername."\n";
					}
					
					if( strpos($attributes, "readonly") === false )
						$output .= "-".$settername.",void,".$type."\n";
					$output .= "-".$gettername.",".$type."\n";
				}
				else
				{
					echo "warning: Couldn't match this property:\n";
					echo $currline."\n";
				}			
				continue;
			}
			else
			{
				$parts = explode(" ",$currline,2);
				if( $parts[0] != "typedef" && $parts[0] != "extern"
					&& $parts[0] != "enum" )
				{
					echo "warning: can't find method kind here:\n";
					echo "\t".$methods[$x-1]."\n";
					echo ">\t".$methods[$x]."\n";
					echo "\t".$methods[$x+1]."\n";
					echo "\t".$methods[$x+2]."\n\n";
				}
				continue;
			}
			
			$currline = ltrim($currline,"+- \t(");
			$endofreturntype = strpos( $currline, ")" );
			if( $endofreturntype == false )
			{
				echo "warning: can't find return type in \"".$methods[$x]."\"\n";
				continue;
			}
			$returntype = trim(str_replace(" *","*",substr($currline,0,$endofreturntype)));
			$currline = substr($currline,$endofreturntype+1,strlen($currline)-$endofreturntype);
			
			if( strpos($currline,":" ) === false )	// No params.
			{
				$output .= trim($currline).",$returntype\n";	// That's our full method name.
				continue;
			}
			
			// Otherwise, parse method name for name labels and params:
			$neuetreffer = array();
			$matchcount = preg_match_all( "/(([A-Za-z0-9_]*):([ ]*)(((\(([A-Za-z0-9_* \[\]<>&:]*)\)){0,1}([ ]*)([A-Za-z0-9_]*))))|(\,([ ]*)(\.\.\.))(([ ]+)([^.]+))*/", $currline, $neuetreffer );
			if( $matchcount < 1 )
			{
				echo "warning: can't find parameters in \"".$methods[$x]."\"\n";
				continue;
			}
			//print_r($neuetreffer);
			
			$methodname = "";
			$paramtypes = ",".$returntype;
			for( $z = 0; $z < sizeof($neuetreffer[0]); $z++ )
			{
				if( $neuetreffer[2][$z] == "" )
					$paramtypes .= ",...";
				else
				{
					$methodname .= $neuetreffer[2][$z].":";
					$currtype = str_replace("[]","*",trim($neuetreffer[7][$z]));	// 9 would be param name.
					$currtype = str_replace(" *","*",$currtype);
					if( trim($currtype) == "" )
						$currtype = "id";
					$paramtypes .= ",".$currtype;
				}
			}
			$output .= trim($methodname).$paramtypes."\n";
		}
		$output .= "\n";
	}

	return $output;
}


function	normalizeWhitespace( $str )
{
	$str = str_replace( "\n", " ", $str );
	$str = str_replace( "\r", " ", $str );
	while( strpos(str,"  ") !== false )
		$str = str_replace( "  ", " ", $str );
	
	return trim($str);
}


// -----------------------------------------------------------------------------
//	Now that we have all headers, we can use this function to find all functions
//	in them:
//

function parseFunctionsFromString( $headerstr, $currfmwkname )
{
	$output = "";
	$treffer = array();
	$fullregexp = "/extern[ \r\n]*(\\\"C\\\")*[ \r\n]*((const[ \r\n]*)*[A-Za-z][A-Za-z_0-9]*[ \r\n*]*)[ \r\n]*([A-Za-z][A-Za-z_0-9]*)[ \r\n]*\(([^)]*)\)/";
	$matchcount = preg_match_all( $fullregexp, $headerstr, $treffer );
	if( $matchcount == 0 )
		return "";
	echo "note: ".$currfmwkname.": $matchcount functions found\n";
	
	for( $x = 0; $x < sizeof($treffer[0]); $x++ )
	{
		$returntype = normalizeWhitespace( $treffer[2][$x] );
		$funcname = normalizeWhitespace($treffer[4][$x]);
		$paramlist = normalizeWhitespace( $treffer[5][$x] );
		$paramlist = str_replace( "*", "* ", $paramlist );
		while( strpos($paramlist," *") !== false )
			$paramlist = str_replace( " *", "*", $paramlist );
		$paramlist = trim( $paramlist );
		
		$output .= "=".$funcname.",".$returntype;
		$output .= extractParams( $paramlist ) . "\n";
	}
	
	//print_r($treffer);
	
	return $output;
}


// -----------------------------------------------------------------------------
//	Now that we have all headers, we can use this function to find all function
//	callback typedefs in them:
//

function parseProcPtrsFromString( $headerstr, $currfmwkname )
{
	global $knowntypes;

	$output = "";
	$treffer = array();
	$fullregexp = "/typedef[ \t\n\r]([A-Za-z_]([A-Za-z_0-9]*))[ \t\n\r]*\([ \t\n\r]*\*[ \t\n\r]*([A-Za-z_]([A-Za-z_0-9]*))\)[ \t\n\r]*\(([^)]*)\)*/";
	$matchcount = preg_match_all( $fullregexp, $headerstr, $treffer );
	if( $matchcount == 0 )
		return "";
	echo "note: ".$currfmwkname.": $matchcount procPointer types found\n";
	
	for( $x = 0; $x < sizeof($treffer[0]); $x++ )
	{
		$returntype = normalizeWhitespace( $treffer[1][$x] );
		$funcname = normalizeWhitespace($treffer[3][$x]);
		$paramlist = normalizeWhitespace( $treffer[5][$x] );
		$paramlist = str_replace( "*", "* ", $paramlist );
		while( strpos($paramlist," *") !== false )
			$paramlist = str_replace( " *", "*", $paramlist );
		$paramlist = trim( $paramlist );
		$normalizedparamlist = extractParams( $paramlist );
		
		$output .= "&".$funcname.",".$returntype.$normalizedparamlist . "\n";
		
		$knowntypes[$funcname] = array( "&", ",".$returntype.$normalizedparamlist."\n" );
	}
	
	//print_r($treffer);
	
	return $output;
}


// -----------------------------------------------------------------------------
//	Now that we have all headers, we can use this function to find all other
//	typedefs in them:
//

function parseTypedefsFromString( $headerstr, $currfmwkname )
{
	global $knowntypes;

	$output = "";
	$treffer = array();
	$fullregexp = "/typedef[ \t\n\r](((un){0,1}signed[ \t]*){0,1}[A-Za-z_]([A-Za-z_0-9]*))[ \t\n\r]*([A-Za-z_]([A-Za-z_0-9]*))[ \t\n\r]*;/";
	$matchcount = preg_match_all( $fullregexp, $headerstr, $treffer );
	if( $matchcount == 0 )
		return "";
	echo "note: ".$currfmwkname.": $matchcount typedefs found\n";
	
	for( $x = 0; $x < sizeof($treffer[0]); $x++ )
	{
		$oldname = $treffer[1][$x];
		$newname = $treffer[5][$x];
		
		if( isset($knowntypes[$oldname]) )
			$output .= $knowntypes[$oldname][0].$newname.$knowntypes[$oldname][1];
		else
			$output .= "~".$newname.",".$oldname."\n";
	}
	
	//print_r($treffer);
	
	return $output;
}


function parseEnumTypedefsFromString( $headerstr, $currfmwkname )
{
	global $knowntypes;

	$output = "";
	$treffer = array();
	$fullregexp = "/typedef[ \t\n\r]*enum[ \t\n\r]*([A-Za-z_]([A-Za-z_0-9]*))[ \t\n\r]*{([^}]*)}[ \t\n\r]*([A-Za-z_]([A-Za-z_0-9]*));/";
	$matchcount = preg_match_all( $fullregexp, $headerstr, $treffer );
	if( $matchcount == 0 )
		return "";
	echo "note: ".$currfmwkname.": $matchcount enum typedefs found\n";
	
	for( $x = 0; $x < sizeof($treffer[0]); $x++ )
	{
		$oldname = $treffer[1][$x];	// Just the enum name. Might wanna save this elsewhere.
		$enumconstants = $treffer[3][$x];
		$newname = $treffer[4][$x];

		$output .= "~".$newname.",int\n";
		
		$startnum = -1;
		$constants = explode(",",$enumconstants);
		for( $y = 0; $y < sizeof($constants); $y++ )
		{
			$parts = array();
			preg_match("/[ \t\n\r]*([A-Za-z_]([A-Za-z_0-9]*))[ \t\n\r]*(=[ \t\n\r]*([^ ,]*))*/",$constants[$y], $parts);
			
			$constantname = $parts[1];
			if( isset($parts[4]) )
				$constantvalue = $startnum = $parts[4];
			else
				$constantvalue = ++$startnum;
			$output .= "e".$constantname.",".$constantvalue."\n";
		}
	}
	
	return $output;
}


function extractParams( $paramlist )
{
	$output = "";
	$params = explode( ",", $paramlist );
	for( $x = 0; $x < sizeof($params); $x++ )
	{
		$treffer2 = array();
		$fullregexp2 = "/[ \t]*((const)*[ \t]*[A-Za-z]([A-Za-z_0-9]*)([*]*))([ ]*)(([A-Za-z_]([A-Za-z_0-9]*))*)/";
		$matchcount = preg_match( $fullregexp2, $params[$x], $treffer2 );
		//print_r($treffer2);
		$output .= ",".$treffer2[1];
		// [6] would be name of variable.
	}
	return $output;
}


//echo "Writing to disk.\n";
	
$fd = fopen("frameworkheaders.hhc","w");
fwrite( $fd, $directions );
fwrite( $fd, $output );
fclose( $fd );

//echo "Finished.\n";

?>