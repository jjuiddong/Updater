//
// 2017-01-08, jjuiddong
// Downloader Configuration
//
#pragma once


class cDownloaderConfig
{
public:
	cDownloaderConfig();
	virtual ~cDownloaderConfig();

	bool Read(const string &fileName);
	bool Write(const string &fileName);

	string m_projName;
	string m_ftpAddr;
	string m_ftpId;
	string m_ftpPasswd;
	string m_ftpDirectory;
	string m_localDirectory;
	string m_exeFileName;
};
