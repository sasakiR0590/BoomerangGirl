#include"LoadCSV.h"
#include<fstream>
#include<sstream>
#include<codecvt>
#include<map>
#include<stdlib.h>

LoadCSV::LoadCSV()
{

}

LoadCSV::~LoadCSV()
{

}

void LoadCSV::LoadStatus(string filename)
{
	FILE* _file;
	_file = fopen(filename.c_str(), "r");

	char _buffer[512];

	char _char_status_name[256];

	while(fgets(_buffer, sizeof(_buffer), _file) != NULL)
	{
		float _data = 0.0f;

		sscanf(_buffer, "%s %f", &_char_status_name, &_data);
		std::string _status_name = _char_status_name;
		_status.insert(make_pair(_status_name, _data));
	}

	fclose(_file);
}