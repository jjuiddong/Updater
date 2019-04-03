//
// 2019-04-03, jjuiddong
// message
//
#pragma once


struct sMessage
{
	enum Enum {
		NONE,
		LOGIN,
		DOWNLOAD_BEGIN,
		DOWNLOAD,
		DOWNLOAD_DONE,
		UPLOAD_BEGIN,
		UPLOAD,
		UPLOAD_DONE,
		ZIP_PROCESS_BEGIN,
		ZIP_BEGIN,
		ZIP,
		ZIP_DONE,
		ZIP_PROCESS_DONE,
		ERR,
		FINISH
	};

	Enum type;
	int data; // especially ERR state data (cFTPScheduler::sState)
	string fileName;
	int totalBytes;
	int readBytes;
	int progressBytes;

	sMessage() {}
	sMessage(const Enum type0, const string &fileName0, const int totalBytes0=0
		, const int readBytes0=0, const int progressBytes0=0, const int data0=0)
		: type(type0), fileName(fileName0), totalBytes(totalBytes0)
		, readBytes(readBytes0), progressBytes(progressBytes0)
		, data(data0) 
	{
	}
};

