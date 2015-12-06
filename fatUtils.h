/*-------------------------------------------*/
/*
 *
 *
/*-------------------------------------------*/
 extern struct FSI BPB_FSI_info;
 extern struct BPB_32 bpb_32;
 extern long openedFile[100];
 extern int openedFileNum;
 extern FILE *file;

 #define CLUSTER_END 0xFFFFFFFF
 #define OFFSET_CONST 32
/*-------------------------------------------*/
/*
 *
 *
/*-------------------------------------------*/
// Declaration of function of struct type "DIR" *found in FAT_32Structs.h*
struct DIR find_dir_file_entry(unsigned int cluster, char *name);
// Void declarations
void change_val_cluster(unsigned int value, unsigned int cluster);
void empty_val_cluster(unsigned int cluster);
// Int declarations
int equals(unsigned char name1[], unsigned char name2[]);
int unopened(long offset);
// Unsigned Int declarations
unsigned int FAT_32(unsigned int cluster);
unsigned int return_clus_dir_entry(unsigned int cluster, char *name);
// Long declarations
long return_sector_offset(long sec);
long first_sector_cluster(unsigned int cluster);
long find_cluster_ENTRY_EMPTY(unsigned int cluster);
long return_entry_offset(unsigned int cluster, char *name);
long return_path_cluster(char *string);

/*-------------------------------------------*/
/*
 *
 *
/*-------------------------------------------*/

struct DIR find_dir_file_entry(unsigned int cluster, char *name){
	int i;
	long offset;
	char file_name[12];
	struct DIR DIR_entry;

