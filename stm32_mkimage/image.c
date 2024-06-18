
#include <time.h>
#include "crc.h"
#include "mkimage.h"
#include "image.h"
 
static table_entry_t uimage_arch[] = {
    {IH_ARCH_INVALID,   NULL,           "Invalid ARCH", },
    {IH_ARCH_ALPHA,     "alpha",        "Alpha",        },
    {IH_ARCH_ARM,       "arm",          "ARM",          },
    {IH_ARCH_I386,      "x86",          "Intel x86",    },
    {IH_ARCH_IA64,      "ia64",         "IA64",         },
    {IH_ARCH_M68K,      "m68k",         "M68K",         },
    {IH_ARCH_MICROBLAZE,"microblaze",   "MicroBlaze",   },
    {IH_ARCH_MIPS,      "mips",         "MIPS",         },
    {IH_ARCH_MIPS64,    "mips64",       "MIPS 64 Bit",  },
    {IH_ARCH_NIOS,      "nios",         "NIOS",         },
    {IH_ARCH_NIOS2,     "nios2",        "NIOS II",      },
    {IH_ARCH_PPC,       "powerpc",      "PowerPC",      },
    {IH_ARCH_PPC,       "ppc",          "PowerPC",      },
    {IH_ARCH_S390,      "s390",         "IBM S390",     },
    {IH_ARCH_SH,        "sh",           "SuperH",       },
    {IH_ARCH_SPARC,     "sparc",        "SPARC",        },
    {IH_ARCH_SPARC64,   "sparc64",      "SPARC 64 Bit", },
    {IH_ARCH_BLACKFIN,  "blackfin",     "Blackfin",     },
    {IH_ARCH_AVR32,     "avr32",        "AVR32",        },
    {-1,                "",             "",             },
};

static table_entry_t uimage_os[] = {
    {IH_OS_INVALID,     NULL,           "Invalid OS",   },
    {IH_OS_LINUX,       "linux",        "Linux",        },
    {IH_OS_LYNXOS,      "lynxos",       "LynxOS",       },
    {IH_OS_NETBSD,      "netbsd",       "NetBSD",       },
    {IH_OS_RTEMS,       "rtems",        "RTEMS",        },
    {IH_OS_U_BOOT,      "u-boot",       "U-Boot",       },
    {IH_OS_QNX,         "qnx",          "QNX",          },
    {IH_OS_VXWORKS,     "vxworks",      "VxWorks",      },
    {IH_OS_INTEGRITY,   "integrity",    "INTEGRITY",    },
    {IH_OS_4_4BSD,      "4_4bsd",    "  4_4BSD",        },
    {IH_OS_DELL,        "dell",         "Dell",         },
    {IH_OS_ESIX,        "esix",         "Esix",         },
    {IH_OS_FREEBSD,     "freebsd",      "FreeBSD",      },
    {IH_OS_IRIX,        "irix",         "Irix",         },
    {IH_OS_NCR,         "ncr",          "NCR",          },
    {IH_OS_OPENBSD,     "openbsd",      "OpenBSD",      },
    {IH_OS_PSOS,        "psos",         "pSOS",         },
    {IH_OS_SCO,         "sco",          "SCO",          },
    {IH_OS_SOLARIS,     "solaris",      "Solaris",      },
    {IH_OS_SVR4,        "svr4",         "SVR4",         },
    {-1,                "",             "",             },
};

static table_entry_t uimage_type[] = {
    {IH_TYPE_INVALID,       NULL,           "Invalid Image",            },
    {IH_TYPE_FILESYSTEM,    "filesystem",   "Filesystem Image",         },
    {IH_TYPE_FIRMWARE,      "firmware",     "Firmware",                 },
    {IH_TYPE_KERNEL,        "kernel",       "Kernel Image",             },
    {IH_TYPE_MULTI,         "multi",        "Multi-File Image",         },
    {IH_TYPE_RAMDISK,       "ramdisk",      "RAMDisk Image",            },
    {IH_TYPE_SCRIPT,        "script",       "Script",                   },
    {IH_TYPE_STANDALONE,    "standalone",   "Standalone Program",       },
    {IH_TYPE_FLATDT,        "flat_dt",      "Flat Device Tree",         },
    {IH_TYPE_KWBIMAGE,      "kwbimage",     "Kirkwood Boot Image",      },
    {IH_TYPE_IMXIMAGE,      "imximage",     "Freescale i.MX Boot Image",},
    {-1,                    "",             "",                         },
};

