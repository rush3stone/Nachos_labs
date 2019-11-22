// filehdr.h 
//	Data structures for managing a disk file header.  
//
//	A file header describes where on disk to find the data in a file,
//	along with other information about the file (for instance, its
//	length, owner, etc.)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"

#ifndef FILEHDR_H
#define FILEHDR_H

#include "disk.h"
#include "bitmap.h"

#define NumOfInt_Header 2
#define NumOfTime_Header 3
#define MaxTypeLength 5        // 4 + 1('/0')
#define LengthOfTimeStr 25     // 24 + 1
#define LengthOfAllString MaxTypeLength + LengthOfTimeStr*NumOfTime_Header

// pyq: num of direct pointers to the file's data sectors
#define NumDirect 	((SectorSize - NumOfInt_Header * sizeof(int) - LengthOfAllString) / sizeof(int))
#define MaxFileSize 	(NumDirect * SectorSize)
// Lab5 fileSys: limit of directory depth
#define MaxDirectoryDepth 10

// The following class defines the Nachos "file header" (in UNIX terms,  
// the "i-node"), describing where on disk to find all of the data in the file.
// The file header is organized as a simple table of pointers to
// data blocks. 
//
// The file header data structure can be stored in memory or on disk.
// When it is on disk, it is stored in a single sector -- this means
// that we assume the size of this data structure to be the same
// as one disk sector.  Without indirect addressing, this
// limits the maximum file length to just under 4K bytes.
//
// There is no constructor; rather the file header can be initialized
// by allocating blocks for the file (if it is a new file), or by
// reading it from disk.

class FileHeader {
  public:
    bool Allocate(BitMap *bitMap, int fileSize);// Initialize a file header, 
						//  including allocating space 
						//  on disk for the file data
    void Deallocate(BitMap *bitMap);  		// De-allocate this file's 
						//  data blocks

    void FetchFrom(int sectorNumber); 	// Initialize file header from disk
    void WriteBack(int sectorNumber); 	// Write modifications to file header
					//  back to disk

    int ByteToSector(int offset);	// Convert a byte offset into the file
					// to the disk sector containing
					// the byte

    int FileLength();			// Return the length of the file 
					// in bytes

    void Print();			// Print the contents of the file.

    //Lab5-Ex２ update info of Header
    void HeaderCreateInit(char *typ);  //initialize all info for creation
    
    void setFileType(char *typ) {strcmp(typ, "") ? strcpy(fileType, typ) : strcpy(fileType, "None");}
    void setCreateTime(char *t) {strcpy(createdTime, t);}
    void setVisitTime(char *t) {strcpy(lastVisitedTime, t);}
    void setModifyTime(char *t) {strcpy(lastModifiedTime, t);}
    void setHeaderSector(int sectorID) {headerSector = sectorID;}
    int getHeaderSector() {return headerSector;}
  

  private:
    int numBytes;			// Number of bytes in the file
    int numSectors;			// Number of data sectors in the file
    int dataSectors[NumDirect];		// Disk sector numbers for each data 
					// block in the file

    // Lab5 filesys-Ex2: add info of file
    char fileType[MaxTypeLength];
    char createdTime[LengthOfTimeStr];      
    char lastVisitedTime[LengthOfTimeStr];
    char lastModifiedTime[LengthOfTimeStr];
    // char *fileRoad[MaxDirectoryDepth];  //pyq: the limit of Directory depth

    int headerSector;   //保存文件头所在的sector编号
};

extern char* getFileType(char *fileName);
extern char* getCurrentTime(void);


#endif // FILEHDR_H
