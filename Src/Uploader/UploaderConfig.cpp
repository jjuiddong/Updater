
#include "stdafx.h"
#include "UploaderConfig.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>


cUploaderConfig::cUploaderConfig()
{
}

cUploaderConfig::~cUploaderConfig()
{
	Clear();
}


bool cUploaderConfig::Read(const string &fileName)
{
	Clear();

	try
	{
		using boost::property_tree::ptree;

		ptree props;
		boost::property_tree::read_json(fileName, props);
		ptree &children = props.get_child("project");
		for each(ptree::value_type vt in children)
		{
			sProjectInfo *info = new sProjectInfo;
			info->name = vt.second.get<string>("project name");
			info->ftpAddr = vt.second.get<string>("ftp address");
			info->ftpId = vt.second.get<string>("ftp id");
			info->ftpPasswd = vt.second.get<string>("ftp pass");
			info->ftpDirectory = vt.second.get<string>("ftp dir");
			info->lastestDirectory = vt.second.get<string>("lastest dir");
			info->backupDirectory = vt.second.get<string>("backup dir");
			info->sourceDirectory = vt.second.get<string>("source dir");
			info->exeFileName = vt.second.get<string>("exe file name");
			info->lastestVer = vt.second.get<string>("lastest version");

			m_projInfos.push_back(info);
		}
	}
	catch (std::exception&e)
	{
		dbg::Log("Read Error, upload configuraton,  %s\n", e.what());
		return false;
	}

	return true;
}


void cUploaderConfig::Clear()
{
	for each (auto p in m_projInfos)
		delete p;
	m_projInfos.clear();
}
