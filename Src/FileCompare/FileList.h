//
// 2019-04-20, jjuiddong
// upload/download file list
//
#pragma once

#include "FTPScheduler.h"


class cFileList
{
public:
	cFileList();
	virtual ~cFileList();

	bool AddFile(const cFTPScheduler::sCommand &cmd);
	long GetUploadFileSize(const string &sourceDirectory);
	long GetTotalSourceFileSize(const string &sourceDirectory);
	long GetTotalUploadSourceFileSize(const string &sourceDirectory);
	long GetTotalFileSize(const string &sourceDirectory);
	void RemoveTemporalZipFile(const string &sourceDirectory);

	void Clear();


public:
	vector<cFTPScheduler::sCommand> m_files;
};
