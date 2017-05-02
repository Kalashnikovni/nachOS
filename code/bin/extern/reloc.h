struct reloc {
    long           r_vaddr;          // (Virtual) address of reference.
    long           r_symndx;         // Index into symbol table.
    unsigned short r_type;           // Relocation type.
    unsigned       r_symndx_  : 24,  // Index into symbol table.
                   r_reserved : 3,
                   r_type_    : 4,   // Relocation type.
                   r_extern   : 1;   // If 1, symndx is an index into the
                                     // external symbol table, else symndx
                                     // is a section number.
};
