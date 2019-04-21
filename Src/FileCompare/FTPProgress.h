//
// 2019-04-20, jjuiddong
// FTP Upload, Download Progress
//
#pragma once


//--------------------------------------------------------------------------------------------------
// cProgressNotify
class cProgressNotify : public nsFTP::CFTPClient::CNotification
{
public:
	cProgressNotify(cFTPScheduler *p) : m_p(p) {}

	virtual void OnPreReceiveFile(const tstring& strSourceFile, const tstring& strTargetFile
		, long lFileSize)
	{
		g_message.push(new sMessage(sMessage::DOWNLOAD_BEGIN
			, wstr2str(strTargetFile), m_sourceFileName, lFileSize));

		m_progress = 0;
		m_fileName = wstr2str(strTargetFile);
	}

	virtual void OnBytesReceived(const nsFTP::TByteVector& vBuffer, long lReceivedBytes)
	{
		m_progress += lReceivedBytes;

		g_message.push(new sMessage(sMessage::DOWNLOAD
			, m_fileName, m_sourceFileName, m_fileSize, lReceivedBytes, m_progress));
	}

	virtual void OnEndReceivingData(long lReceivedBytes)
	{
	}

	virtual void OnPostReceiveFile(const tstring& strSourceFile, const tstring& strTargetFile
		, long lFileSize)
	{
		g_message.push(new sMessage(sMessage::DOWNLOAD_DONE
			, m_fileName, m_sourceFileName, m_fileSize, 0, m_fileSize));
	}

	virtual void OnPreSendFile(const tstring& strSourceFile, const tstring& strTargetFile
		, long lFileSize)
	{
		g_message.push(new sMessage(sMessage::UPLOAD_BEGIN
			, wstr2str(strSourceFile), m_sourceFileName, lFileSize, 0, 0));

		m_progress = 0;
		m_fileSize = lFileSize;
		m_fileName = wstr2str(strSourceFile);
	}

	virtual void OnBytesSent(const nsFTP::TByteVector& vBuffer, long lSentBytes)
	{
		m_progress += lSentBytes;

		g_message.push(new sMessage(sMessage::UPLOAD
			, m_fileName, m_sourceFileName, m_fileSize, lSentBytes, m_progress));
	}

	virtual void OnPostSendFile(const tstring& strSourceFile, const tstring& strTargetFile
		, long lFileSize)
	{
		g_message.push(new sMessage(sMessage::UPLOAD_DONE
			, m_fileName, m_sourceFileName, m_fileSize, 0, m_fileSize));
	}

	cFTPScheduler *m_p; // reference
	string m_fileName;
	string m_sourceFileName;
	long m_fileSize;
	long m_progress;
};

