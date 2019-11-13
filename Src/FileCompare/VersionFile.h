//
// 2017-01-08, jjuiddong
// uploader, downloader version file
// high version number is latest file
// compare version file, create version file
//
// filename	<version>	<filesize (bytes)>		<compressed filesize (bytes)>
//
// sample
// version 1
// filename1.txt	1	100
// path1/filename2.txt	2	10000
// path2/filename3.txt	1	20000
//
#pragma once


class cVersionFile
{
public:
	struct sCompareInfo
	{
		enum Enum {NOT_UPDATE, UPDATE, REMOVE};
		Enum state;
		int version;
		int maxVersion;
		long fileSize;
		long compressSize;
		string fileName;
	};

	// tuple<version, file name, file size> ==> version : negative -> remove file
	struct sVersionInfo
	{
		int version;
		long fileSize;
		long compressSize;
		string fileName;
	};

	cVersionFile();
	cVersionFile(const cVersionFile &rhs);
	virtual ~cVersionFile();

	bool Create(const string &directoryName);
	bool Read(const string &fileName);
	bool Write(const string &fileName);
	void Update(const string &directoryPath, vector<pair<DifferentFileType::Enum, string>> &diffFiles);
	int Compare(const cVersionFile &ver, OUT vector<sCompareInfo> &out);
	sVersionInfo* GetVersionInfo(const string &fileName);
	void Clear();

	cVersionFile& operator=(const cVersionFile &rhs);


public:
	int m_version; // patch version number
	vector<sVersionInfo> m_files;
};
