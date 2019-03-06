//
//  fake_filesystem.cpp
//  bugmatic
//
//  Created by Uli Kusterer on 19/11/2016.
//  Copyright © 2016 Uli Kusterer. All rights reserved.
//

#include "fake_filesystem.hpp"
#include <dirent.h>
#include <sys/stat.h>
#include <vector>
#include <iostream>


namespace fake {
namespace filesystem {


bool	exists( const path& inPath )
{
	struct stat sb;

	return( stat(inPath.string().c_str(), &sb) == 0 );
}


std::ostream& operator << ( std::ostream& inOutputStream, const path& inPath )
{
	return inOutputStream << inPath.string().c_str();
}


path path::filename() const
{
	off_t searchStart = std::string::npos;
	if( mPath.length() > 0 && mPath[mPath.length()-1] == '/' )	// Ends in slash?
		searchStart = mPath.length() -2;	// Ignore trailing slash.
	size_t pos = mPath.rfind("/", searchStart);
    if( pos != std::string::npos )
        return path(mPath.substr(pos +1));
	return path("");
}


path path::parent_path() const
{
	off_t searchStart = std::string::npos;
	if( mPath.length() > 0 && mPath[mPath.length()-1] == '/' )	// Ends in slash?
		searchStart = mPath.length() -2;	// Ignore trailing slash.
	size_t pos = mPath.rfind("/",searchStart);
    if( pos != std::string::npos )
	{
        return path(mPath.substr(0,pos));
	}
	return path("");
}


path& path::operator /= ( const path& inAppendee )
{
	if( mPath.length() > 0 && mPath[mPath.length()-1] != '/' )
		mPath.append("/");
	mPath.append(inAppendee.mPath);
	
	return *this;
}


path path::operator / ( const path& inAppendee ) const
{
	path	newPath(*this);
	if( mPath.length() > 0 && mPath[mPath.length()-1] != '/' )
		newPath.mPath.append("/");
	newPath.mPath.append(inAppendee.mPath);
	
	return newPath;
}


directory_iterator::directory_iterator( path inPath )
{
	mPath = inPath;
    mDir = new dir(opendir( inPath.string().c_str() ));
	
	// Fetch the first file entry:
	++(*this);
}


directory_iterator::directory_iterator( const directory_iterator& inOriginal )
{
	mPath = inOriginal.mPath;
	mDir = inOriginal.mDir->acquire();
}


directory_iterator::~directory_iterator()
{
	if( mDir )
		mDir->release();
}


directory_iterator& directory_iterator::operator =( const directory_iterator& inOriginal )
{
	mPath = inOriginal.mPath;
	if( mDir != inOriginal.mDir )
	{
		if( mDir )
			mDir->release();
		if( inOriginal.mDir )
			mDir = inOriginal.mDir->acquire();
		else
			mDir = nullptr;
	}
	return *this;
}

directory_iterator directory_iterator::operator ++ ()
{
	if( !mDir || mDir->get_dir() == nullptr )
	{
		mEntry.mPath = path("");
		return *this;
	}
	
    struct dirent *dp = nullptr;
	do
	{
		dp = readdir( mDir->get_dir() );
		if( dp )
			mEntry.mPath = mPath / path(dp->d_name);
	}
	while( dp && (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0) );

	if( dp == nullptr )
		mEntry.mPath = path("");
	
	return *this;
}


bool	directory_iterator::operator == ( const directory_iterator& inOther ) const
{
	return inOther.mEntry.mPath == mEntry.mPath;
}

bool	directory_iterator::operator != ( const directory_iterator& inOther ) const
{
	return inOther.mEntry.mPath != mEntry.mPath;
}


bool create_directory( const path& p )
{
	int result = mkdir( p.string().c_str(), 0777 );
	
	return (result == 0);
}


bool create_directories( const path& p )
{
	std::vector<path>	pathsToCreate;
	path				currPath = p;
	while( !exists(currPath) && currPath.string().length() > 0 )
	{
		pathsToCreate.push_back(currPath);
		currPath = currPath.parent_path();
	}
	for( path pathToCreate : pathsToCreate )
	{
		if( !create_directory(pathToCreate) )
			return false;
	}
	return true;
}

} // namespace fake
	
} // namespace filesystem
