//#define DEBUG_API
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#ifndef DEBUG_API
#include <config.h>
#endif

#include "filelistop.h"

filelist_t* create_filelist(char* folder) {
	filelist_t* head=NULL;
	filelist_t *cur,*t;
	
	struct dirent* ent = NULL; 
	DIR *pDir;
	t=malloc(sizeof(filelist_t));
	if(!t) {
#		ifdef DEBUG
		printf("memory alloc error!\n");
#		endif
		return head;
	}
	t->name=NULL;
	t->pre=t;
	t->next=t;
	head=t;
	cur=head;
	pDir=opendir(folder);
	int temp_len;
	while (NULL != (ent=readdir(pDir))) {
#	ifndef WIN32
		if (ent->d_reclen==24) {  /* not subdirectory. */
		if (ent->d_type==8) {
#	else	
		if(strcmp(ent->d_name,".")&&strcmp(ent->d_name,"..")) {
			if(1) {
#	endif
				t=malloc(sizeof(filelist_t));
				if(t) {
					t->name=malloc(strlen(ent->d_name)+strlen(folder)+2);
					if(!t->name) {
						free(t);
#						ifdef DEBUG
						printf("memory alloc error!\n");
#						endif
						return head;
					}
					strcpy(t->name,folder);
					temp_len=strlen(folder)-1;
					if(t->name[temp_len]!='/') {
						t->name[temp_len+1]='/';
						t->name[temp_len]=0;
					}
					strcat(t->name,ent->d_name);
					t->pre=cur;
					cur->next=t;
					head->pre=t;
					t->next=head;
					cur=t;
				} else {
#					ifdef DEBUG
					printf("memory alloc error!\n");
#					endif
					return head;		
				}
				//printf("%s\n", ent->d_name);
			} 
			/* does not think about sub directories util have more time. */
		}
	}

	closedir(pDir);
	return head;
}

void free_filelist(filelist_t* filelist) {
	filelist_t *head,*cur,*pre;
	if(filelist) {
		head=filelist;
		cur=filelist;
		while(cur->next!=head) {
			pre=cur;
			cur=cur->next;
			if(pre->name) free(pre->name);
			free(pre);
		}
		free(filelist);
	}
}

int enum_file(filelist_t* filelist,filelist_t** ppnext,char **file_name) {
	if(!filelist||filelist->next==filelist) {
		*file_name=NULL;
		*ppnext=NULL;
		return -1;
	}
	if(!(*ppnext)) *ppnext=filelist->next;
	*file_name=(*ppnext)->name;
	if((*ppnext)!=filelist) *ppnext=(*ppnext)->next;
	else {
		*ppnext=NULL;
		return -1;
	}
	
	return 0;
}

char* dump_filelist(filelist_t* filelist) {
	filelist_t *pre=NULL;
	char* file_name;
	if(filelist) {
		while(!enum_file(filelist,&pre,&file_name)) {
			printf("%s\n",file_name);
		}
	}
}

#ifdef DEBUG_API
int main(int argc,char* argv[]) {
	filelist_t *files,*pre;
	char* name;
	if(argc<=1) {
		printf(" 2 args reqired.\n");
		exit(1);
	}
	files=create_filelist(argv[1]);
	if(files) {
		dump_filelist(files);
	}
	free_filelist(files);
}
#endif
