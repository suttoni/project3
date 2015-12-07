/*-------------------------------------------*/
/*
 * FAT_32 Filesystem - Project 3 - Header for structs and a few global constants
 * Team Satisfries - Ian Sutton, Ibrahim Atiya, Sai Gunasegaran, Yilin Wang
 *
/*-------------------------------------------*/
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
/*-------------------------------------------*/
/*
 * Constant declaratios to be used through the program
 *
/*-------------------------------------------*/
#define END_OF_CLUSTER 0x0FFFFFF8
#define ATTRIBUTE_NAME_LONG 0x0F
#define ENTRY_LAST 0x00
#define ENTRY_EMPTY 0xE5

struct FSI {
	// unsigned chars
	unsigned char	FSI_Reserved2[12];
	unsigned char 	FSI_Reserved1[480];
	// unsigned ints
	unsigned int 	FSI_TrailSig;
	unsigned int 	FSI_LeadSig;
	unsigned int 	FSI_StrucSig;
	unsigned int 	FSI_Free_Count;
	unsigned int 	FSI_Nxt_Free;
} __attribute__((packed));

struct DIR {
	// unsigned chars
	unsigned char 	DIR_Name[11];
	unsigned char 	DIR_Attr;
	unsigned char 	DIR_NTRes;
	unsigned char 	DIR_CrtTimeTenth;
	// unsigned shorts
	unsigned short 	DIR_CrtTime;
	unsigned short 	DIR_CrtDate;
	unsigned short 	DIR_LstAccDate;
	unsigned short 	DIR_FstClusHI;
	unsigned short 	DIR_WrtTime;
	unsigned short 	DIR_WrtDate;
	unsigned short 	DIR_FstClusLO;
	// unsigned int
	unsigned int 	DIR_FileSize;
} __attribute__((packed));

struct BPB_32 {
	// unsigned chars
	unsigned char	BS_jmpBoot[3];
	unsigned char	BS_OEMName[8];
	unsigned char	BPB_NumFATs;
	unsigned char	BPB_SecPerClus;
	unsigned char	BPB_Media;
	unsigned char	BPB_Reserved[12];
	unsigned char	BS_DrvNum;
	unsigned char	BS_Reserved1;
	unsigned char	BS_BootSig;
	unsigned char	BS_VolLab[11];
	unsigned char	BS_FilSysType[8];
	// unsigned shorts
	unsigned short	BPB_BytsPerSec;
	unsigned short	BPB_RsvdSecCnt;
	unsigned short	BPB_RootEntCnt;
	unsigned short	BPB_TotSec16;
	unsigned short	BPB_FATSz16;
	unsigned short	BPB_SecPerTrk;
	unsigned short	BPB_NumHeads;
	unsigned short	BPB_ExtFlags;
	unsigned short	BPB_FSVer;
	unsigned short	BPB_FSI_info;
	unsigned short	BPB_BkBootSec;
	// unsigned ints
	unsigned int	BPB_HiddSec;
	unsigned int	BPB_TotSec32;
	unsigned int	BPB_FATSz32;
	unsigned int	BS_VolID;
	unsigned int	BPB_RootClus;
} __attribute__((packed));
