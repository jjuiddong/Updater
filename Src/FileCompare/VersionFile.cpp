
#include "stdafx.h"
#include "VersionFile.h"


cVersionFile::cVersionFile()
	: m_version(CURRENT_VERSION)
{
}

cVersionFile::cVersionFile(const cVersionFile &rhs)
{
	operator=(rhs);
}

cVersionFile::~cVersionFile()
{
	Clear();
}


// Create VersionFile Class
bool cVersionFile::Create(const string &directoryName)
{
	Clear();

	list<string> files;
	common::CollectFiles({}, directoryName, files);
	if (files.empty())
		return false; 

	for each (auto file in files)
	{
		const string fileName = DeleteCurrentPath(RelativePathTo(directoryName, file));
		m_verFiles.push_back(pair<int, string>(1, fileName));
	}

	return true;
}


// Read Version File
bool cVersionFile::Read(const string &fileName)
{
	Clear();

	std::ifstream ifs(fileName);
	if (!ifs.is_open())
		return false;

	string line;
	while (getline(ifs, line))
	{
		if (line.empty())
			continue;

		vector<string> strs;
		common::tokenizer(line, "\t", "", strs);
		if (strs.size() < 2)
			continue;

		if (strs[0] == "version")
		{
			m_version = atoi(strs[1].c_str());
		}
		else
		{
			m_verFiles.push_back(pair<int, string>(atoi(strs[1].c_str()), strs[0]));
		}
	}

	return true;
}


// Write Version File
bool cVersionFile::Write(const string &fileName)
{
	using namespace std;

	std::ofstream ofs(fileName);
	if (!ofs.is_open())
		return false;

	ofs << "version" << "\t" << m_version << endl;

	for each (auto file in m_verFiles)
		ofs << file.second << "\t" << file.first << endl;

	return true;
}


// Update Version File
void cVersionFile::Update(const string &directoryPath, vector<pair<DifferentFileType::Enum, string>> &diffFiles)
{
	for each (auto diff in diffFiles)
	{
		const string fileName = DeleteCurrentPath(RelativePathTo(directoryPath, diff.second));

		bool isFind = false;
		// Find in VersionFiles array
		for (u_int i=0; i < m_verFiles.size(); ++i)
		{
			if (m_verFiles[i].second == fileName)
			{
				isFind = true;
				switch (diff.first)
				{
				case DifferentFileType::ADD:
					dbg::Log("Error Occur, Already Exist File, Add operation %s \n", fileName.c_str());
					break;
				case DifferentFileType::REMOVE:
					m_verFiles[i].first = -1;
					break;
				case DifferentFileType::MODIFY:
					++m_verFiles[i].first; // Version Update
					break;
				}

				break; // done
			}
		}

		if (!isFind)
		{
			switch (diff.first)
			{
			case DifferentFileType::ADD:
				m_verFiles.push_back(pair<int, string>(1, fileName));
				break;
			case DifferentFileType::REMOVE:
				dbg::Log("Error Occur, Not Exist File, Remove operation %s \n", fileName.c_str());
				break;
			case DifferentFileType::MODIFY:
				dbg::Log("Error Occur, Not Exist File, Modify operation %s \n", fileName.c_str());
				break;
			}
		}
	}
}


// Compare between VersionFile
// return value : update file count, (update or remove)
int cVersionFile::Compare(const cVersionFile &ver, OUT vector<sCompareInfo> &out)
{
	int updateCount = 0;

	// this ==> Another VersionFile
	for each (auto ver1 in m_verFiles)
	{
		bool isFind = false;
		for each (auto ver2 in ver.m_verFiles)
		{
			if (ver1.second == ver2.second)
			{
				isFind = true;
				
				sCompareInfo compInfo;
				compInfo.fileName = ver1.second;

				if (ver1.first == ver2.first)
				{
					compInfo.state = sCompareInfo::NOT_UPDATE;
				}
				else if (ver1.first < ver2.first)
				{
					updateCount++;
					compInfo.state = sCompareInfo::UPDATE;
				}
				else if (-1 == ver2.first)
				{
					updateCount++;
					compInfo.state = sCompareInfo::REMOVE;
				}

				out.push_back(compInfo);
				break; // done
			}
		}

		if (!isFind)
		{
			// Not Found in Another Version File, Then Remove
			updateCount++;
			sCompareInfo compInfo;
			compInfo.fileName = ver1.second;
			compInfo.state = sCompareInfo::REMOVE;
			out.push_back(compInfo);
		}
	}

	// Another VersionFile ==> this
	for each (auto ver1 in ver.m_verFiles)
	{
		bool isFind = false;
		for each (auto ver2 in m_verFiles)
		{
			if (ver1.second == ver2.second)
			{
				isFind = true;
				// Nothing~
				break;
			}
		}

		if (!isFind)
		{
			// Not Found in This Version File, Then Add File
			updateCount++;
			sCompareInfo compInfo;
			compInfo.fileName = ver1.second;
			compInfo.state = (-1==ver1.first)? sCompareInfo::REMOVE : sCompareInfo::UPDATE;
			out.push_back(compInfo);
		}
	}

	return updateCount;
}


cVersionFile& cVersionFile::operator=(const cVersionFile &rhs)
{
	if (this != &rhs)
	{
		m_version = rhs.m_version;
		m_verFiles = rhs.m_verFiles;
	}
	return *this;
}


void cVersionFile::Clear() 
{
	m_version = CURRENT_VERSION;
	m_verFiles.clear();
}
