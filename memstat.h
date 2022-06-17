#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
struct me_memory{
	size_t mTotal;
	size_t mFree;
	size_t mAvailable;
	size_t mBuffers;
	size_t mCached;
	size_t mSwapCached;
	size_t mActive;
	size_t mInactive;
	size_t mlocked;};
void outmeminfo(struct me_memory* mem){
	printf("MemTotal: %ld Mb\n"
			"MemFree: %ld Mb\n"
			"MemAvailable: %ld Mb\n"
			"Buffers: %ld Mb\n"
			"Cached: %ld Mb\n"
			"SwapCached: %ld Mb\n"
			"Active: %ld Mb\n"
			"Inactive: %ld Mb\n"
			"Mlocked: %ld Mb\n",mem->mTotal/1024,mem->mFree/1024,mem->mAvailable/1024,mem->mBuffers/1024,mem->mCached/1024,mem->mSwapCached/1024,mem->mActive/1024,mem->mInactive/1024,mem->mlocked/1024);
	}
void setstructmem(struct me_memory* a,char* row,long int* value){
	switch((int)*row){
		case 0:a->mTotal=(size_t)*value;
		case 1:a->mFree=(size_t)*value;
		case 2:a->mAvailable=(size_t)*value;
		case 3:a->mBuffers=(size_t)*value;
		case 4:a->mCached=(size_t)*value;
		case 5:a->mSwapCached=(size_t)*value;
		case 6:a->mActive=(size_t)*value;
		case 7:a->mInactive=(size_t)*value;
		case 12:a->mlocked=(size_t)*value;
		}}
int getmeminfo(struct me_memory* a){
	FILE *filestat;
	filestat=fopen("/proc/meminfo","r");
	if(filestat==NULL){printf("File not open abort");return 1;}
	char temp;
	char rows=0;
	char buf[32];
	int i=0;
	long int temp_val=0;
	while((temp=fgetc(filestat))!=EOF){
		if((char)temp=='\n'){
			buf[i]='\0';
			temp_val=atol(buf);;
			setstructmem(a,&rows,&temp_val);
			i=0;rows+=1;}
		if(!isdigit(temp)){continue;}
		else {buf[i]=temp;i+=1;}}
	fclose(filestat);
	return 0;}
/*int main(int argc, char **argv){
	struct me_memory mem;
	getmeminfo(&mem);
	outmeminfo(&mem);
	return 0;}

*/
