#ifndef __IMAGE_H__
#define __IMAGE_H__

#include <stdint.h>
#include <asm/byteorder.h>
#define IH_OS_INVALID       0    /* Invalid OS      */
#define IH_OS_RTTHREAD      1    /* RTThread           */

#define IH_ARCH_INVALID     0    /* Invalid CPU    */
#define IH_ARCH_ARM         1    /* ARM             */

#define IH_TYPE_INVALID         0    /* Invalid Image          */
#define IH_TYPE_KERNEL          1    /* OS Kernel Image        */
#define IH_TYPE_MAX             10

#define IH_COMP_NONE            0    /*  No     Compression Used    */

#define IH_MAGIC                0x27051957  /* Image Magic Number       */
#define IH_NMLEN                32          /* Image Name Length        */

/* Error codes: informative error codes */
#define FDT_ERR_NOTFOUND    1
    /* FDT_ERR_NOTFOUND: The requested node or property does not exist */
#define FDT_ERR_EXISTS        2
    /* FDT_ERR_EXISTS: Attemped to create a node or property which
     * already exists */
#define FDT_ERR_NOSPACE        3
    /* FDT_ERR_NOSPACE: Operation needed to expand the device
     * tree, but its buffer did not have sufficient space to
     * contain the expanded tree. Use fdt_open_into() to move the
     * device tree to a buffer with more space. */

/* Error codes: codes for bad parameters */
#define FDT_ERR_BADOFFSET    4
    /* FDT_ERR_BADOFFSET: Function was passed a structure block
     * offset which is out-of-bounds, or which points to an
     * unsuitable part of the structure for the operation. */
#define FDT_ERR_BADPATH        5
    /* FDT_ERR_BADPATH: Function was passed a badly formatted path
     * (e.g. missing a leading / for a function which requires an
     * absolute path) */
#define FDT_ERR_BADPHANDLE    6
    /* FDT_ERR_BADPHANDLE: Function was passed an invalid phandle
     * value.  phandle values of 0 and -1 are not permitted. */
#define FDT_ERR_BADSTATE    7
    /* FDT_ERR_BADSTATE: Function was passed an incomplete device
     * tree created by the sequential-write functions, which is
     * not sufficiently complete for the requested operation. */

/* Error codes: codes for bad device tree blobs */
#define FDT_ERR_TRUNCATED    8
    /* FDT_ERR_TRUNCATED: Structure block of the given device tree
     * ends without an FDT_END tag. */
#define FDT_ERR_BADMAGIC    9
    /* FDT_ERR_BADMAGIC: Given "device tree" appears not to be a
     * device tree at all - it is missing the flattened device
     * tree magic number. */
#define FDT_ERR_BADVERSION    10
    /* FDT_ERR_BADVERSION: Given device tree has a version which
     * can't be handled by the requested operation.  For
     * read-write functions, this may mean that fdt_open_into() is
     * required to convert the tree to the expected version. */
#define FDT_ERR_BADSTRUCTURE    11
    /* FDT_ERR_BADSTRUCTURE: Given device tree has a corrupt
     * structure block or other serious error (e.g. misnested
     * nodes, or subnodes preceding properties). */
#define FDT_ERR_BADLAYOUT    12
    /* FDT_ERR_BADLAYOUT: For read-write functions, the given
     * device tree has it's sub-blocks in an order that the
     * function can't handle (memory reserve map, then structure,
     * then strings).  Use fdt_open_into() to reorganize the tree
     * into a form suitable for the read-write operations. */

/* "Can't happen" error indicating a bug in libfdt */
#define FDT_ERR_INTERNAL    13
    /* FDT_ERR_INTERNAL: libfdt has failed an internal assertion.
     * Should never be returned, if it is, it indicates a bug in
     * libfdt itself. */

#define FDT_ERR_MAX        13

/*
 * Legacy format image header,
 * all data in network byte order (aka natural aka bigendian).
 */
