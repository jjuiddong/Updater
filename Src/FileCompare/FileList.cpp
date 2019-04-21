
#include "stdafx.h"
#include "FileList.h"


cFileList::cFileList()
{
}

cFileList::~cFileList()
{
	Clear();
}


bool cFileList::AddFile(const cFTPScheduler::sCommand &cmd)
{
	m_files.push_back(cmd);
	return true;
}


// FTP 업로드를 위해 임시로 만들어진 압축파일을 제거한다.
void cFileList::RemoveTemporalZipFile(const string &sourceDirectory)
{
	for (auto &file : m_files)
	{
		const string zipFileName = ftppath::GetLocalFileName(
			sourceDirectory, file.fileName) + ".zip";
		remove(zipFileName.c_str());
	}
}


// 전체 업로드 될 파일크기를 리턴한다.
long cFileList::GetUploadFileSize(const string &sourceDirectory)
{
	long totalFileSize = 0;
	for (auto &file : m_files)
	{
		if (cFTPScheduler::eCommandType::UPLOAD != file.cmd)
			continue;

		long fileSize = 0;
		if (file.isCompressed)
		{
			fileSize = (long)common::FileSize(
				ftppath::GetLocalFileName(sourceDirectory, file.fileName) + ".zip");
		}
		else
		{
			fileSize = (long)common::FileSize(
				ftppath::GetLocalFileName(sourceDirectory, file.fileName));
		}

		totalFileSize += fileSize;
	}
	return totalFileSize;
}


// 전체 원본 파일 크기를 리턴한다.
long cFileList::GetTotalSourceFileSize(const string &sourceDirectory)
{
	long totalFileSize = 0;
	for (auto &file : m_files)
	{
		const long fileSize = (long)common::FileSize(
			ftppath::GetLocalFileName(sourceDirectory, file.fileName));
		totalFileSize += fileSize;
	}
	return totalFileSize;
}


// 업로드 될 파일들에서 전체 원본 파일 크기를 리턴한다.
long cFileList::GetTotalUploadSourceFileSize(const string &sourceDirectory)
{
	long totalFileSize = 0;
	for (auto &file : m_files)
	{
		if (cFTPScheduler::eCommandType::UPLOAD != file.cmd)
			continue;

		const long fileSize = (long)common::FileSize(
			ftppath::GetLocalFileName(sourceDirectory, file.fileName));

		totalFileSize += fileSize;
	}
	return totalFileSize;
}


// 전체 파일 크기를 리턴한다.
// 압축된 파일일 경우 압축 파일크기, 그렇지 않을 경우 파일크기로 계산한다.
long cFileList::GetTotalFileSize(const string &sourceDirectory)
{
	long totalFileSize = 0;
	for (auto &file : m_files)
	{
		long fileSize = 0;
		if (file.isCompressed)
		{
			fileSize = (long)common::FileSize(
				ftppath::GetLocalFileName(sourceDirectory
					, file.fileName) + ".zip");
		}
		else
		{
			fileSize = (long)common::FileSize(
				ftppath::GetLocalFileName(sourceDirectory
					, file.fileName));
		}
		totalFileSize += fileSize;
	}
	return totalFileSize;
}


void cFileList::Clear()
{
	m_files.clear();
}
