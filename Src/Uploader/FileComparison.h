//
// 2017-01-07, jjuiddong
// file comparison
//
#pragma once



void CompareDirectory(const string &srcDirectory, const string &compareDirectory,
	OUT vector<std::pair<int, string>> &out
);


bool CompareFile(const string &fileName1, const string &fileName2);

