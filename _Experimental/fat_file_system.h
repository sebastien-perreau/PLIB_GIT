#ifndef __DEF_FAT_FILE_SYSTEM
#define __DEF_FAT_FILE_SYSTEM

#define FAT32_FILE_SYSTEM_FAT_TABLE_END_OF_CHAIN            0x0ffffff8
#define FAT16_FILE_SYSTEM_FAT_TABLE_END_OF_CHAIN            0xfff8

typedef enum
{
    FAT_FILE_SYSTEME_FA_LFN           = 0x0f,                           // Long File Name attribute (always equal to 0x0f if the 32 bytes entry is a part of a Long File Name)
    FAT_FILE_SYSTEME_FA_READ          = 0x01,
    FAT_FILE_SYSTEME_FA_HIDDEN        = 0x02,
    FAT_FILE_SYSTEME_FA_SYSTEM        = 0x04,
    FAT_FILE_SYSTEME_FA_VOLUME_NAME   = 0x08,
    FAT_FILE_SYSTEME_FA_DIRECTORY     = 0x10,
    FAT_FILE_SYSTEME_FA_ARCHIVE       = 0x20
} FAT_FILE_SYSTEM_FILE_ATTRIBUTES;

typedef enum
{
    FAT_FILE_SYSTEM_DFFC_ENTRY_NOT_USED_AND_NOTHING_AFTER   = 0x00,
    FAT_FILE_SYSTEM_DFFC_ENTRY_NOT_USED                     = 0xe5,
    FAT_FILE_SYSTEM_DFFC_ENTRY_ON_CURRENT_DIRECTORY         = 0x2e        
} FAT_FILE_SYSTEM_DIRECTORY_FILENAME_FIRST_CHARACTER;

typedef enum
{
    FAT_FILE_SYSTEM_FLAG_READ_BLOCK_OP_READ_REQUESTED   = 1,
    FAT_FILE_SYSTEM_FLAG_READ_BLOCK_OP_READ_ON_GOING    = 2,
    FAT_FILE_SYSTEM_FLAG_READ_BLOCK_OP_READ_TERMINATED  = 3
} FAT_FILE_SYSTEM_FLAGS_READ_OP;

typedef union
{
    struct 
    {
        unsigned                        read_only:1;                    // Bit 0: This flag is used to prevent programs from not automatically overwriting or deleting this entry.
        unsigned                        hidden:1;                       // Bit 1: This flag indicates to the system that the file should be hidden when doing normal directory listings. But in a lot of programs this can be overwritten by the user.
        unsigned                        system:1;                       // Bit 2: This flag indicates that the file/directory is important for the system, and shouldn't be manipulated without any concern.
        unsigned                        volume_name:1;                  // Bit 3: When this flag is set, the directory entry is not pointing to a file, but to nothing. Thus the the Starting cluster must point the cluster 0. The only information used from this entry is the filename (8 bytes) plus the filename extension (3 bytes). These bytes form an 11 byte long volume label (without any .) There may be only one valid entry in the entire volume with this flag set. This entry must be in the root directory and preferably among the first entries, if not, then MS-DOS can have trouble displaying the right volume label if there are long file names present. This volume name should be the same as the one in the boot sector.
        unsigned                        directory:1;                    // Bit 4: This flag is set, when an entry in the directory table is not pointing to the beginning of a file, but to another directory table. A sub-directory. The sub-directory is placed in the cluster, which the Starting Cluster field points to. The format of this sub-directory table is identical to the root directory table.
        unsigned                        archive:1;                      // Bit 5: This flag is used by backup utilities. The flag is set when ever a file is created, renamed or changed. Backup utilities can then use this flag to determine which files that has been modified since the last backup.
        unsigned                        :2;                             // Reserved
    };
    struct
    {
        uint8_t                         value;
    };
} fat_file_system_file_attributes_t;

typedef union
{
    struct 
    {
        unsigned                        seconds:5;                      // seconds (0..29 - multiply by 2)
        unsigned                        minutes:6;                      // minutes (0..59)
        unsigned                        hours:5;                        // hours (0..23)
    };
    struct
    {
        uint16_t                        value;                          // hours [15..11] minutes [10..5] seconds [4..0]
    };
} fat_file_system_time_t;