	for(;;){
		offset = return_sector_offset(first_sector_cluster(cluster));
		fseek(file, offset, SEEK_SET);

		long temp = return_sector_offset(first_sector_cluster(cluster)) + bpb_32.BPB_BytsPerSec * bpb_32.BPB_SecPerClus;
		while (temp >= offset){

			fread(&DIR_entry, sizeof(struct DIR), 1, file);

			if (DIR_entry.DIR_Name[0] == ENTRY_EMPTY)
				continue;

			else if (DIR_entry.DIR_Name[0] == ENTRY_LAST)
				return DIR_entry;

			else if (DIR_entry.DIR_Name[0] == 0x05)
				DIR_entry.DIR_Name[0] = ENTRY_EMPTY;

			if (DIR_entry.DIR_Attr != ATTRIBUTE_NAME_LONG){

				for (i=0; i<11; i++)
					file_name[i]=DIR_entry.DIR_Name[i];

				file_name[11]= '\0';

				if (strcmp(file_name, name) == 0)
					return DIR_entry;
			}
			offset+=32;
		}

		cluster = FAT_32(cluster);

		if (END_OF_CLUSTER < cluster)
			break;
	}
}
/*-------------------------------------------*/
/*
 *
 *
/*-------------------------------------------*/
void change_val_cluster(unsigned int value, unsigned int cluster){
	long offset;
	offset = bpb_32.BPB_RsvdSecCnt * bpb_32.BPB_BytsPerSec + cluster * 4;
	fseek(file, offset, SEEK_SET);
	fwrite(&value, sizeof(unsigned int), 1, file);
	fflush(file);
}
/*-------------------------------------------*/
/*
 *
 *
/*-------------------------------------------*/
void empty_val_cluster(unsigned int cluster){
	int i = 0;
	unsigned char *empty;
	unsigned int value = 0;
	long offset;
	
	empty = (unsigned char *)malloc(sizeof(unsigned char) * bpb_32.BPB_BytsPerSec * bpb_32.BPB_SecPerClus);

	for (i; i < bpb_32.BPB_BytsPerSec * bpb_32.BPB_SecPerClus; i++)
		empty[i] = 0;
	
	offset = bpb_32.BPB_RsvdSecCnt*bpb_32.BPB_BytsPerSec + cluster * 4;

	if (FAT_32(cluster) <= 0x0FFFFFEF && FAT_32(cluster) >= 0x00000002)
		empty_val_cluster(FAT_32(cluster));		//recursively empty all the cluster linked

	fseek(file, return_sector_offset(first_sector_cluster(cluster)), SEEK_SET);
	fwrite(empty, bpb_32.BPB_BytsPerSec*bpb_32.BPB_SecPerClus, 1, file);
	fseek(file, offset, SEEK_SET);
	fwrite(&value, sizeof(unsigned int), 1, file);
}
/*-------------------------------------------*/
/*
 *
 *
/*-------------------------------------------*/
int equals(unsigned char name1[], unsigned char name2[]){
	int i = 0;
	for (i; i < 11; i++){
		if (name1[i] != name2[i])
			return 0;
	}
	return 1;
}
/*-------------------------------------------*/
/*
 *
 *
/*-------------------------------------------*/
int unopened(long offset){
	int i = 0;

	for (i; i < openedFileNum; i++){
		if (openedFile[i] == offset)
			return 0;
	}
	return 1;
}
/*-------------------------------------------*/
/*
 *
 *
/*-------------------------------------------*/
unsigned int FAT_32(unsigned int cluster){
	unsigned int next_cluster;
	long offset;

	offset = bpb_32.BPB_RsvdSecCnt*bpb_32.BPB_BytsPerSec + cluster*4;
	fseek(file, offset, SEEK_SET);
	fread(&next_cluster, sizeof(unsigned int), 1, file);

	return next_cluster;
}
/*-------------------------------------------*/
/*
 *
 *
/*-------------------------------------------*/
unsigned int return_clus_dir_entry(unsigned int cluster, char *name){
	int i = 0;
	char file_name[12];
	long offset;
	struct DIR DIR_entry;

	for(;;){
		offset = return_sector_offset(first_sector_cluster(cluster));
		fseek(file, offset, SEEK_SET);
		long temp = return_sector_offset(first_sector_cluster(cluster)) + bpb_32.BPB_BytsPerSec * bpb_32.BPB_SecPerClus;

		while ( temp >= offset ){
			fread(&DIR_entry, sizeof(struct DIR), 1, file);
			offset+=32;

			if (DIR_entry.DIR_Name[0] == ENTRY_EMPTY)
				continue;
			else if (DIR_entry.DIR_Name[0] == ENTRY_LAST)
				break;
			else if (DIR_entry.DIR_Name[0] == 0x05)
				DIR_entry.DIR_Name[0] = ENTRY_EMPTY;

			if (DIR_entry.DIR_Attr != ATTRIBUTE_NAME_LONG){

				for (i; i < 11; i++)
					file_name[i] = DIR_entry.DIR_Name[i];

				file_name[11] = '\0';

				if (strcmp(file_name, name) == 0)
					return (DIR_entry.DIR_FstClusHI << 16 | DIR_entry.DIR_FstClusLO);
			}
		}

		cluster = FAT_32(cluster);

		if (END_OF_CLUSTER < cluster)
			break;
	}
}
/*-------------------------------------------*/
/*
 *
 *
/*-------------------------------------------*/
long return_sector_offset(long sec){
	return (bpb_32.BPB_BytsPerSec * sec);
}
/*-------------------------------------------*/
/*
 *
 *
/*-------------------------------------------*/
long first_sector_cluster(unsigned int cluster){
	return ( (cluster - 2) * bpb_32.BPB_SecPerClus + bpb_32.BPB_RsvdSecCnt + bpb_32.BPB_FATSz32 * 2);
}
/*-------------------------------------------*/
/*
 *
 *
/*-------------------------------------------*/
long find_cluster_empty_entry(unsigned int cluster){
	int i;
	unsigned int next_cluster;
	long offset;
	struct FSI BPB_FSI_info;
	struct DIR DIR_entry;

	for(;;){
		offset=return_sector_offset(first_sector_cluster(cluster));
		fseek(file, offset, SEEK_SET);
		long temp = return_sector_offset(first_sector_cluster(cluster)) + bpb_32.BPB_BytsPerSec * bpb_32.BPB_SecPerClus;

		while ( temp >= offset ){
			fread(&DIR_entry, sizeof(struct DIR), 1, file);

			if ((DIR_entry.DIR_Name[0] != ENTRY_EMPTY) && (DIR_entry.DIR_Name[0] != ENTRY_LAST))
				break;
			else
				return offset;

			offset += OFFSET_CONST;
		}

		if (END_OF_CLUSTER <= FAT_32(cluster)){
			fseek(file, bpb_32.BPB_FSI_info * bpb_32.BPB_BytsPerSec, SEEK_SET);
			fread(&BPB_FSI_info, sizeof(struct FSI), 1, file);
			
			if (BPB_FSI_info.FSI_Nxt_Free != CLUSTER_END)
				next_cluster = BPB_FSI_info.FSI_Nxt_Free + 1;
				
			else
				next_cluster = 0x00000002;

			for ( ; ; next_cluster++){
				if (FAT_32(next_cluster) == 0x00000000){
					change_val_cluster(next_cluster, cluster);
					change_val_cluster(END_OF_CLUSTER, next_cluster);
					BPB_FSI_info.FSI_Nxt_Free=cluster;
					fseek(file, bpb_32.BPB_FSI_info*bpb_32.BPB_BytsPerSec, SEEK_SET);
					fwrite(&BPB_FSI_info, sizeof(struct FSI), 1, file);
					fflush(file);
					break;
				}
				if (next_cluster == CLUSTER_END)
					next_cluster = 0x00000001;
			}
			cluster = next_cluster;
		}
		else
			cluster = FAT_32(cluster);
	}	
}
/*-------------------------------------------*/
/*
 *
 *
/*-------------------------------------------*/
long return_entry_offset(unsigned int cluster, char *name){	
	int i;
	char file_name[12];
	long offset;
	struct DIR DIR_entry;

	for(;;){
		offset = return_sector_offset(first_sector_cluster(cluster));
		fseek(file, offset, SEEK_SET);
		long temp = return_sector_offset(first_sector_cluster(cluster)) + bpb_32.BPB_BytsPerSec * bpb_32.BPB_SecPerClus;

		while ( temp >= offset ){
			fread(&DIR_entry, sizeof(struct DIR), 1, file);

			if (DIR_entry.DIR_Name[0] == 0x05)
				DIR_entry.DIR_Name[0] = ENTRY_EMPTY;

			if (DIR_entry.DIR_Attr != ATTRIBUTE_NAME_LONG){

				for (i=0; i<11; i++)
					file_name[i]=DIR_entry.DIR_Name[i];

				file_name[11] = '\0';

				if (strcmp(file_name, name) == 0)
					return offset;
			}

			offset += OFFSET_CONST;
		}
		cluster = FAT_32(cluster);

		if (END_OF_CLUSTER < cluster)
			break;
	}
}
/*-------------------------------------------*/
/*
 *
 *
/*-------------------------------------------*/
long return_path_cluster(char *string){
	int i = 1;
	int j = 0;
	long cluster;
	unsigned char name[11];
	cluster = bpb_32.BPB_RootClus;

	while(1){
		for (i; ; i++){

			if (string[i] != '/' && string[i] != '\0'){
				name[j] = string[i];
			}
			else{
				name[j] = '\0';
				break;
			}		
		}

		if (strcmp(name, "") != 0)
			cluster = return_clus_dir_entry(cluster, name);
		else
			break;
	}
	return cluster;
}
