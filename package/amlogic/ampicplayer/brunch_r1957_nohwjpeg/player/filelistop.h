#ifndef FILE_LIST_OP_H__
#define FILE_LIST_OP_H__
typedef struct s_filelist_t {
	char* name;
	struct s_filelist_t *pre,*next;
} filelist_t;

#endif /* FILE_LIST_OP_H__ */

extern filelist_t* create_filelist(char* folder);
extern void free_filelist(filelist_t* filelist);
extern int enum_file(filelist_t* filelist,filelist_t** ppnext,char **file_name);
extern char* dump_filelist(filelist_t* filelist);
