// MapList.cpp: Implementation of MapList.
//

#include "stdafx.h"

//----------------------------------------------------------------------------
// Name: MapList
// Desc: MapList constructor.
//----------------------------------------------------------------------------
MapList::MapList(char* mapFilePath)
{
	char fullyQualifiedPath[MAXBUFSIZE];
	size_t requiredSize;
	getenv_s(&requiredSize, fullyQualifiedPath, MAXBUFSIZE, "SigCurrentProject");
	strcat_s(fullyQualifiedPath, mapFilePath);

	mapFile.open(fullyQualifiedPath);
	if ( !mapFile.is_open() )
	{
		printf("Failed to open %s.\n", fullyQualifiedPath);
		return;
	}

	string line;
	while ( !mapFile.eof() )
	{
		getline(mapFile, line);
		if( line.length( ) > 0 )
			pathList.push_back(line);
	}

	mapFile.close();
}

//----------------------------------------------------------------------------
// Name: ~MapList
// Desc: MapList destructor.
//----------------------------------------------------------------------------
MapList::~MapList(void)
{
}

//----------------------------------------------------------------------------
// Name: GetMapCount
// Desc: Returns the number of maps in the path list.
//----------------------------------------------------------------------------
int MapList::GetMapCount(void)
{
	return pathList.size();
}

//----------------------------------------------------------------------------
// Name: GetMapPath
// Desc: Returns a path from the list.
//----------------------------------------------------------------------------
const char* MapList::GetMapPath(int mapNumber)
{
	if ( mapNumber >= GetMapCount() )
		mapNumber = 0;

	return pathList[mapNumber].c_str();
}