typedef struct image_header {
    uint32_t    ih_magic;    /* Image Header Magic Number    */
    uint32_t    ih_hcrc;    /* Image Header CRC Checksum    */
    uint32_t    ih_time;    /* Image Creation Timestamp    */
    uint32_t    ih_size;    /* Image Data Size        */
    uint32_t    ih_load;    /* Data     Load  Address        */
    uint32_t    ih_ep;        /* Entry Point Address        */
    uint32_t    ih_dcrc;    /* Image Data CRC Checksum    */
    uint8_t        ih_os;        /* Operating System        */
    uint8_t        ih_arch;    /* CPU architecture        */
    uint8_t        ih_type;    /* Image Type            */
    uint8_t        ih_comp;    /* Compression Type        */
    uint8_t        ih_name[IH_NMLEN];    /* Image Name        */
} image_header_t;

typedef struct image_info {
    ulong        start, end;        /* start/end of blob */
    ulong        image_start, image_len; /* start of image within blob, len of image */
    ulong        load;            /* load addr for the image */
    uint8_t        comp, type, os;        /* compression, type of image, os type */
} image_info_t;

/*
 * Legacy and FIT format headers used by do_bootm() and do_bootm_<os>()
 * routines.
 */
typedef struct bootm_headers {
    /*
     * Legacy os image header, if it is a multi component image
     * then boot_get_ramdisk() and get_fdt() will attempt to get
     * data from second and third component accordingly.
     */
    image_header_t    *legacy_hdr_os;        /* image header pointer */
    image_header_t    legacy_hdr_os_copy;    /* header copy */
    ulong        legacy_hdr_valid;

    int        verify;        /* getenv("verify")[0] != 'n' */

#define    BOOTM_STATE_START    (0x00000001)
#define    BOOTM_STATE_LOADOS    (0x00000002)
#define    BOOTM_STATE_RAMDISK    (0x00000004)
#define    BOOTM_STATE_FDT        (0x00000008)
#define    BOOTM_STATE_OS_CMDLINE    (0x00000010)
#define    BOOTM_STATE_OS_BD_T    (0x00000020)
#define    BOOTM_STATE_OS_PREP    (0x00000040)
#define    BOOTM_STATE_OS_GO    (0x00000080)
    int        state;
} bootm_headers_t;

/*
 * Some systems (for example LWMON) have very short watchdog periods;
 * we must make sure to split long operations like memmove() or
 * checksum calculations into reasonable chunks.
 */
#ifndef CHUNKSZ
#define CHUNKSZ (64 * 1024)
#endif

#ifndef CHUNKSZ_CRC32
#define CHUNKSZ_CRC32 (64 * 1024)
#endif

#ifndef CHUNKSZ_MD5
#define CHUNKSZ_MD5 (64 * 1024)
#endif

#ifndef CHUNKSZ_SHA1
#define CHUNKSZ_SHA1 (64 * 1024)
#endif

#define uimage_to_cpu(x)        __be32_to_cpu(x)
#define cpu_to_uimage(x)        __cpu_to_be32(x)

/*
 * Translation table for entries of a specific type; used by
 * get_table_entry_id() and get_table_entry_name().
 */
typedef struct table_entry {
    int    id;
    char    *sname;        /* short (input) name to find table entry */
    char    *lname;        /* long (output) name to print for messages */
} table_entry_t;

/*
 * get_table_entry_id() scans the translation table trying to find an
 * entry that matches the given short name. If a matching entry is
 * found, it's id is returned to the caller.
 */
int get_table_entry_id (table_entry_t *table,
        const char *table_name, const char *name);
/*
 * get_table_entry_name() scans the translation table trying to find
 * an entry that matches the given id. If a matching entry is found,
 * its long name is returned to the caller.
 */
char *get_table_entry_name (table_entry_t *table, char *msg, int id);

const char *genimg_get_os_name (uint8_t os);
const char *genimg_get_arch_name (uint8_t arch);
const char *genimg_get_type_name (uint8_t type);
const char *genimg_get_comp_name (uint8_t comp);
int genimg_get_os_id (const char *name);
int genimg_get_arch_id (const char *name);
int genimg_get_type_id (const char *name);
int genimg_get_comp_id (const char *name);
void genimg_print_size (uint32_t size);

