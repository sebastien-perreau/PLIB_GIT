#ifndef __DEF_FAT16_FILE_SYSTEM
#define __DEF_FAT16_FILE_SYSTEM

typedef enum
{
    FAT16_FILE_SYSTEME_FA_LFN           = 0x0f,                         // Long File Name attribute (always equal to 0x0f if the 32 bytes entry is a part of a Long File Name)
    FAT16_FILE_SYSTEME_FA_READ          = 0x01,
    FAT16_FILE_SYSTEME_FA_HIDDEN        = 0x02,
    FAT16_FILE_SYSTEME_FA_SYSTEM        = 0x04,
    FAT16_FILE_SYSTEME_FA_VOLUME_NAME   = 0x08,
    FAT16_FILE_SYSTEME_FA_DIRECTORY     = 0x10,
    FAT16_FILE_SYSTEME_FA_ARCHIVE       = 0x20
} FAT16_FILE_SYSTEM_FILE_ATTRIBUTES;

typedef enum
{
    FAT16_FILE_SYSTEM_FLAG_READ_BLOCK_OP_READ_REQUESTED   = 1,
    FAT16_FILE_SYSTEM_FLAG_READ_BLOCK_OP_READ_ON_GOING    = 2,
    FAT16_FILE_SYSTEM_FLAG_READ_BLOCK_OP_READ_TERMINATED  = 3
} FAT16_FILE_SYSTEM_FLAGS_READ_OP;

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
} fat16_file_system_file_attributes_t;

typedef union
{
    struct 
    {
        unsigned                        seconds:5;
        unsigned                        minutes:6;
        unsigned                        hours:5;
    };
    struct
    {
        uint16_t                        value;
    };
} fat16_file_system_time_t;

typedef union
{
    struct 
    {
        unsigned                        day:5;
        unsigned                        month:4;
        unsigned                        year:7;
    };
    struct
    {
        uint16_t                        value;
    };
} fat16_file_system_date_t;

typedef union
{
    struct
    {
        bool                            is_found;                       // This bit is set if the file is found on the SD Card
        bool                            is_fat_updated;
        FAT16_FILE_SYSTEM_FLAGS_READ_OP is_read_block_op;               // 0: not used, 1: read_block_requested, 2: read_block_on_going, 3: read_block_terminated
        bool                            is_read_file_stopped;           // 0: play / continue, 1: stop / pause
        bool                            is_read_file_terminated;        // 0: no, 1: yes
        unsigned                        :2;
    };
    struct
    {
        uint8_t                         value;
    };
} fat16_file_system_flags_t;

typedef struct
{
    char                                file_name[255];                 // File name and extension (example "My Folder\my_file.extension")
    fat16_file_system_file_attributes_t file_attributes;
    uint8_t                             creation_time_ms;               // Due to size limitations this field (1 byte) only contains the millisecond stamp in counts of 10 milliseconds. Therefore valid values are between 0 and 199 inclusive.
    fat16_file_system_time_t            creation_time_h_m_s;            // The 16 bit time field contain the time of day when this entry was created. This value never change. hours (0..23) [15..11] minutes (0..59) [10..5] seconds (0..29 - multiply by 2) [4..0]
    fat16_file_system_date_t            creation_date;                  // The 16 bit date field contain the date of day when this entry was created. This value never change. year (0..127 -> 1980..2107) [15..9] month (1..12) [8..5] day (1..31) [4..0]
    fat16_file_system_date_t            last_access_date;               // The 16 bit date field contain the date when the entry was last read or written to. year (0..127 -> 1980..2107) [15..9] month (1..12) [8..5] day (1..31) [4..0]
    uint16_t                            extended_address_index;         // Reserved for fat32
    fat16_file_system_time_t            last_write_time_h_m_s;          // The 16 bit time field contain the time of day when this entry was created. hours (0..23) [15..11] minutes (0..59) [10..5] seconds (0..29 - multiply by ) [4..0]
    fat16_file_system_date_t            last_write_date;                // The 16 bit date field contain the date of day when this entry was created. year (0..127 -> 1980..2107) [15..9] month (1..12) [8..5] day (1..31) [4..0]
    uint16_t                            first_cluster_of_the_file;      // This 16-bit field points to the starting cluster number of entries data. If the entry is a directory this entry point to the cluster which contain the beginning of the sub-directory. If the entry is a file then this entry point to the cluster holding the first chunk of data from the file.
    uint32_t                            file_size;                      // This 32-bit field count the total file size in bytes. For this reason the file system driver must not allow more than 4 GB to be allocated to a file. For other entries than files then file size field should be set to 0.
    
    fat16_file_system_flags_t           flags;                          
    uint32_t                            first_sector_of_the_file;       
    DYNAMIC_TAB_WORD                    fat;                            // Contains the File Allocation Table for the file
    
    state_machine_t                     sm_read;
    DYNAMIC_TAB_BYTE                    buffer;
    uint32_t                            _data_address;
    uint16_t                            _data_length;
    
    void                                *p_sd_card;
    
} fat16_file_system_entry_t;

