/// Data structures that describe the MIPS COFF format.

#ifndef NACHOS_BIN_COFF__H
#define NACHOS_BIN_COFF__H


#ifdef HOST_x86_64
typedef int DWORD;
#else
typedef long DWORD;
#endif

struct filehdr {
    unsigned short f_magic;   /// Magic number.
    unsigned short f_nscns;   /// Number of sections.
    DWORD          f_timdat;  /// Time & date stamp.
    DWORD          f_symptr;  /// File pointer to symbolic header.
    DWORD          f_nsyms;   /// Size of symbolic header.
    unsigned short f_opthdr;  /// Size of optional header.
    unsigned short f_flags;   /// Flags.
};

#define MIPSELMAGIC  0x0162

#define OMAGIC   0407
#define SOMAGIC  0x0701

typedef struct aouthdr {
    short magic;       /// See above.
    short vstamp;      /// Version stamp.
    DWORD tsize;       /// Text size in bytes, padded to DW boundary.
    DWORD dsize;       /// Initialized data "  ".
    DWORD bsize;       /// Uninitialized data "   ".
    DWORD entry;       /// Entry point.
    DWORD text_start;  /// Base of text used for this file.
    DWORD data_start;  /// Base of data used for this file.
    DWORD bss_start;   /// Base of bss used for this file.
    DWORD gprmask;     /// General purpose register mask.
    DWORD cprmask[4];  /// Co-processor register masks.
    DWORD gp_value;    /// The gp value used for this object.
} AOUTHDR;
#define AOUTHSZ  sizeof (AOUTHDR)


struct scnhdr {
    char           s_name[8];  /// Section name.
    DWORD          s_paddr;    /// Physical address, aliased s_nlib.
    DWORD          s_vaddr;    /// Virtual address.
    DWORD          s_size;     /// Section size.
    DWORD          s_scnptr;   /// File pointer to raw data for section.
    DWORD          s_relptr;   /// File pointer to relocation.
    DWORD          s_lnnoptr;  /// File pointer to gp histogram.
    unsigned short s_nreloc;   /// Number of relocation entries.
    unsigned short s_nlnno;    /// Number of gp histogram entries.
    DWORD          s_flags;    /// Flags.
};


#endif