typedef union
{
    struct 
    {
        unsigned                        day:5;                          // day of month (1..31)
        unsigned                        month:4;                        // month of year (1..12)
        unsigned                        year:7;                         // years from 1980 (0..127 -> 1980..2107)
    };
    struct
    {
        uint16_t                        value;                          // year [15..9] month [8..5] day [4..0]
    };
} fat_file_system_date_t;

typedef union
{
    struct
    {
        bool                            is_found;                       // This bit is set if the file is found on the SD Card
        FAT_FILE_SYSTEM_FLAGS_READ_OP   is_read_block_op;               // 0: not used, 1: read_block_requested, 2: read_block_on_going, 3: read_block_terminated
        bool                            is_read_file_stopped;           // 0: play / continue, 1: stop / pause
        bool                            is_read_file_terminated;        // 0: no, 1: yes
        unsigned                        :3;
    };
    struct
    {
        uint8_t                         value;
    };
} fat_file_system_flags_t;

typedef struct
{
    char                                file_name[255];                 // File name and extension (example "My Folder\\my_file.extension")
    fat_file_system_file_attributes_t   file_attributes;                // The attribute byte defines a set of flags which can be set for directories, volume name, hidden files, system files, etc.     
    fat_file_system_time_t              last_write_time;                // The 16 bit time field contain the time of day when this entry was created. hours (0..23) [15..11] minutes (0..59) [10..5] seconds (0..29 - multiply by ) [4..0]
    fat_file_system_date_t              last_write_date;                // The 16 bit date field contain the date of day when this entry was created. year (0..127 -> 1980..2107) [15..9] month (1..12) [8..5] day (1..31) [4..0]
    uint32_t                            first_cluster_of_the_file;      // This 32-bit (16-bit for FAT16 - 32-bit for FAT32) field points to the starting cluster number of entries data. If the entry is a directory this entry point to the cluster which contain the beginning of the sub-directory. If the entry is a file then this entry point to the cluster holding the first chunk of data from the file.
    uint32_t                            file_size;                      // This 32-bit field count the total file size in bytes. For this reason the file system driver must not allow more than 4 GB to be allocated to a file. For other entries than files then file size field should be set to 0.
    
    fat_file_system_flags_t             flags;
    state_machine_t                     sm_read;
    DYNAMIC_TAB_BYTE                    buffer;
    uint32_t                            current_cluster_of_the_file;
    uint32_t                            _data_address;
    uint16_t                            _data_length;
    
    uint32_t                            _current_jump_index;            // This variable is the current "jump index" in the FAT table when using sd_card_read_file_data(...) routine. 
    uint32_t                            _current_fat_sector;            // The current sector where the last cluster in localized in the FAT table when using sd_card_read_file_data(...) routine. 
    uint32_t                            _current_data_sector;           // The current sector where the data is stored when using sd_card_read_file_data(...) routine. 
    uint16_t                            _index_data_in_sector;          // Value between [0..511] when using sd_card_read_file_data(...) routine. 
    
    void                                *p_sd_card;
    
} fat_file_system_entry_t;

#define FAT_FILE_SYSTEM_ENTRY_INSTANCE(_name, _file_name, _p_sd_card)       \
{                                                                           \
    .file_name = _file_name,                                                \
    .file_attributes = 0,                                                   \
    .last_write_time = 0,                                                   \
    .last_write_date = 0,                                                   \
    .first_cluster_of_the_file = 0,                                         \
    .file_size = 0,                                                         \
    .flags = {0},                                                           \
    .sm_read = {0},                                                         \
    .buffer = {0, 0, 0},                                                    \
    .current_cluster_of_the_file = 0,                                       \
    ._data_address = 0,                                                     \
    ._data_length = 0,                                                      \
    ._current_jump_index = 0,                                               \
    ._current_fat_sector = 0,                                               \
    ._current_data_sector = 0,                                              \
    ._index_data_in_sector = 0,                                             \
    .p_sd_card = (void*) &_p_sd_card                                        \
}

#define FILE_DEF(_p_sd_card, _name, _file_name)                             \
static fat_file_system_entry_t _name = FAT_FILE_SYSTEM_ENTRY_INSTANCE(_name, _file_name, _p_sd_card)

typedef union
{
    struct
    {
        uint8_t                         minor;
        uint8_t                         major;
    };
    struct
    {
        uint16_t                        value;
    };
} fat32_file_system_version_t;