#define FAT16_FILE_SYSTEM_ENTRY_INSTANCE(_name, _file_name, _p_fat_ram, _p_sd_card)     \
{                                                                           \
    .file_name = _file_name,                                                \
    .file_attributes = 0,                                                   \
    .creation_time_ms = 0,                                                  \
    .creation_time_h_m_s = 0,                                               \
    .creation_date = 0,                                                     \
    .last_access_date = 0,                                                  \
    .extended_address_index = 0,                                            \
    .last_write_time_h_m_s = 0,                                             \
    .last_write_date = 0,                                                   \
    .first_cluster_of_the_file = 0,                                         \
    .file_size = 0,                                                         \
    .flags = {0},                                                           \
    .fat = {_p_fat_ram, 0, 0},                                              \
    .sm_read = {0},                                                         \
    .buffer = {0, 0, 0},                                                    \
    ._data_address = 0,                                                     \
    ._data_length = 0,                                                      \
    .p_sd_card = (void*) &_p_sd_card                                        \
}

#define FILE_DEF(_p_sd_card, _name, _file_name)                             \
static uint16_t _name ## _fat_ram_allocation[500];                          \
static fat16_file_system_entry_t _name = FAT16_FILE_SYSTEM_ENTRY_INSTANCE(_name, _file_name, _name ## _fat_ram_allocation, _p_sd_card)

typedef struct
{
    uint32_t                            reserved_region_start;                      // boot sector of the partition
    uint32_t                            fat_region_start;                           // = reserved_region_start + var->boot_sector.total_number_of_reserved_sectors
    uint32_t                            root_directory_region_start;                // = fat_region_start + (var->boot_sector.number_of_file_allocation_tables * var->boot_sector.number_of_sectors_per_fat)
    uint32_t                            data_space_region_start;                    // = root_directory_region_start + (var->boot_sector.number_of_possible_root_directory_entries * 32 / var->boot_sector.bytes_per_sector)
                                                                                    // Important: The DATA SPACE start at Cluster 2 because Cluster 0 and Cluster 1 are reserved.        
    uint32_t                            jump_command;
    char                                oem_name[9];
    uint16_t                            number_of_bytes_per_sector;
    uint8_t                             number_of_sectors_per_cluster;
    uint16_t                            number_of_reserved_sectors;
    uint8_t                             number_of_file_allocation_tables;
    uint16_t                            number_of_possible_root_directory_entries;
    uint16_t                            small_number_of_sectors;                    // Used when volume size < 32MB
    uint32_t                            large_number_of_sectors;                    // Used when volume size > 32MB
    uint8_t                             media_descriptor;
    uint16_t                            number_of_sectors_per_fat;
    uint16_t                            number_of_sectors_per_track;
    uint16_t                            number_of_heads;
    uint32_t                            number_of_hidden_sectors;
    uint8_t                             physical_drive_number;
    uint8_t                             current_head;
    uint8_t                             boot_signature;
    uint32_t                            volume_id;
    char                                volume_label[12];
    char                                file_system_type[9];
} fat16_file_system_boot_sector_t;

#define FAT16_FILE_SYSTEM_MBR_PARTITION_ENTRY_1_OFFSET      0x1be
#define FAT16_FILE_SYSTEM_MBR_PARTITION_ENTRY_2_OFFSET      0x1ce
#define FAT16_FILE_SYSTEM_MBR_PARTITION_ENTRY_3_OFFSET      0x1de
#define FAT16_FILE_SYSTEM_MBR_PARTITION_ENTRY_4_OFFSET      0x1ee

typedef struct
{
    bool                                is_existing;                        //              true: if datas present / false: all is cleared (0x00)
    uint8_t                             boot_descriptor;                    // [1 byte]     0x80: if active partition / 0x00 if inactive
    uint32_t                            first_partition_sector;             // [3 bytes]    CHS address of first absolute sector in partition (ignore cos LBA used these days)
    uint8_t                             file_system_descriptor;             // [1 byte]     0x04: 16-bit FAT < 32M / 0x06: 16-bit FAT >= 32M / 0x0e: DOS CHS mapped
    uint32_t                            last_partition_sector;              // [3 bytes]    CHS address of last absolute sector in partition (ignore cos LBA used these days)
    uint32_t                            first_sector_of_the_partition;      // [4 bytes]    Number of sector between the MBR (sector 0) and the first sector of the partition (called Boot Sector)
    uint32_t                            number_of_sector_in_the_partition;  // [4 bytes]    Number of sector in the partition x 512 bytes per sector = total size of the partition (in bytes)
} fat16_file_system_partition_entry_t;

typedef struct
{
    fat16_file_system_partition_entry_t partition_entry[4];
} fat16_file_system_master_boot_record_t;

#define fat16_file_system_get_first_sector_of_cluster_N(data_space_region_start, sectors_per_cluster, cluster_index)        ((uint32_t) (data_space_region_start + (cluster_index - 2) * sectors_per_cluster))
#define fat16_file_system_get_fat_sector_of_cluster_N(fat_region_start, bytes_per_sector, cluster_index)                    ((uint32_t) (fat_region_start + cluster_index * 2 / bytes_per_sector))

#define sd_card_is_read_operation_terminated(file)          ((file.flags.is_read_op == FAT16_FILE_SYSTEM_FLAG_READ_OP_READ_TERMINATED) ? 1 : 0)

#define sd_card_read_file_pause(file)                       (file.flags.is_read_file_stopped = 1)
#define sd_card_read_file_play(file)                        (file.flags.is_read_file_stopped = 0)
#define sd_card_read_file_restart_playback(file)            (file.flags.is_read_file_stopped = 0, file.flags.is_read_file_terminated = 0, file.sm_read.index = 0)
#define sd_card_is_read_file_terminated(file)               (file.flags.is_read_file_terminated)

#endif