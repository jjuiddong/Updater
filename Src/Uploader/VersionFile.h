//
// 2017-01-08, jjuiddong
// uploader, downloader version file
// high version number is lastest file
// compare version file, create version file
//
// filename	<version>
//
// sample
// version 1
// filename1.txt	1
// path1/filename2.txt	2
// path2/filename3.txt	1
//
#pragma once


class cVersionFile
{
public:
	struct sCompareInfo
	{
		enum Enum {NOT_UPDATE, UPDATE, REMOVE};
		Enum state;
		string fileName;
	};

	cVersionFile();
	cVersionFile(const cVersionFile &rhs);
	virtual ~cVersionFile();

	bool Create(const string &directoryName);
	bool Read(const string &fileName);
	bool Write(const string &fileName);
	void Update(const string &directoryPath, vector<pair<DifferentFileType::Enum, string>> &diffFiles);
	bool Compare(const cVersionFile &ver, OUT vector<sCompareInfo> &out);
	void Clear();

	cVersionFile& operator=(const cVersionFile &rhs);


public:
	enum {CURRENT_VERSION = 1};

	int m_version; // cVersionFile version number
	vector<pair<int, string>> m_verFiles;	// pair<version number, file name>
};