typedef union
{
    struct
    {
        unsigned                        active_fat:5;
        unsigned                        :2;
        unsigned                        enable:1;
        unsigned                        :8;
    };
    struct
    {
        uint16_t                        value;
    };
} fat32_file_system_handling_flags_t;

typedef struct
{
    uint32_t                            lead_signature;                             // This field indicates that this sector has FSInfo structure.  This value always should be "0x41615252" and never changed.
    uint32_t                            struct_signature;                           // This field also indicates that this sector has FSInfo structure and always should have "0x61417272".
    uint32_t                            free_cluster_count;                         // This item has the number of free cluster in volume.  If this field is "0xFFFFFFFF", the file system does not count the number of free cluster in volume.
    uint32_t                            next_free_cluster;                          // This value helps the programmer find a free cluster quickly.  So, this field has the location of the cluster located most recently.  If this field is "0xFFFFFFFF", the file system does not know where is free cluster.
} fat_file_system_information_t;

typedef struct
{
                    // **** Start Regions ****
    uint32_t                            reserved_region_start;                      // boot sector of the partition
    uint32_t                            fat_region_start;                           // = reserved_region_start + var->boot_sector.total_number_of_reserved_sectors
    uint32_t                            root_directory_region_start;                // = fat_region_start + (var->boot_sector.number_of_file_allocation_tables * var->boot_sector.number_of_sectors_per_fat)
    uint32_t                            data_space_region_start;                    // = root_directory_region_start + (var->boot_sector.number_of_possible_root_directory_entries * 32 / var->boot_sector.bytes_per_sector)                                                                                     
    uint32_t                            file_system_information_region_start;       // N/A FAT16    Sector number of FSInfo from the boot sector of the partition. Usually this exists a second sector (just after the boot sector) for the File System Information.
    uint32_t                            backup_boot_record_region_start;            // N/A FAT16    Sector number of the boot record backup from the boot sector of the partition.  
    uint32_t                            backup_file_system_information_region_start;// N/A FAT16    Sector number of the FSInfo backup from the boot sector of the partition.  
                                                                            
                    // **** CODE and OS Name ****                                                                    
    uint32_t                            jump_boot_code;                             //              Code to jump to the bootstrap code.
    char                                oem_name[9];                                //              Oem ID - Name of the formatting OS (8 bytes length - the 9th byte is '\0' for LOG)                                                                                    
                    // **** BIOS Parameter Block ****
    uint16_t                            number_of_bytes_per_sector;                 //              This value is the number of bytes in each physical sector. The allowed values are: 512, 1024, 2048 or 4096 bytes. A lot of code are assuming 512 bytes per sectors.
    uint8_t                             number_of_sectors_per_cluster;              //              This is the number of sectors per cluster. The allowed values are: 1, 2, 4, 8, 16, 32 or 128.
    uint16_t                            number_of_reserved_sectors;                 //              Since the reserved region always contain the boot sector a zero value in this field is not allowed. The usual setting of this value is 1. The value is used to calculate the location for the first sector containing the FAT.
    uint8_t                             number_of_file_allocation_tables;           //              This is the number of FAT copies in the file system. The recommended value is 2 (and then have two FAT copies). The usage of two copies are to prevent data loss if one or part of one FAT copy is corrupted.
    uint16_t                            number_of_possible_root_directory_entries;  // N/A FAT32 -  This value contain the number of entries in the root directory. Its recommended that the number of entries is an even multiple of the BytesPerSector values. The recommended value for FAT16 volumes is 512 entries (compatibility reasons).
    uint32_t                            number_of_sectors_in_the_partition;         //              This data can be found on "small_number_of_sectors" or "large_number_of_sectors" depending of the volume size. 
                                                                                    //              "small_number_of_sectors" used when volume size is FAT16 <= 32MB
                                                                                    //              "large_number_of_sectors" used when volume size is FAT16 > 32MB or FAT32
    uint8_t                             media_descriptor;                           //              0xF0     2.88 MB     3.5-inch, 2-sided, 36-sector
                                                                                    //              0xF0     1.44 MB     3.5-inch, 2-sided, 18-sector
                                                                                    //              0xF8     ?           Fixed disk
                                                                                    //              0xF9     720 KB      3.5-inch, 2-sided, 9-sector
                                                                                    //              0xF9     1.2 MB      5.25-inch, 2-sided, 15-sector
                                                                                    //              0xFA     ?           ?
                                                                                    //              0xFB     ?           ?
                                                                                    //              0xFC     180 KB      5.25-inch, 1-sided, 9-sector
                                                                                    //              0xFD     360 KB      5.25-inch, 2-sided, 9-sector
                                                                                    //              0xFE     160 KB      5.25-inch, 1-sided, 8-sector
                                                                                    //              0xFF     320 KB      5.25-inch, 2-sided, 8-sector
    uint32_t                            number_of_sectors_per_fat;                  //              16-bit for FAT16 and 32-bit for FAT32 are necessary. This is the number of sectors occupied by one copy of the FAT.
    uint16_t                            number_of_sectors_per_track;                //              This value is used when the volume is on a media which have a geometry, that is when the LBA number is broken down into a Cylinder-Head-Sector address. This field represents the multiple of the max. Head and Sector value used when the volume was formatted. The field itself is used to check if the LBA to CHS translation has changed, since the formatting. And for calculating the correct Cylinder, Head and Sector values for the translation algorithm.
    uint16_t                            number_of_heads;                            //              This value is used when the volume is on a media which have a geometry, that is when the LBA number is broken down into a Cylinder-Head-Sector address. This field represents the Head value used when the volume was formatted. The field itself is used to check if the LBA to CHS translation has changed, since the formatting. And for calculating the correct Cylinder, Head and Sector values for the translation algorithm.
    uint32_t                            number_of_hidden_sectors;                   //              When the volume is on a media that is partitioned, this value contains the number of sectors preceeding the first sector of the volume.
    fat32_file_system_handling_flags_t  fat_handling_flags;                         // N/A FAT16
    fat32_file_system_version_t         fat32_version;                              // N/A FAT16    // High byte is 'Major' and Low byte is 'Minor' version.
    uint32_t                            cluster_number_of_root_directory_table;     // N/A FAT16  
                    // **** Extended BIOS Parameter Block ****
    uint8_t                             physical_drive_number;                      //              This is the integer 13h drive number of the drive. The value 00h is used for the first floppy drive and the value 80h is used for the first harddrive.
    uint8_t                             current_head;                               //              Reserved byte. It was original used to store the cylinder on which the boot sector is located.
    uint8_t                             boot_signature;                             //              If this byte contain a value of 29h it indicates that the next three fields are available (volume_id, volume_label and file_system_type).
    uint32_t                            volume_id;                                  //              This value is a 32 bit random number, which, combined with the volume label, makes is possible to track removable media and check if the correct one is inserted.
    char                                volume_label[12];                           //              This 11 byte long string (the 12th byte is '\0' for LOG) should match the volume label entry in the root directory. If no such entry is available this field should contain the string 'NO NAME ' (11 bytes long string).
    char                                file_system_type[9];                        //              This 8 byte long string (the 9th byte is '\0' for LOG) should be used for informational display only. Thats because its sometime incorrectly set. The field should contain the string 'FAT16 ' (8 bytes long string).
} fat_file_system_boot_sector_t;

