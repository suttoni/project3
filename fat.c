/*-------------------------------------------*/
/*
 *
 *
/*-------------------------------------------*/
#include "fatStructs.h"
#include "fatUtils.h"
#define ATTR_CONST 0x20

/*-------------------------------------------*/
/*
 *
 *
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
 *
 *
/*-------------------------------------------*/
 int cd(char *name);
 int rm(char *name);
 void openfile(char *name, char *mode);
 void close_file(char *name);


/*-------------------------------------------*/
/*
 *
 *
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

	while(i < 100){
		openedFile[i]=0;
		openedReadFile[i] = 0;
		openedWriteFile[i] = 0;
		++i;
	}
	
	if (argc == 2){
		if (file = fopen(argv[1], "rb+")){
			fread(&bpb_32, sizeof(struct BPB_32), 1, file);

			fatImgName = argv[1];
			currCluster = bpb_32.BPB_RootClus;

			workingDir[0] = '/';
			workingDir[1] = '\0';		
			parentDir[0] = '\0';
			parentCluster = -1;
			
			for(;;){
				printf("%s:%s>", fatImgName, workingDir);
				scanf("%s", operation);

				if (strcmp(operation, "info") == 0)
					info();

				else if (strcmp(operation, "ls") == 0)
					ls();

				else if (strcmp(operation, "cd") == 0){
					scanf("%s", name);
					getchar();
					cd(name);
				}
				else if (strcmp(operation, "touch") == 0){
					scanf("%s", name);
					getchar();
					touch(name);
				}

				else if (strcmp(operation, "fopen") == 0){
					scanf("%s", name);
					scanf("%s", mode);
					getchar();
					openfile(name, mode);
				}

				else if (strcmp(operation, "fclose") == 0){
					scanf("%s", name);
					getchar();
					close_file(name);
				}
				/*else if (strcmp(operation, "fread")==0)
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
					printf("Incorrect arguments! Need <info, ls, cd, touch, fopen, fclose, fread, fwrite, rm, mkdir, rmdir, exit>\n");			
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
 *
 *
/*-------------------------------------------*/
int cd(char *name){
	int i = 0;
	int j = 0;
	char fileName[12];
	struct DIR DIR_entry;

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
}
/*-------------------------------------------*/
/*
 *
 *
/*-------------------------------------------*/
void close_file(char *name){
	int i = 0;
	int j = 0;
	long offset;
	char fileName[12];
	struct DIR DIR_entry;
	
	while (name[i] != '\0') {
		if (name[i] < 'a' || name[i] > 'z')
			break;
		else
			name[i] -= OFFSET_CONST;
		++i;
	}

	while (i < 8) {
		if (name[i] == '\0' || name[i] == '.'){
			++i;
			break;
		}
		else{
			fileName[i] = name[i];
			++i;
		}
	}

	for (j = i; j < 8; j++)
		fileName[j] = ' ';

	if (name[i] == '.') {
		i++;
		j=8;

		while (j < 11){
			if (name[i] != '\0')
				fileName[j] = name[i];
			else
				break;
			++i;
			++j;
		}

		while (j < 12){
			fileName[j] = ' ';
			++j;
		}
	}

	else {
		while (i < 11){
			fileName[i] = ' ';
			++i;
		}
	}

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

				for (j = i; j < openedFileNum  -1; j++)
					openedFile[j]=openedFile[j+1];

				openedFile[j]=0;
				openedFileNum--;

				while( i < readFileNum){
					if (openedReadFile[i] == offset){
						break; 
					}
					++i;
				}

				for (j = i; j < readFileNum-1; j++)
                    openedReadFile[j]=openedReadFile[j+1];

                openedReadFile[j] = 0;
                readFileNum--;

				while (i < writeFileNum){
                    if (openedWriteFile[i] == offset){
                        break;
                    }
                    ++i;
                }

                for (j = i; j < writeFileNum-1; j++)
                    openedWriteFile[j]=openedWriteFile[j+1];

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
}
/*-------------------------------------------*/
/*
 *
 *
/*-------------------------------------------*/
void openfile(char *name, char *mode){
	int i = 0;
	int j = 0;
	long offset;
	char fileName[12];
	struct DIR DIR_entry;
	
	while (name[i] != '\0') {
		if (name[i] < 'a' || name[i] > 'z')
			break;
		else
			name[i] -= OFFSET_CONST;
		++i;
	}

	while (i < 8) {
		if (name[i] == '\0' || name[i] == '.'){
			++i;
			break;
		}
		else{
			fileName[i] = name[i];
			++i;
		}
	}

	for (j = i; j < 8; j++)
		fileName[j] = ' ';

	if (name[i] == '.') {
		i++;
		j=8;

		while (j < 11){
			if (name[i] != '\0')
				fileName[j] = name[i];
			else
				break;
			++i;
			++j;
		}

		while (j < 12){
			fileName[j] = ' ';
			++j;
		}
	}

	else {
		while (i < 11){
			fileName[i] = ' ';
			++i;
		}
	}

	fileName[11]='\0';
	
	DIR_entry=find_dir_file_entry(currCluster, fileName);

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
					printf("File has been for reading.\n");
				}

				else if(mode == "w"){
					openedWriteFile[writeFileNum] = offset;
					writeFileNum++;
					printf("File has been for writing.\n");
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
}
/*-------------------------------------------*/
/*
 *
 *
/*-------------------------------------------*/
int rm (char *name){
	int i, j;
	long offset;
	unsigned int next_cluster;
	char fileName[12];
	char empty[32];
	struct DIR DIR_empty_entry, DIR_entry;
	struct FSI FSInfo;
	
	while (name[i] != '\0') {
		if (name[i] < 'a' || name[i] > 'z')
			break;
		else
			name[i] -= OFFSET_CONST;
		++i;
	}

	while (i < 8) {
		if (name[i] == '\0' || name[i] == '.'){
			++i;
			break;
		}
		else{
			fileName[i] = name[i];
			++i;
		}
	}

	for (j = i; j < 8; j++)
		fileName[j] = ' ';

	if (name[i] == '.') {
		i++;
		j=8;

		while (j < 11){
			if (name[i] != '\0')
				fileName[j] = name[i];
			else
				break;
			++i;
			++j;
		}

		while (j < 12){
			fileName[j] = ' ';
			++j;
		}
	}

	else {
		while (i < 11){
			fileName[i] = ' ';
			++i;
		}
	}

	fileName[11] = '\0';
	
	while(i < 32){
		empty[i] = '\0';
		++i;
	}
	
	DIR_entry = find_dir_file_entry(currCluster, fileName);

	if (DIR_entry.DIR_Name[0] != ENTRY_LAST){
		if (DIR_entry.DIR_Attr == ATTR_CONST){
			offset = return_entry_offset(currCluster, fileName);
			if (!unopened(offset))
				printf("Err: already opened!\n");

			else {
				next_cluster=(DIR_entry.DIR_FstClusHI << 16 | DIR_entry.DIR_FstClusLO);
				EmptyValueOfCluster(next_cluster);
				fseek(file, offset, SEEK_SET);
				fwrite(&empty, OFFSET_CONST, 1, file);
			}
		}
		else
			printf("Err: not a file!\n");
	}
	else
		printf("Err: no such entry!\n");
}
