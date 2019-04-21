
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


// FTP ���ε带 ���� �ӽ÷� ������� ���������� �����Ѵ�.
void cFileList::RemoveTemporalZipFile(const string &sourceDirectory)
{
	for (auto &file : m_files)
	{
		const string zipFileName = ftppath::GetLocalFileName(
			sourceDirectory, file.fileName) + ".zip";
		remove(zipFileName.c_str());
	}
}


// ��ü ���ε� �� ����ũ�⸦ �����Ѵ�.
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


// ��ü ���� ���� ũ�⸦ �����Ѵ�.
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


// ���ε� �� ���ϵ鿡�� ��ü ���� ���� ũ�⸦ �����Ѵ�.
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


// ��ü ���� ũ�⸦ �����Ѵ�.
// ����� ������ ��� ���� ����ũ��, �׷��� ���� ��� ����ũ��� ����Ѵ�.
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