#define FAT_FILE_SYSTEM_MBR_PARTITION_ENTRY_1_OFFSET        0x1be
#define FAT_FILE_SYSTEM_MBR_PARTITION_ENTRY_2_OFFSET        0x1ce
#define FAT_FILE_SYSTEM_MBR_PARTITION_ENTRY_3_OFFSET        0x1de
#define FAT_FILE_SYSTEM_MBR_PARTITION_ENTRY_4_OFFSET        0x1ee

typedef enum
{
    FAT_FILE_SYSTEM_DESCRIPTOR_UNKNOWN                      = 0x00,
    FAT_FILE_SYSTEM_DESCRIPTOR_FAT12                        = 0x01,
    FAT_FILE_SYSTEM_DESCRIPTOR_FAT16_CHS_LOW                = 0x04,
    FAT_FILE_SYSTEM_DESCRIPTOR_FAT16_CHS_HIGH               = 0x06,
    FAT_FILE_SYSTEM_DESCRIPTOR_EXTENDED_MSDOS_CHS           = 0x05,
    FAT_FILE_SYSTEM_DESCRIPTOR_FAT32_CHS                    = 0x0b,
    FAT_FILE_SYSTEM_DESCRIPTOR_FAT32_LBA                    = 0x0c,
    FAT_FILE_SYSTEM_DESCRIPTOR_FAT16_LBA                    = 0x0e,
    FAT_FILE_SYSTEM_DESCRIPTOR_EXTENDED_MSDOS_LBA           = 0x0f,
            
    FAT_FILE_SYSTEM_DESCRIPTOR_MAX_LENGTH                   = 0xff
} FAT_FILE_SYSTEM_DESCRIPTOR;

