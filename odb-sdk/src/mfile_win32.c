#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "odb-sdk/mem.h"
#include "odb-sdk/mfile.h"
#include "odb-sdk/utf8.h"
#include "odb-sdk/log.h"

int
mfile_map_cow_with_extra_padding(struct mfile* mf, struct ospathc filepath, int padding)
{
    HANDLE hFile;
    LARGE_INTEGER liFileSize;
    HANDLE hMapping;
    void* address;
    struct utf16 utf16_filename = empty_utf16();

    if (utf8_to_utf16(&utf16_filename, ospathc_view(filepath)) != 0)
        goto utf16_conv_failed;

    /* Try to open the file */
    hFile = CreateFileW(
        utf16_cstr(utf16_filename), /* File name */
        GENERIC_READ,           /* Read only */
        FILE_SHARE_READ,
        NULL,                   /* Default security */
        OPEN_EXISTING,          /* File must exist */
        FILE_ATTRIBUTE_NORMAL,  /* Default attributes */
        NULL);                  /* No attribute template */
    if (hFile == INVALID_HANDLE_VALUE)
    {
        log_sdk_err(
            "Failed to open file {quote:%s}: {win32error}\n",
            ospathc_cstr(filepath));
        goto open_failed;
    }

    hMapping = CreateFileMappingW(
        hFile,                 /* File handle */
        NULL,                  /* Default security attributes */
        PAGE_READONLY,         /* Read-only */
        0, 0,                  /* High/Low size of mapping. Zero means entire file */
        NULL);                 /* Don't name the mapping */
    if (hMapping == NULL)
    {
        log_sdk_err(
            "Failed to create file mapping for file {quote:%s}: {win32error}\n",
            ospathc_cstr(filepath));
        goto create_file_mapping_failed;
    }

    address = MapViewOfFile(
        hMapping,               /* File mapping handle */
        FILE_MAP_READ,         /* Copy-on-Write */
        0, 0,                  /* High/Low offset of where the mapping should begin in the file */
        0);                    /* Length of mapping. Zero means entire file */
    if (address == NULL)
    {
        log_sdk_err(
            "Failed to map view of file {quote:%s}: {win32error}\n",
            ospathc_cstr(filepath));
        goto map_view_failed;
    }

    /*
     *    Can't copy-on-write a larger range without changing the file on disk
                                        ..::..                             
                                     .-=+==+++=-:.                         
                                   :=-----======+=:                        
                                  .=-=-----=====++=:                       
                                  :====-----===+**+=.                      
                                  :==+*+---=******++=.                     
                                  --=+*+===**+*+***#*=                     
                                  :=======++==+=+**#+-                     
                                  .=======+*===+**#*=                      
                                   :-==++=***++****=.                      
                                     -======+*+***#%#*+*#***=.             
                              .:-+***+=++=++**+**#%%###########-           
                            :+*######*+=======+*#%%#############-          
                           .+*#########*====+*#%%%##########%####:         
                           *###############%%%%%%##########%%#####         
                           *###############%%%%%###########%%####%+        
                          .#############################%##%%#####%-       
                          :###############################%%%##%###*       
                          -############################%##%%%%#%%###-      
         :-==-....        -############################%##%%%%######*      
           ..:==----=.    +#%#######################*=++##%%%%#######-     
             .------==+*.-#%%#####################*=-=++*#%@%%######%*.    
         .--=--===----=+#%%%%####################+===+*%%#%@%%#######%=    
       .=======----=+**#%%%%%%#################*---===*%%%%%%%%%%%%####:   
        ..:=*+=++++*#%%%%%%%%@################*----==*%%@%%%%%%%%%%%%%%*   
                   .+%%%%%%%%%%###############=-==+*#%@@@%%%%%%%%%%%%%%%#  
                     .*%%%%%%@%#############++-=**#%%%@@%%%%%%%%%%%%%%%%#. 
                       :#%%%%%%#####################%%%%%%%%%%%%%%%%%%%%*  
                         .=##%%%###################%%%%%%%%%%%%%%%%%%%%#   
                             .-+##################%%%%%%%@@@@@@%%%%%%%+    

                Guess I'll copy the entire fucking file
                thanks windows
     */
    /* Determine file size in bytes */
    if (!GetFileSizeEx(hFile, &liFileSize))
        goto get_file_size_failed;
    if (liFileSize.QuadPart + padding > (1ULL << 31) - 1)  /* mf->size is an int */
    {
        log_sdk_err(
            "Failed to map file {quote:%s}: Mapping files >4GiB is not implemented\n",
            ospathc_cstr(filepath));
        goto get_file_size_failed;
    }
    if (mfile_map_mem(mf, liFileSize.LowPart + padding) != 0)
        goto alloc_copy_failed;

    memcpy(mf->address, address, liFileSize.LowPart);

    /* Don't need mapped file anymore */
    UnmapViewOfFile(address);
    CloseHandle(hMapping);
    CloseHandle(hFile);
    utf16_deinit(utf16_filename);

    return 0;

    alloc_copy_failed          :
    get_file_size_failed       : UnmapViewOfFile(address);
    map_view_failed            : CloseHandle(hMapping);
    create_file_mapping_failed : CloseHandle(hFile);
    open_failed                : utf16_deinit(utf16_filename);
    utf16_conv_failed          : return -1;
}

int
mfile_map_mem(struct mfile* mf, int size)
{
    HANDLE mapping = CreateFileMapping(
        INVALID_HANDLE_VALUE,  /* File handle */
        NULL,                  /* Default security attributes */
        PAGE_READWRITE,        /* Read + Write access */
        0, size,               /* High/Low size of mapping. Zero means entire file */
        NULL);                 /* Don't name the mapping */
    if (mapping == NULL)
    {
        log_sdk_err(
            "Failed to create file mapping of size {emph:%d}: {win32error}\n",
            size);
        goto create_file_mapping_failed;
    }

    mf->address = MapViewOfFile(
        mapping,               /* File mapping handle */
        FILE_MAP_WRITE,        /* Read + Write */
        0, 0,                  /* High/Low offset of where the mapping should begin in the file */
        size);                 /* Length of mapping. Zero means entire file */
    if (mf->address == NULL)
    {
        log_sdk_err(
            "Failed to map memory of size {emph:%d}: {win32error}\n",
            size);
        goto map_view_failed;
    }

    CloseHandle(mapping);

    mem_track_allocation(mf->address);
    mf->size = size;

    return 0;

    map_view_failed            : CloseHandle(mapping);
    create_file_mapping_failed : return -1;
}

void mfile_unmap(struct mfile* mf)
{
    mem_track_deallocation(mf->address);
    UnmapViewOfFile(mf->address);
}
