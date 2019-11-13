# Updater
File Upload and Download program with VS2017 C++
- FTP file upload, download
- File Version Management
- Zip/Unzip Uploader, downloader multithreading
 - https://bitbucket.org/wbenny/ziplib
- Uploader program is Management File Version
- Downloader program is Download File from FTP Server
- Uploader process sequence
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
- Downloader process sequence
  1. Download latest version file from FTP server
  2. Compare local and download version file
  3. Decided which file to download
  4. Download file from FTP Server
  5. Finish
 
 --------------------------------------------------------------
Uploader

![Alt text](https://github.com/jjuiddong/Updater/blob/master/Doc/img1.png?raw=true)

Uploader Different Window

![Alt text](https://github.com/jjuiddong/Updater/blob/master/Doc/img2.png?raw=true)

Downloader

![Alt text](https://github.com/jjuiddong/Updater/blob/master/Doc/img3.png?raw=true)
