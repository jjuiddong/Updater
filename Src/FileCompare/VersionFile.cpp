
#include "stdafx.h"
#include "VersionFile.h"


cVersionFile::cVersionFile()
	: m_version(1)
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
	common::CollectFiles2({}, directoryName, directoryName, files);
	if (files.empty())
		return false; 

	for each (auto file in files)
		m_files.push_back({ 1, (long)FileSize(directoryName + "/" + file), 0, file} );

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
		if (strs.size() == 2)
		{
			if (strs[0] == "version")
			{
				m_version = atoi(strs[1].c_str());
			}
		}
		else if (strs.size() >= 4)
		{
			m_files.push_back({ atoi(strs[1].c_str()), atol(strs[2].c_str()), atol(strs[3].c_str()), strs[0] });
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

	// filename \t version \t filesize
	for each (auto file in m_files)
		ofs << file.fileName << "\t" << file.version << "\t" << file.fileSize << "\t" << file.compressSize << endl;

	return true;
}


// Update Version File
void cVersionFile::Update(const string &directoryPath
	, vector<pair<DifferentFileType::Enum, string>> &diffFiles)
{
	using namespace std;

	for (auto &diff : diffFiles)
	{
		const string fileName = diff.second;

		bool isFind = false;
		// Find in VersionFiles array
		for (u_int i=0; i < m_files.size(); ++i)
		{
			if (m_files[i].fileName == fileName)
			{
				isFind = true;
				switch (diff.first)
				{
				case DifferentFileType::ADD:
					if (m_files[i].version < 0)
						m_files[i].version = abs(m_files[i].version) + 1; // add file
					else
						dbg::Log("Error Occur, Already Exist File, Add operation %s \n", fileName.c_str());
					break;
				case DifferentFileType::REMOVE:
					m_files[i].version = -abs(m_files[i].version); // Negative version is Remove file
					break;
				case DifferentFileType::MODIFY:
					m_files[i].version = abs(m_files[i].version) + 1; // Version Update
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
				m_files.push_back({ 1, (long)FileSize(GetFullFileName(directoryPath+fileName)), 0, fileName });
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

	// Check Update or Remove
	for (auto &ver1 : m_files)
	{
		bool isFind = false;
		for (auto &ver2 : ver.m_files)
		{
			if (ver1.fileName != ver2.fileName)
				continue;

			isFind = true;

			sCompareInfo compInfo;
			compInfo.version = ver2.version; // compare file version
			compInfo.maxVersion = max(ver1.version, ver2.version);
			compInfo.fileName = ver1.fileName;

			const bool isSame = (ver1.version == ver2.version) && (ver1.fileSize == ver2.fileSize);
			if (isSame)
			{
				compInfo.state = sCompareInfo::NOT_UPDATE;
			}
			else
			{
				updateCount++;

				if ((ver1.version < 0) && (ver2.version < 0)) // Negative Version is Remove File
				{
					compInfo.state = sCompareInfo::REMOVE;
				}
				else if (0 > ver2.version)
				{
					compInfo.state = sCompareInfo::REMOVE;
				}
				else
				{
					compInfo.state = sCompareInfo::UPDATE;
					compInfo.fileSize = ver2.fileSize; // update file size
					compInfo.compressSize = ver2.compressSize;
				}
			}

			out.push_back(compInfo);
			break; // find!! done
		}

		if (!isFind)
		{
			// Not Found in FTP Server Version File, Then Remove
			updateCount++;
			sCompareInfo compInfo;
			compInfo.version = ver1.version;
			compInfo.fileName = ver1.fileName;
			compInfo.state = sCompareInfo::REMOVE;
			out.push_back(compInfo);
		}
	}

	// Check Add file
	for each (auto ver1 in ver.m_files)
	{
		bool isFind = false;
		for each (auto ver2 in m_files)
		{
			if (ver1.fileName == ver2.fileName)
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
			compInfo.version = ver1.version;
			compInfo.fileName = ver1.fileName;
			compInfo.fileSize = ver1.fileSize;
			compInfo.compressSize = ver1.compressSize;
			compInfo.state = (0 > ver1.version)? sCompareInfo::REMOVE : sCompareInfo::UPDATE;
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
		m_files = rhs.m_files;
	}
	return *this;
}


void cVersionFile::Clear() 
{
	m_version = 1;
	m_files.clear();
}


cVersionFile::sVersionInfo* cVersionFile::GetVersionInfo(const string &fileName)
{
	for (u_int i=0; i < m_files.size(); ++i)
	{
		if (fileName == m_files[i].fileName)
			return &m_files[i];
	}
	return NULL;
}
