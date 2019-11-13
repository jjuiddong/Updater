# Updater
File Upload and Download program with VS2017 C++ MFC
- FTP file upload, download
- File Version Management
- Zip/Unzip Uploader, downloader multithreading
 - https://bitbucket.org/wbenny/ziplib
- Uploader program is Management File Version
- Downloader program is Download File from FTP Server
- Test with FileZilla Server 0.9.60 beta
- **Notice!! FTP Shared folders authority check (file: read/write/delete/append, directory: create/delete/list/+subdirs)**
- Uploader Process Sequence
  1. Compare Source and Latest Directory Files
  2. Check Diffrent File and Listing FileName
  3. Create Version File
  4. Download Version File from FTP Server
  5. Compare FTP Server Version File and Update Version File
  6. Decided which file to upload 
  7. Zip Upload Files
  8. Upload Files to FTP Server
  9. Copy or Remove Latest Directory Files from Source Directory Files
  10. Copy Source Directory files to Backup Directory with Zip if need
  11. Finish
- Downloader Process Sequence
  1. Download latest version file from FTP server
  2. Compare local and download version file
  3. Decided which file to download
  4. Download file from FTP Server
  5. Finish
 
 --------------------------------------------------------------
Uploader

![Alt text](https://github.com/jjuiddong/Updater/blob/master/Doc/2019-11-13-1.png?raw=true)

Uploader Different Window

![Alt text](https://github.com/jjuiddong/Updater/blob/master/Doc/2019-11-13-diffdlg.png?raw=true)

Project Editor

![Alt text](https://github.com/jjuiddong/Updater/blob/master/Doc/2019-11-13-projecteditor.png?raw=true)

Downloader

![Alt text](https://github.com/jjuiddong/Updater/blob/master/Doc/2019-11-13-downloader.png?raw=true)
