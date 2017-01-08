
#include "stdafx.h"
#include "DownloaderConfig.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>


cDownloaderConfig::cDownloaderConfig()
{
}

cDownloaderConfig::~cDownloaderConfig()
{
}


bool cDownloaderConfig::Read(const string &fileName)
{
	try
	{
		using boost::property_tree::ptree;

		ptree props;
		boost::property_tree::read_json(fileName, props);
		m_ftpAddr = props.get<string>("ftp address");
		m_ftpId = props.get<string>("ftp id");
		m_ftpPasswd = props.get<string>("ftp pass");
		m_ftpDirectory = props.get<string>("ftp dir");
		m_localDirectory = props.get<string>("local dir");
		m_exeFileName = props.get<string>("exe file name");
	}
	catch (std::exception&e)
	{
		dbg::Log("downloader configuraton Read Error,  %s\n", e.what());
		return false;
	}

	return true;
}


bool cDownloaderConfig::Write(const string &fileName)
{

	return true;
}
