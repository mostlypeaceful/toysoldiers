// MapList.h: Holds the list of maps that are read from a file.
//

#pragma once

#include "stdafx.h"

using namespace std;

class MapList
{
private:
	ifstream mapFile;
	vector<string> pathList;

public:
	MapList(char* mapFilePath = "\\test\\bin\\map_list.txt");
	~MapList(void);

	int GetMapCount(void);
	const char* GetMapPath(int mapNumber);
};
