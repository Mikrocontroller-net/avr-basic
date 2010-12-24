//
#include "sd_card/fat.h"
#include <string.h>
//************************************************************************
uint8_t find_file_in_dir(struct fat_fs_struct* fs, struct fat_dir_struct* dd, const char* name, struct fat_dir_entry_struct* dir_entry)	{
   	while(fat_read_dir(dd, dir_entry)) {
   	 	if(strcmp(dir_entry->long_name, name) == 0) {
           	fat_reset_dir(dd);
           	return 1;
       	}
   	}
   	return 0;
}

//************************************************************************
struct fat_file_struct* open_file_in_dir(struct fat_fs_struct* fs, struct fat_dir_struct* dd, const char* name) {
   	struct fat_dir_entry_struct file_entry;
   	if(!find_file_in_dir(fs, dd, name, &file_entry)) return 0;
    return fat_open_file(fs, &file_entry);
}			
