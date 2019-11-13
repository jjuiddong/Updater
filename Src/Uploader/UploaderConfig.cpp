
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
			sProjectInfo info;
			info.name = vt.second.get<string>("project name");
			info.ftpAddr = vt.second.get<string>("ftp address");
			info.ftpId = vt.second.get<string>("ftp id");
			info.ftpPasswd = vt.second.get<string>("ftp pass");
			info.ftpDirectory = vt.second.get<string>("ftp dir");
			info.latestDirectory = vt.second.get<string>("latest dir");
			info.backupDirectory = vt.second.get<string>("backup dir");
			info.sourceDirectory = vt.second.get<string>("source dir");
			info.exeFileName = vt.second.get<string>("exe file name");

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


bool cUploaderConfig::Write(const string &fileName) 
{
	try
	{
		using boost::property_tree::ptree;

		ptree props;
		ptree project;

		for each (auto proj in m_projInfos)
		{
			ptree info;
			info.put<string>("project name", proj.name);
			info.put<string>("ftp address", proj.ftpAddr);
			info.put<string>("ftp id", proj.ftpId);
			info.put<string>("ftp pass", proj.ftpPasswd);
			info.put<string>("ftp dir", proj.ftpDirectory);
			info.put<string>("latest dir", proj.latestDirectory);
			info.put<string>("backup dir", proj.backupDirectory);
			info.put<string>("source dir", proj.sourceDirectory);
			info.put<string>("exe file name", proj.exeFileName);
			project.push_back(std::make_pair("", info));
		}

		props.add_child("project", project);
		boost::property_tree::write_json(fileName, props);
	}
	catch (std::exception&e)
	{
		dbg::Log("Write Error, upload configuraton,  %s\n", e.what());
		return false;
	}

	return true;
}


void cUploaderConfig::Clear()
{
	m_projInfos.clear();
}