static table_entry_t uimage_comp[] = {
    {IH_COMP_NONE,      "none",     "uncompressed",     },
    {IH_COMP_BZIP2,     "bzip2",    "bzip2 compressed", },
    {IH_COMP_GZIP,      "gzip",     "gzip compressed",  },
    {IH_COMP_LZMA,      "lzma",     "lzma compressed",  },
    {IH_COMP_LZO,       "lzo",      "lzo compressed",   },
    {-1,                "",         "",                 },
};

static void genimg_print_time (time_t timestamp);

/*****************************************************************************/
/* Legacy format routines */
/*****************************************************************************/
int image_check_hcrc (const image_header_t *hdr)
{
    ulong hcrc;
    ulong len = image_get_header_size ();
    image_header_t header;

    /* Copy header so we can blank CRC field for re-calculation */
    memmove (&header, (char *)hdr, image_get_header_size ());
    image_set_hcrc (&header, 0);

    hcrc = crc32 (0, (unsigned char *)&header, len);

    return (hcrc == image_get_hcrc (hdr));
}

int image_check_dcrc (const image_header_t *hdr)
{
    ulong data = image_get_data (hdr);
    ulong len = image_get_data_size (hdr);
    ulong dcrc = crc32_wd (0, (unsigned char *)data, len, CHUNKSZ_CRC32);

    return (dcrc == image_get_dcrc (hdr));
}

/**
 * image_multi_count - get component (sub-image) count
 * @hdr: pointer to the header of the multi component image
 *
 * image_multi_count() returns number of components in a multi
 * component image.
 *
 * Note: no checking of the image type is done, caller must pass
 * a valid multi component image.
 *
 * returns:
 *     number of components
 */
ulong image_multi_count (const image_header_t *hdr)
{
    ulong i, count = 0;
    uint32_t *size;

    /* get start of the image payload, which in case of multi
     * component images that points to a table of component sizes */
    size = (uint32_t *)image_get_data (hdr);

    /* count non empty slots */
    for (i = 0; size[i]; ++i)
        count++;

    return count;
}

/**
 * image_multi_getimg - get component data address and size
 * @hdr: pointer to the header of the multi component image
 * @idx: index of the requested component
 * @data: pointer to a ulong variable, will hold component data address
 * @len: pointer to a ulong variable, will hold component size
 *
 * image_multi_getimg() returns size and data address for the requested
 * component in a multi component image.
 *
 * Note: no checking of the image type is done, caller must pass
 * a valid multi component image.
 *
 * returns:
 *     data address and size of the component, if idx is valid
 *     0 in data and len, if idx is out of range
 */
void image_multi_getimg (const image_header_t *hdr, ulong idx,
            ulong *data, ulong *len)
{
    int i;
    uint32_t *size;
    ulong offset, count, img_data;

    /* get number of component */
    count = image_multi_count (hdr);

    /* get start of the image payload, which in case of multi
     * component images that points to a table of component sizes */
    size = (uint32_t *)image_get_data (hdr);

    /* get address of the proper component data start, which means
     * skipping sizes table (add 1 for last, null entry) */
    img_data = image_get_data (hdr) + (count + 1) * sizeof (uint32_t);

    if (idx < count) {
        *len = uimage_to_cpu (size[idx]);
        offset = 0;

        /* go over all indices preceding requested component idx */
        for (i = 0; i < idx; i++) {
            /* add up i-th component size, rounding up to 4 bytes */
            offset += (uimage_to_cpu (size[i]) + 3) & ~3 ;
        }

        /* calculate idx-th component data address */
        *data = img_data + offset;
    } else {
        *len = 0;
        *data = 0;
    }
}

static void image_print_type (const image_header_t *hdr)
{
    const char *os, *arch, *type, *comp;

    os = genimg_get_os_name (image_get_os (hdr));
    arch = genimg_get_arch_name (image_get_arch (hdr));
    type = genimg_get_type_name (image_get_type (hdr));
    comp = genimg_get_comp_name (image_get_comp (hdr));

    printf ("%s %s %s (%s)\n", arch, os, type, comp);
}

