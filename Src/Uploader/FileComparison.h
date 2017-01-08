//
// 2017-01-07, jjuiddong
// file comparison
//
#pragma once



struct DifferentFileType
{
	enum Enum {ADD, REMOVE, MODIFY};
};


void CompareDirectory(const string &srcDirectory, const string &compareDirectory,
	OUT vector<pair<DifferentFileType::Enum, string>> &out
);


bool CompareFile(const string &fileName1, const string &fileName2);

