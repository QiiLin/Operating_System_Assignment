#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "ext2_helper.c"


unsigned char *disk;

int main(int argc, char **argv) {
    // check path parameter is the not enough or the input is not absolute path
    if(argc != 4 || argv[3][0] != '/') {
        fprintf(stderr, "Usage: ext2_cp <ext2 image file name> <file path on your OS> <abs file path on image>\n");
        exit(1);
    }
    // read file as raw binary
    FILE * fp = fopen(argv[2], "rb");
    if (fp == NULL) {
        fprintf(stderr, "%s\n","Reading local file %s unsuccessfully" );
        exit(ENOENT);
    }
    // step 1: read the disk
    disk = read_disk(argv[1]);
    // step 2: get info from the file
    struct stat fileInfo;
    if (stat(argv[2], &fileInfo) != 0) {
        fprintf(stderr, "%s\n", "file was not found");
        exit(ENOENT);
    }
    if (!S_ISREG(fileInfo.st_mode)) {
        fprintf(stderr, "%s: wrong file type given\n", argv[2]);
        exit(1);
    }
    // by defn the file name is the name from the input file
    char *file_name = get_file_name(argv[2]);
    // check if the path passed in is a valid asbolute path leading to a directory
    int dir_inode = read_path(disk, argv[3]);
    // init dir_name
    char* new_file_name;
    struct ext2_inode *place_inode;
    // check if the path can be found
    if (dir_inode == -1) {
      char dir_name[EXT2_NAME_LEN];
      // if the target path is looking for directory
      if (argv[3][strlen(argv[3]) - 1] == '/') {
          fprintf(stderr, "%s: No such directory\n", argv[3]);
          exit(ENOENT);
      }
      // this will modified the path to the parent directory
      // and save the name of new file name into the dir_name
      pop_last_file_name(argv[3], dir_name);
      // do another check if the parent directory exist
      dir_inode = read_path(disk, argv[3]);
      if (dir_inode == -1) {
        fprintf(stderr, "%s: No such file or directory\n", argv[3]);
        exit(ENOENT);
      }
      place_inode = get_inode(disk, dir_inode);
      // if it is not a directory
      if (!S_ISDIR(place_inode->i_mode)) {
        fprintf(stderr, "%s: No such directory\n", argv[3]);
        exit(ENOENT);
      }
      new_file_name = dir_name;
    } else {
      new_file_name = file_name;
      place_inode = get_inode(disk, dir_inode);
      if (!S_ISDIR(place_inode->i_mode)) {
        fprintf(stderr, "%s: No such directory\n", argv[3]);
        exit(ENOENT);
      }
    }
    long file_size = fileInfo.st_size;
    // check if the new_file_name name exist
    int valid_filename = check_valid_file(disk, dir_inode, new_file_name);
    if (valid_filename < 0) {
        fprintf(stderr, "A file named %s already exists in the directory",file_name);
        exit(EEXIST);
    }
    // start the operation
    // Step 1: make sure we have enough block and inode to perform this
    // allocate inodes
    int free_inode_index = find_free_inode(disk);
    if (free_inode_index == -1) {
      fprintf(stderr, "%s: No inode avaiable\n", argv[0]);
      exit(1);
    }
    int required_block = get_required_block(file_size);
    int indir_block = 0;
    // check if it need redirect block
    if (required_block > 12) {
      indir_block = 1;
    }
    int* free_blocks = find_free_blocks(disk, required_block + indir_block);
    if (free_blocks[0] == -1) {
      free(free_blocks);
      fprintf(stderr, "%s: No blocks avaiable\n", argv[0]);
      exit(1);
    }
    set_bitmap(0, disk, free_inode_index, 1);
    for (int i = 0; i < required_block + indir_block; i++) {
      set_bitmap(1, disk, free_blocks[i], 1);
    }
    // add dir_entry for the new inode
    int result = add_link_to_dir(place_inode, disk, new_file_name, free_inode_index,
        EXT2_FT_REG_FILE);
    if (result == -1) {
      // need to revert the bitmap
      set_bitmap(0, disk, free_inode_index, 0);
      for (int i = 0; i < required_block + indir_block; i++) {
        set_bitmap(1, disk, free_blocks[i], 1);
      }
      free(free_blocks);
      fprintf(stderr, "%s: No blocks avaiable\n", argv[0]);
      exit(1);
    }
    // Step 2: start create inodes and update it property
    struct ext2_inode *file_inode = initialize_inode(disk, free_inode_index,EXT2_S_IFREG, file_size);
    file_inode->i_blocks = (required_block + indir_block) *2;
    // step 3: set file Buffer
    unsigned char tmp_buffer[EXT2_BLOCK_SIZE+1];
    unsigned int * indirect_block;
    // step 4: create block and fill the block in there
    for(int i = 0; i < required_block; i++) {
      if (i < 12) {
        file_inode->i_block[i] = free_blocks[i];
      } else {
        // if it reach here then the indir_block must equal to 1
        // so the  free_blocks[required_block]; exist
        if (i == 12 ) {
          // first time access inderect so we need init the indirect_block
          file_inode->i_block[12] = free_blocks[required_block];
          indirect_block = (unsigned int *)(disk + file_inode->i_block[12] * EXT2_BLOCK_SIZE);
        }
        indirect_block[i-12] = free_blocks[i];
      }
    }
    int block_number;
    // step 5: start memcpy stuff into it
    for(int i = 0; i < required_block; i++) {
      if (i < 12) {
        // direct case
        block_number = file_inode->i_block[i];
      } else {
        // indirect case
        unsigned int *in_dir = (unsigned int *) (disk + EXT2_BLOCK_SIZE * file_inode->i_block[12]);
        block_number = in_dir[i - 12];
      }
      unsigned int *block = (unsigned int *)(disk + block_number * EXT2_BLOCK_SIZE);
      long unsigned int result_count = fread(tmp_buffer, 1,  EXT2_BLOCK_SIZE, fp);
      memcpy(block, tmp_buffer, result_count);
    }
    // update used block and inodes
    struct ext2_super_block *sb = (struct ext2_super_block *)(disk + 1024);
    // get the inode bitmap to manipulate its bits if there are free inode in the bitmap
    struct ext2_group_desc *bgd = (struct ext2_group_desc *) (disk + 2048);
    bgd->bg_free_inodes_count--;
    sb->s_free_inodes_count--;
    bgd->bg_free_blocks_count -= (required_block + indir_block);
    sb->s_free_blocks_count -= (required_block + indir_block);
    return 0;
}