/**
 * image_print_contents - prints out the contents of the legacy format image
 * @ptr: pointer to the legacy format image header
 * @p: pointer to prefix string
 *
 * image_print_contents() formats a multi line legacy image contents description.
 * The routine prints out all header fields followed by the size/offset data
 * for MULTI/SCRIPT images.
 *
 * returns:
 *     no returned results
 */
void image_print_contents (const void *ptr)
{
    const image_header_t *hdr = (const image_header_t *)ptr;
    const char *p = "";

    printf ("%sImage Name:   %.*s\n", p, IH_NMLEN, image_get_name (hdr));
    printf ("%sCreated:      ", p);
    genimg_print_time ((time_t)image_get_time (hdr));
    printf ("%sImage Type:   ", p);
    image_print_type (hdr);
    printf ("%sData Size:    ", p);
    genimg_print_size (image_get_data_size (hdr));
    printf ("%sLoad Address: %08x\n", p, image_get_load (hdr));
    printf ("%sEntry Point:  %08x\n", p, image_get_ep (hdr));

    if (image_check_type (hdr, IH_TYPE_MULTI) ||
            image_check_type (hdr, IH_TYPE_SCRIPT)) {
        int i;
        ulong data, len;
        ulong count = image_multi_count (hdr);

        printf ("%sContents:\n", p);
        for (i = 0; i < count; i++) {
            image_multi_getimg (hdr, i, &data, &len);

            printf ("%s   Image %d: ", p, i);
            genimg_print_size (len);

            if (image_check_type (hdr, IH_TYPE_SCRIPT) && i > 0) {
                /*
                 * the user may need to know offsets
                 * if planning to do something with
                 * multiple files
                 */
                printf ("%s    Offset = 0x%08lx\n", p, data);
            }
        }
    }
}

/*****************************************************************************/
/* Shared dual-format routines */
/*****************************************************************************/

void genimg_print_size (uint32_t size)
{
    printf ("%d Bytes = %.2f kB = %.2f MB\n",
            size, (double)size / 1.024e3,
            (double)size / 1.048576e6);
}

static void genimg_print_time (time_t timestamp)
{
    printf ("%s", ctime(&timestamp));
}

/**
 * get_table_entry_name - translate entry id to long name
 * @table: pointer to a translation table for entries of a specific type
 * @msg: message to be returned when translation fails
 * @id: entry id to be translated
 *
 * get_table_entry_name() will go over translation table trying to find
 * entry that matches given id. If matching entry is found, its long
 * name is returned to the caller.
 *
 * returns:
 *     long entry name if translation succeeds
 *     msg otherwise
 */
char *get_table_entry_name (table_entry_t *table, char *msg, int id)
{
    for (; table->id >= 0; ++table) {
        if (table->id == id)
            return table->lname;
    }
    return (msg);
}

const char *genimg_get_os_name (uint8_t os)
{
    return (get_table_entry_name (uimage_os, "Unknown OS", os));
}

const char *genimg_get_arch_name (uint8_t arch)
{
    return (get_table_entry_name (uimage_arch, "Unknown Architecture", arch));
}

const char *genimg_get_type_name (uint8_t type)
{
    return (get_table_entry_name (uimage_type, "Unknown Image", type));
}

const char *genimg_get_comp_name (uint8_t comp)
{
    return (get_table_entry_name (uimage_comp, "Unknown Compression", comp));
}

int get_table_entry_id (table_entry_t *table,
        const char *table_name, const char *name)
{
    table_entry_t *t;
    int first = 1;

    for (t = table; t->id >= 0; ++t) {
        if (t->sname && strcasecmp(t->sname, name) == 0)
            return (t->id);
    }

    fprintf (stderr, "\nInvalid %s Type - valid names are", table_name);
    for (t = table; t->id >= 0; ++t) {
        if (t->sname == NULL)
            continue;
        fprintf (stderr, "%c %s", (first) ? ':' : ',', t->sname);
        first = 0;
    }
    fprintf (stderr, "\n");
    return (-1);
}

int genimg_get_os_id (const char *name)
{
    return (get_table_entry_id (uimage_os, "OS", name));
}

int genimg_get_arch_id (const char *name)
{
    return (get_table_entry_id (uimage_arch, "CPU", name));
}

int genimg_get_type_id (const char *name)
{
    return (get_table_entry_id (uimage_type, "Image", name));
}

int genimg_get_comp_id (const char *name)
{
    return (get_table_entry_id (uimage_comp, "Compression", name));
}