/*******************************************************************/
/* Legacy format specific code (prefixed with image_) */
/*******************************************************************/
static inline uint32_t image_get_header_size (void)
{
    return (sizeof (image_header_t));
}

#define image_get_hdr_l(f) \
    static inline uint32_t image_get_##f(const image_header_t *hdr) \
    { \
        return uimage_to_cpu (hdr->ih_##f); \
    }
image_get_hdr_l (magic);    /* image_get_magic */
image_get_hdr_l (hcrc);        /* image_get_hcrc */
image_get_hdr_l (time);        /* image_get_time */
image_get_hdr_l (size);        /* image_get_size */
image_get_hdr_l (load);        /* image_get_load */
image_get_hdr_l (ep);        /* image_get_ep */
image_get_hdr_l (dcrc);        /* image_get_dcrc */

#define image_get_hdr_b(f) \
    static inline uint8_t image_get_##f(const image_header_t *hdr) \
    { \
        return hdr->ih_##f; \
    }
image_get_hdr_b (os);        /* image_get_os */
image_get_hdr_b (arch);        /* image_get_arch */
image_get_hdr_b (type);        /* image_get_type */
image_get_hdr_b (comp);        /* image_get_comp */

static inline char *image_get_name (const image_header_t *hdr)
{
    return (char *)hdr->ih_name;
}

static inline uint32_t image_get_data_size (const image_header_t *hdr)
{
    return image_get_size (hdr);
}

/**
 * image_get_data - get image payload start address
 * @hdr: image header
 *
 * image_get_data() returns address of the image payload. For single
 * component images it is image data start. For multi component
 * images it points to the null terminated table of sub-images sizes.
 *
 * returns:
 *     image payload data start address
 */
static inline ulong image_get_data (const image_header_t *hdr)
{
    return ((ulong)hdr + image_get_header_size ());
}

static inline uint32_t image_get_image_size (const image_header_t *hdr)
{
    return (image_get_size (hdr) + image_get_header_size ());
}
static inline ulong image_get_image_end (const image_header_t *hdr)
{
    return ((ulong)hdr + image_get_image_size (hdr));
}

#define image_set_hdr_l(f) \
    static inline void image_set_##f(image_header_t *hdr, uint32_t val) \
    { \
        hdr->ih_##f = cpu_to_uimage (val); \
    }
image_set_hdr_l (magic);    /* image_set_magic */
image_set_hdr_l (hcrc);        /* image_set_hcrc */
image_set_hdr_l (time);        /* image_set_time */
image_set_hdr_l (size);        /* image_set_size */
image_set_hdr_l (load);        /* image_set_load */
image_set_hdr_l (ep);        /* image_set_ep */
image_set_hdr_l (dcrc);        /* image_set_dcrc */

#define image_set_hdr_b(f) \
    static inline void image_set_##f(image_header_t *hdr, uint8_t val) \
    { \
        hdr->ih_##f = val; \
    }
image_set_hdr_b (os);        /* image_set_os */
image_set_hdr_b (arch);        /* image_set_arch */
image_set_hdr_b (type);        /* image_set_type */
image_set_hdr_b (comp);        /* image_set_comp */

static inline void image_set_name (image_header_t *hdr, const char *name)
{
    strncpy (image_get_name (hdr), name, IH_NMLEN);
}

int image_check_hcrc (const image_header_t *hdr);
int image_check_dcrc (const image_header_t *hdr);

static inline int image_check_magic (const image_header_t *hdr)
{
    return (image_get_magic (hdr) == IH_MAGIC);
}
static inline int image_check_type (const image_header_t *hdr, uint8_t type)
{
    return (image_get_type (hdr) == type);
}
static inline int image_check_arch (const image_header_t *hdr, uint8_t arch)
{
    return (image_get_arch (hdr) == arch);
}
static inline int image_check_os (const image_header_t *hdr, uint8_t os)
{
    return (image_get_os (hdr) == os);
}

ulong image_multi_count (const image_header_t *hdr);
void image_multi_getimg (const image_header_t *hdr, ulong idx,
            ulong *data, ulong *len);

void image_print_contents (const void *hdr);

#endif    /* __IMAGE_H__ */
