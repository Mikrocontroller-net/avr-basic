//

// Prototypen
uint8_t find_file_in_dir(struct fat_fs_struct*, struct fat_dir_struct*, const char*, struct fat_dir_entry_struct*);
struct fat_file_struct* open_file_in_dir(struct fat_fs_struct*, struct fat_dir_struct*, const char*);
