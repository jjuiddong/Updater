//
// 2017-01-07, jjuiddong
// Read Upload configuration
//
#pragma once


class cUploaderConfig
{
public:
	cUploaderConfig();
	virtual ~cUploaderConfig();

	bool Read(const string &fileName);
	bool Write(const string &fileName);
	void Clear();


public:
	struct sProjectInfo
	{
		string name;
		string ftpAddr;
		string ftpId;
		string ftpPasswd;
		string ftpDirectory;
		string lastestDirectory;
		string backupDirectory;
		string sourceDirectory;
		string exeFileName;
	};

	vector<sProjectInfo> m_projInfos;
};
