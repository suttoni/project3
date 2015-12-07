/*-------------------------------------------*/
/*
 * FAT_32 Filesystem - Project 3
 * Team Satisfries - Ian Sutton, Ibrahim Atiya, Sai Gunasegaran, Yilin Wang
 *
/*-------------------------------------------*/
#include "fatStructs.h"
#include "fatUtils.h"

#define ATTR_CONST 0x20

/*-------------------------------------------*/
/*
 * Setting up our variables to be used through  
 * the program
/*-------------------------------------------*/
char *fatImgName;
char *parentDir;
char *workingDir;

int openedFileNum;
int readFileNum;
int writeFileNum;

unsigned int parentCluster;
unsigned int currCluster;

long openedReadFile[100];
long openedWriteFile[100];
long openedFile[100];

struct BPB_32 bpb_32;
FILE *file;

/*-------------------------------------------*/
/*
 * Forward function declarations
 *
/*-------------------------------------------*/
 int cd(char *name);
 int rm(char *name);
 int info();
 int ls(char *name);
 int create(char *name);
 void open(char *name, char *mode);
 void close(char *name);


/*-------------------------------------------*/
/*
 * The main function - this is where the magic happens
 * Calls the necessary functions depending on the input
 * from the user
/*-------------------------------------------*/
int main(int argc, char *argv[]){
	int i = 0;
	char mode[1];
	char name[13];
	char operation[6];

	writeFileNum = 0;
	readFileNum = 0;
	openedFileNum = 0;
	workingDir = (char*)malloc(200*sizeof(char));
	parentDir = (char*)malloc(200*sizeof(char));

	// Initialize file arrays by setting their contents to 0
	while(i < 100){
		openedFile[i] = 0;
		openedReadFile[i] = 0;
		openedWriteFile[i] = 0;
		++i;
	}
	
	// Begin usage
	if (argc == 2){
		if (file = fopen(argv[1], "rb+")){
			fread(&bpb_32, sizeof(struct BPB_32), 1, file);

			fatImgName = argv[1];
			currCluster = bpb_32.BPB_RootClus;

			workingDir[0] = '/';
			workingDir[1] = '\0';		
			parentDir[0] = '\0';
			parentCluster = -1;
			
			// Begin infinite loop and gather information from user
			for(;;){
				printf("%s:%s>", fatImgName, workingDir);
				scanf("%s", operation);

				if (strcmp(operation, "info") == 0)
					info();

				else if (strcmp(operation, "ls") == 0){
					scanf("%s", name);
					getchar();
					ls(name);
				}
				else if (strcmp(operation, "cd") == 0){
					scanf("%s", name);
					getchar();
					cd(name);
				}
				else if (strcmp(operation, "create") == 0){
					scanf("%s", name);
					getchar();
					create(name);
				}
				else if (strcmp(operation, "fopen") == 0){
					scanf("%s", name);
					scanf("%s", mode);
					getchar();
					open(name, mode);
				}
				else if (strcmp(operation, "fclose") == 0){
					scanf("%s", name);
					getchar();
					close(name);
				}/*
				else if (strcmp(operation, "fread")==0)
					readfile();
				else if (strcmp(operation, "fwrite")==0)
					writefile();*/
				else if (strcmp(operation, "rm") == 0){
					scanf("%s", name);
					getchar();
					rm(name);
				}
				else if (strcmp(operation, "mkdir") == 0){
					scanf("%s", name);
					getchar();
					mkdir(name);
				}
				else if (strcmp(operation, "rmdir") == 0){
					scanf("%s", name);
					getchar();
					rmdir(name);
				}
				else if (strcmp(operation, "exit") == 0){
					fclose(file);
					break;
				}
				else
					printf("Incorrect arguments! Need <info, ls, cd, create, fopen, fclose, fread, fwrite, rm, mkdir, rmdir, exit>\n");			
			}
			return 0;
		}
		else{
			printf("Could not find FAT_32 image!\n");
			return -1;
		}
	}
	else{
		printf("Incorrect number of arguments!\n");
		return -1;
	}
}
/*-------------------------------------------*/
/*
 * Changes to the directory given through name
 *
/*-------------------------------------------*/
int cd(char *name){
	int i = 0;
	int j = 0;
	char fileName[12];
	struct DIR DIR_entry;

	// Set up the fileName 
	while (name[i] != '\0'){
		if (name[i] >= 'a' && name[i] <= 'z')
			name[i] -= OFFSET_CONST;

		fileName[i] = name[i];
		++i;
	}

	while (i < 11){
		fileName[i] = ' ';
		++i;
	}

	fileName[i] = '\0';
	
	if (strcmp(name, ".") == 0){
		//What do we do if we cd into the root directory?		
	}

	else if (strcmp(name, "/") == 0){
		workingDir[0] = '/';
		workingDir[1] = '\0';
		currCluster = bpb_32.BPB_RootClus;
	}

	else if (strcmp(name, "..") == 0){
		if (strcmp(workingDir, "/") == 0){
			printf("Error! It's in the root directory.\n");
			return -1;
		}

		else{	
			while (workingDir[i] != '\0')
				++i;
			while ((i = i -1) && workingDir[i] != '/')
				--i;
			
			if (i == 0)
				workingDir[i + 1] = '\0';
			else
				workingDir[i] = '\0';

			currCluster = return_path_cluster(workingDir);
		}
	}
	else{
		// Get the directory entry
		DIR_entry = find_dir_file_entry(currCluster, fileName);

		if (DIR_entry.DIR_Name[0] == ENTRY_LAST)
			printf("Err: No such directory found!\n");
		else {
			if (DIR_entry.DIR_Attr == 0x10){
				currCluster = return_clus_dir_entry(currCluster, fileName);

				while (workingDir[i] != '\0')
					++i;

				if (workingDir[i - 1] != '/')
					workingDir[i++] = '/';

				while (name[j] != '\0'){
					workingDir[i] = name[j];
					++i;
					++j;
				}

				workingDir[i] = '\0';
			}
			else
				printf("Err: That is not a directory!\n");
		}
	}
	return 0;
}// End cd
/*-------------------------------------------*/
/*
 * Removes a file given through the 
 * char *name parameter in main
/*-------------------------------------------*/
int rm (char *name){
	int i = 0;
	int temp;
	long offset;
	unsigned int nextCluster;
	char fileName[12];
	char empty[32];
	struct DIR DIR_entry;
	
	/* The following series of loops set up the file name by accessing indices in name */
	// Prep file name before creating it
	while (name[i] != '\0') {
		if (name[i] < 'a' || name[i] > 'z')
			break;
		else
			name[i] -= OFFSET_CONST;
		++i;
	}

	// Read in name portion of file into fileName
	while (i < 8) {
		if (name[i] != '\0' && name[i] != '.'){
			fileName[i] = name[i];
			++i;
		}
		else{
			temp = i;
			break;
		}
	}

	// Fill up the rest of fileName with spaces
	for (i = temp; i < 8; i++)
		fileName[i] = ' ';

	// Accounting for extensions
	if (name[temp++] == '.') {
		i = 8;

		while (i < 11){
			if (name[temp] != '\0')
				fileName[i] = name[temp++];
			else{
				temp = i;
				break;
			}

			if (i == 10)
				temp = ++i;

			++i;
		}

		while (temp < 11){
			fileName[temp] = ' ';
			++temp;
		}
	}

	else {
		while (temp < 11){
			fileName[temp] = ' ';
			++temp;
		}
	}

	/* Set the end of fileName to null character */
	fileName[11] = '\0';

	i = 0;
	while(i < 32){
		empty[i] = '\0';
		++i;
	}
	
	// Get the directory entry
	DIR_entry = find_dir_file_entry(currCluster, fileName);
	offset = return_entry_offset(currCluster, fileName);

	if (DIR_entry.DIR_Name[0] != 0){
		if (DIR_entry.DIR_Attr == ATTR_CONST){
			if (!unopened(offset))
				printf("Err: already opened!\n");

			else {
				nextCluster = (DIR_entry.DIR_FstClusHI << 16 | DIR_entry.DIR_FstClusLO);
				empty_val_cluster(nextCluster);
				fseek(file, offset, SEEK_SET);
				fwrite(&empty, OFFSET_CONST, 1, file);
				return 0xFFF0;
			}
		}
		else{
			printf("Err: not a file!\n");
			return 0xFFFE;
		}
	}
	else{
		printf("Err: no such entry!\n");
		return 0xFFFE;
	}
}// End rm
/*-------------------------------------------*/
/*
 * A simple function meant to provide the user with
 * information regarding the file system
 *
/*-------------------------------------------*/
int info(){
	long offset;
	struct FSI BPB_FSI_info;

	// Get the offset
	offset = bpb_32.BPB_FSI_info * bpb_32.BPB_BytsPerSec;
	fseek(file, offset, SEEK_SET);
	fread(&BPB_FSI_info, sizeof(struct FSI), 1, file);

	// Print the needed information
	printf("Number of free Sectors: %d\n", BPB_FSI_info.FSI_Free_Count);
	printf("Sectors per Cluster: %d\n", bpb_32.BPB_SecPerClus);
	printf("Total Sectors: %d\n", bpb_32.BPB_TotSec32);
	printf("Bytes per Sector: %d\n", bpb_32.BPB_BytsPerSec);
	printf("Sectors per FAT: %d\n", bpb_32.BPB_FATSz32);
	printf("Number of FATs: %d\n", bpb_32.BPB_NumFATs);
	
	return 0;
}// End info
/*-------------------------------------------*/
/*
 * List the contents of directory
 *
/*-------------------------------------------*/
int ls(char *name){
	int i = 0;
	char ext[4];
	unsigned int c;
	long offset;
	struct DIR DIR_entry;
	c = currCluster;
	
	// Begin infinite loop
	for(;;) {
		offset = return_sector_offset(first_sector_cluster(c));
		fseek(file, offset, SEEK_SET);
		
		// Temp variable for comparison to offset
		long temp = return_sector_offset(first_sector_cluster(c)) + bpb_32.BPB_BytsPerSec * bpb_32.BPB_SecPerClus;
		while (temp >= offset){

			fread(&DIR_entry, sizeof(struct DIR), 1, file);
			offset += OFFSET_CONST;
			
			if (DIR_entry.DIR_Name[0] == 0)
				continue;

			else if (DIR_entry.DIR_Name[0] == 0xE5)
				break;

			else if (DIR_entry.DIR_Name[0] == 0x5)
				DIR_entry.DIR_Name[0] = 0xE5;

			if (DIR_entry.DIR_Attr != ATTRIBUTE_NAME_LONG){
				while (i < 8){

					if (DIR_entry.DIR_Name[i] != ' ')
						name[i] = DIR_entry.DIR_Name[i];

					else{
						name[i] = '\0';
						break;
					}
					++i;
				}

				name[8] = '\0';

				while (i < 3){
					if (DIR_entry.DIR_Name[i+8] != ' ')
						ext[i] = DIR_entry.DIR_Name[i+8];

					else{
						ext[i] = '\0';
						break;
					}
					++i;
				}

				ext[3] = '\0';

				if (strcmp(name, ".") != 0 && strcmp(name, "..") != 0){
					if (ext[0] != '\0')
						printf("%s.%s\n", name, ext);
					else
						printf("%s\n", name);
				}
			}
		}
		c = FAT_32(c);
		if (c >= 0x0FFFFFF8)
			break;
	}
	return 0;
}// End ls
/*-------------------------------------------*/
/*
 * Creates a file with the parameter char *name
 * provided by the user in main
/*-------------------------------------------*/
int create (char *name){
	int i = 0;
	int temp;
	char fileName[12];
	unsigned int newCluster;
	long offset;
	struct DIR emptyEntry, DIR_entry;
	struct FSI BPB_FSI_info;


	/* The following series of loops set up the file name by accessing indices in name */
	// Prep file name before creating it
	while (name[i] != '\0') {
		if (name[i] < 'a' || name[i] > 'z')
			break;
		else
			name[i] -= OFFSET_CONST;
		++i;
	}

	// Read in name portion of file into fileName
	while (i < 8) {
		if (name[i] != '\0' && name[i] != '.'){
			fileName[i] = name[i];
			++i;
		}
		else{
			temp = i;
			break;
		}
	}

	// Fill up the rest of fileName with spaces
	for (i = temp; i < 8; i++)
		fileName[i] = ' ';

	// Accounting for extensions
	if (name[temp++] == '.') {
		i = 8;

		while (i < 11){
			if (name[temp] != '\0')
				fileName[i] = name[temp++];
			else{
				temp = i;
				break;
			}

			if (i == 10)
				temp = ++i;

			++i;
		}

		while (temp < 11){
			fileName[temp] = ' ';
			++temp;
		}
	}

	else {
		while (temp < 11){
			fileName[temp] = ' ';
			++temp;
		}
	}

	/* Set the end of fileName to null character */
	fileName[11] = '\0';
	
	// Get the directory entry
	DIR_entry = find_dir_file_entry(currCluster, fileName);
	if (DIR_entry.DIR_Name[0] == 0){
		offset = find_cluster_empty_entry(currCluster);

		while (i < 11){
			emptyEntry.DIR_Name[i] = fileName[i];
			++i;
		}

		// Set up the directory 
		emptyEntry.DIR_Attr = ATTR_CONST;
		emptyEntry.DIR_NTRes = ENTRY_LAST;
		emptyEntry.DIR_FileSize = CLUSTER_END;
		
		fseek(file, bpb_32.BPB_FSI_info * bpb_32.BPB_BytsPerSec, SEEK_SET);
		fread(&BPB_FSI_info, sizeof(struct FSI), 1, file);
		
		if (BPB_FSI_info.FSI_Nxt_Free == 0xFFFFFFFF)
			newCluster = 2;
		else
			newCluster = BPB_FSI_info.FSI_Nxt_Free + 1;

		for (;;){
			if (FAT_32(newCluster) == 0){
				emptyEntry.DIR_FstClusHI = (newCluster >> 16);
				emptyEntry.DIR_FstClusLO = (newCluster & 0xFFFF);
				change_val_cluster(0x0FFFFFF8, newCluster);
				BPB_FSI_info.FSI_Nxt_Free = newCluster;

				fseek(file, bpb_32.BPB_FSI_info * bpb_32.BPB_BytsPerSec, SEEK_SET);
				fwrite(&BPB_FSI_info, sizeof(struct FSI), 1, file);
				fflush(file);
				break;
			}
			if (newCluster == 0xFFFFFFFF)
				newCluster = 1;
		}

		fseek(file, offset, SEEK_SET);
		fwrite(&emptyEntry, sizeof(struct DIR), 1, file);
		fflush(file);
		return 0xFFF0;
	}
	else{
		printf("Error! Entry already exists.\n");
		return 0xFFFE;
	}
}// End create
/*-------------------------------------------*/
/*
 * Opens a file name with the mode given by
 * user in main
/*-------------------------------------------*/
void open(char *name, char *mode){
	int i = 0;
	int temp;
	long offset;
	char fileName[12];
	struct DIR DIR_entry;
	
	/* The following series of loops set up the file name by accessing indices in name */
	// Prep file name before creating it
	while (name[i] != '\0') {
		if (name[i] < 'a' || name[i] > 'z')
			break;
		else
			name[i] -= OFFSET_CONST;
		++i;
	}

	// Read in name portion of file into fileName
	while (i < 8) {
		if (name[i] != '\0' && name[i] != '.'){
			fileName[i] = name[i];
			++i;
		}
		else{
			temp = i;
			break;
		}
	}

	// Fill up the rest of fileName with spaces
	for (i = temp; i < 8; i++)
		fileName[i] = ' ';

	// Accounting for extensions
	if (name[temp++] == '.') {
		i = 8;

		while (i < 11){
			if (name[temp] != '\0')
				fileName[i] = name[temp++];
			else{
				temp = i;
				break;
			}

			if (i == 10)
				temp = ++i;

			++i;
		}

		while (temp < 11){
			fileName[temp] = ' ';
			++temp;
		}
	}

	else {
		while (temp < 11){
			fileName[temp] = ' ';
			++temp;
		}
	}

	/* Set the end of fileName to null character */
	fileName[11] = '\0';
	
	// Get the directory entry
	DIR_entry = find_dir_file_entry(currCluster, fileName);

	if (DIR_entry.DIR_Name[0] != ENTRY_LAST){
		if (DIR_entry.DIR_Attr == ATTR_CONST){
			offset = return_entry_offset(currCluster, fileName);

			if (!unopened(offset))
				printf("Err: already opened!\n");

			else {
				openedFile[openedFileNum]=offset;
				openedFileNum++;

				if(mode == "r"){
					openedReadFile[readFileNum] = offset;
					readFileNum++;
					printf("File opened for reading.\n");
				}

				else if(mode == "w"){
					openedWriteFile[writeFileNum] = offset;
					writeFileNum++;
					printf("File opened for writing.\n");
				}

				else if(mode == "x"){
					openedWriteFile[writeFileNum] = offset;
                    writeFileNum++;
					openedReadFile[readFileNum] = offset;
                    readFileNum++;
                    printf("File has been opened for reading and writing.\n");
				}
				printf("Err: Invalid mode!\n");
			}
		}
		else
			printf("Err: no such file!\n");
	}
	else
		printf("Err: no such entry!\n");
}// End open
/*-------------------------------------------*/
/*
 * Closes a file. Return an error if the file is already closed
 *
/*-------------------------------------------*/
void close(char *name){
	int i = 0;
	int temp;
	long offset;
	char fileName[12];
	struct DIR DIR_entry;

	/* The following series of loops set up the file name by accessing indices in name */
	// Prep file name before creating it
	while (name[i] != '\0') {
		if (name[i] < 'a' || name[i] > 'z')
			break;
		else
			name[i] -= OFFSET_CONST;
		++i;
	}

	// Read in name portion of file into fileName
	while (i < 8) {
		if (name[i] != '\0' && name[i] != '.'){
			fileName[i] = name[i];
			++i;
		}
		else{
			temp = i;
			break;
		}
	}

	// Fill up the rest of fileName with spaces
	for (i = temp; i < 8; i++)
		fileName[i] = ' ';

	// Accounting for extensions
	if (name[temp++] == '.') {
		i = 8;

		while (i < 11){
			if (name[temp] != '\0')
				fileName[i] = name[temp++];
			else{
				temp = i;
				break;
			}

			if (i == 10)
				temp = ++i;

			++i;
		}

		while (temp < 11){
			fileName[temp] = ' ';
			++temp;
		}
	}

	else {
		while (temp < 11){
			fileName[temp] = ' ';
			++temp;
		}
	}

	/* Set the end of fileName to null character */
	fileName[11] = '\0';
	
	DIR_entry = find_dir_file_entry(currCluster, fileName);

	if (DIR_entry.DIR_Name[0] != ENTRY_LAST){
		if (DIR_entry.DIR_Attr == ATTR_CONST){
			offset = return_entry_offset(currCluster, fileName);

			if (!unopened(offset)){
				while (i < openedFileNum){
					if (openedFile[i] == offset)
						break;
					++i;
				}	

				for (temp = i; temp < openedFileNum - 1; temp++)
					openedFile[temp] = openedFile[temp+1];

				openedFile[temp] = 0;
				openedFileNum--;

				while( i < readFileNum){
					if (openedReadFile[i] == offset){
						break; 
					}
					++i;
				}

				for (temp = i; temp < readFileNum - 1; temp++)
                    openedReadFile[temp]=openedReadFile[temp + 1];

                openedReadFile[temp] = 0;
                readFileNum--;

				while (i < writeFileNum){
                    if (openedWriteFile[i] == offset){
                        break;
                    }
                    ++i;
                }

                for (temp = i; temp < writeFileNum - 1; temp++)
                    openedWriteFile[temp]=openedWriteFile[temp + 1];

                writeFileNum--;
			}
			else 
				printf("Err: not opened!\n");
		}
		else
			printf("Err: no such file!\n");
	}
	else
		printf("Err: no such entry!\n");
}// End close