typedef union
{
    struct 
    {
        unsigned                        sectors:6;                          // sectors
        unsigned                        cylinders:10;                       // cylinders pow(2, cylinders)
    };
    struct
    {
        uint16_t                        value;                              // cylinders [15..6] sectors [5..0]
    };
} fat_file_system_chs_t;

typedef struct
{
    bool                                _is_fat_32_partition;
    uint8_t                             boot_descriptor;                    // [1 byte]     0x80: if active partition / 0x00 if inactive
    uint8_t                             start_partition_head;               // [1+2 bytes]  CHS address of first absolute sector in partition (ignore cos LBA used these days)
    fat_file_system_chs_t               start_partition_cyl_sec;            // The CHS address mode has a limit of 16.515.072 sectors, which is usually 8 Gb. To add partitions beyond this limits, 
                                                                            // the drive must support the LBA address mode. With this addess mode the limit is 2 Tb. When dealing with partitions 
                                                                            // beyond this limit, the packed CHS information in the 'Start of partition' and 'End of partition' fields should be set 
                                                                            // to the maximum possible, and only the LBA address is used. 
    FAT_FILE_SYSTEM_DESCRIPTOR          file_system_descriptor;             // [1 byte]     See. enumeration FAT_FILE_SYSTEM_DESCRIPTOR
    uint8_t                             last_partition_head;                // [1+2 bytes]  CHS address of last absolute sector in partition (ignore cos LBA used these days)
    fat_file_system_chs_t               last_partition_cyl_sec;             // The CHS address mode has a limit of 16.515.072 sectors, which is usually 8 Gb. To add partitions beyond this limits, 
                                                                            // the drive must support the LBA address mode. With this addess mode the limit is 2 Tb. When dealing with partitions 
                                                                            // beyond this limit, the packed CHS information in the 'Start of partition' and 'End of partition' fields should be set 
                                                                            // to the maximum possible, and only the LBA address is used. 
    uint32_t                            first_sector_of_the_partition;      // [4 bytes]    Number of sector between the MBR (sector 0) and the first sector of the partition (called Boot Sector)
    uint32_t                            number_of_sector_in_the_partition;  // [4 bytes]    Number of sector in the partition x 512 bytes per sector = total size of the partition (in bytes)
} fat_file_system_partition_entry_t;

typedef struct
{
    fat_file_system_partition_entry_t partition_entry[4];
} fat_file_system_master_boot_record_t;

// The two defines below are available ONLY in the DATA SPACE region.
#define fat_file_system_get_first_sector_of_cluster_N(cluster_index)        ((uint32_t) (var->boot_sector.data_space_region_start + ((cluster_index) - 2) * var->boot_sector.number_of_sectors_per_cluster))
#define fat_file_system_get_cluster_of_sector_N(sector)                     ((uint32_t) (2 + ((sector) - var->boot_sector.data_space_region_start) / var->boot_sector.number_of_sectors_per_cluster))
#define fat_file_system_get_sector_index_in_cluster(sector)                 ((uint32_t) (((sector) - var->boot_sector.data_space_region_start) % var->boot_sector.number_of_sectors_per_cluster))

#define sd_card_read_file_pause(file)                       (file.flags.is_read_file_stopped = 1)
#define sd_card_read_file_play(file)                        (file.flags.is_read_file_stopped = 0)
#define sd_card_read_file_restart_playback(file)            (file.flags.is_read_file_stopped = 0, file.flags.is_read_file_terminated = 0, file.sm_read.index = 0)
#define sd_card_is_read_file_terminated(file)               (file.flags.is_read_file_terminated)

#define sd_card_read_file_pause_ptr(file)                   (file->flags.is_read_file_stopped = 1)
#define sd_card_read_file_play_ptr(file)                    (file->flags.is_read_file_stopped = 0)
#define sd_card_read_file_restart_playback_ptr(file)        (file->flags.is_read_file_stopped = 0, file->flags.is_read_file_terminated = 0, file->sm_read.index = 0)
#define sd_card_is_read_file_terminated_ptr(file)           (file->flags.is_read_file_terminated)

#endif