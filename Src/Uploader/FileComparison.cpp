
#include "stdafx.h"
#include "FileComparison.h"


// Compare File in Directory
// If Different File Contents Then Store out variable with File Name
// out : pair< command type, file name >
//		- command type : 0 = add
//									  1 = remove
//									  2 = modify 
//
void CompareDirectory(const string &srcDirectory,
	const string &compareDirectory,
	OUT vector<std::pair<int,string>> &out
)
{
	const string srcFullDirectory = common::GetFullFileName(srcDirectory);
	const string compareFullDirectory = common::GetFullFileName(compareDirectory);

	list<string> files1, files2;
	common::CollectFiles({}, srcFullDirectory, files1);
	common::CollectFiles({}, compareFullDirectory, files2);

	list<string> srcFiles;
	for each (auto str in files1)
	{
		const string fileName = common::RelativePathTo(srcFullDirectory, str);
		srcFiles.push_back(fileName);
	}

	list<string> compFiles;
	for each (auto str in files2)
	{
		const string fileName = common::RelativePathTo(compareFullDirectory, str);
		compFiles.push_back(fileName);
	}

	// Compare Files to Add or Modify
	for each (auto fileName in srcFiles)
	{
		auto it = find(compFiles.begin(), compFiles.end(), fileName);
		if (it == compFiles.end())
		{ // add file
			out.push_back( std::pair<int,string>(0, srcFullDirectory+fileName) );
		}
		else
		{
			if (CompareFile(srcFullDirectory + fileName, compareFullDirectory + fileName))
			{ // modify file
				out.push_back(std::pair<int, string>(2, srcFullDirectory+fileName));
			}
		}
	}

	// Search File to Remove Files
	for each (auto fileName in compFiles)
	{
		auto it = find(srcFiles.begin(), srcFiles.end(), fileName);
		if (it == srcFiles.end())
		{ // remove file
			out.push_back(std::pair<int, string>(1, srcFullDirectory+fileName));
		}
	}
}


// If Different file, return true
// check modify date
// check file size
bool CompareFile(const string &fileName1, const string &fileName2)
{
	using namespace std;

	HANDLE hFile1 = CreateFileA(fileName1.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (!hFile1)
		return false;

	HANDLE hFile2 = CreateFileA(fileName2.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (!hFile2)
		return false;

	DWORD fileSizeH1;
	DWORD fileSizeL1 = GetFileSize(hFile1, &fileSizeH1);
	DWORD fileSizeH2;
	DWORD fileSizeL2 = GetFileSize(hFile2, &fileSizeH2);

	FILETIME crTime1, accessTime1, writeTime1;
	FILETIME crTime2, accessTime2, writeTime2;
	GetFileTime(hFile1, &crTime1, &accessTime1, &writeTime1);
	GetFileTime(hFile2, &crTime2, &accessTime2, &writeTime2);

	bool retVal = false;

	if (fileSizeL1 != fileSizeL2)
		retVal = true;

	if ((writeTime1.dwLowDateTime != writeTime2.dwLowDateTime) ||
		(writeTime1.dwHighDateTime != writeTime2.dwHighDateTime))
		retVal = true;

	CloseHandle(hFile1);
	CloseHandle(hFile2);

	return retVal;
}
