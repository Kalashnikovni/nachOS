/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2017 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.


#include "instr.h"
#include "encode.h"
#include "int.h"

#include <stdio.h>
#include <string.h>


#define FAST   0


extern char mem[];
extern int TRACE, Regtrace;

// Machine registers
static int Reg[32];  // General purpose registers.
static int HI, LO;   // mul/div machine registers.

// Statistics gathering places
static int numjmpls;
static int arch1cycles;

// Condition-code calculations
#define B31(z)  (((z) >> 31) & 0x1)  // Extract bit 31.

// Code looks funny but is fast thanks to MIPS!
#define CcAdd(rr, op1, op2)                  \
    N = (rr) < 0;                            \
    Z = (rr) == 0;                           \
    C = (unsigned) (rr) < (unsigned) (op2);  \
    V = ((op1) ^ (op2)) >= 0 && ((op1) ^ (rr)) < 0;

#define CcSub(rr, op1, op2)                                       \
    N = (rr) < 0;                                                 \
    Z = (rr) == 0;                                                \
    V = B31(((op1) & ~(op2) & ~(rr)) | (~(op1) & (op2) & (rr)));  \
    C = (unsigned) (op1) < (unsigned) (op2);

    /* C = B31((~op1 & op2) | (rr & (~op1 | op2))); */

#define CcLogic(rr)   \
    N = ((rr) < 0);   \
    Z = ((rr) == 0);  \
    V = 0;            \
    C = 0;

#define CcMulscc(rr, op1, op2)                                    \
    N = (rr) < 0;                                                 \
    Z = (rr) == 0;                                                \
    V = B31(((op1) & (op2) & ~(rr)) | (~(op1) & ~(op2) & (rr)));  \
    C = B31(((op1) & (op2)) | (~rr & ((op1) | (op2))));


void
RunProgram(int startpc, int argc, char *argv[])
{
    int aci, ai;
    int instr, pc, xpc, npc;
    int i;  // Temporary for local stuff.
    int icount;

    icount = 0;
    pc = startpc; npc = pc + 4;
    i = MEMSIZE - 1024 + memoffset;  // Initial SP value.
    Reg[29] = i;                     // Initialize SP.
    // Setup argc and argv stuff (icky!)
    Store(i, argc);
    aci = i + 4;
    ai  = aci + 32;
    for (unsigned j = 0; j < argc; ++j) {
        strcpy((mem - memoffset) + ai, argv[j]);
        Store(aci, ai);
        aci += 4;
        ai += strlen(argv[j]) + 1;
    }

    for (;;) {
        icount++;
        xpc = pc;
        pc = npc;
        npc = pc + 4;
        instr = IFetch(xpc);
        Reg[0] = 0;  // Force r0 = 0.

        if (instr != 0)  // Eliminate no-ops.
        {
            switch ((instr >> 26) & 0x0000003f)
            {
                case I_SPECIAL:
                {
                    switch (instr & 0x0000003f)
                    {

                        case I_SLL:
                            Reg[rd(instr)] = Reg[rt(instr)] << shamt(instr);
                            break;
                        case I_SRL:
                            Reg[rd(instr)] = (unsigned) Reg[rt(instr)]
                                             >> shamt(instr);
                            break;
                        case I_SRA:
                            Reg[rd(instr)] = Reg[rt(instr)] >> shamt(instr);
                            break;
                        case I_SLLV:
                            Reg[rd(instr)] = Reg[rt(instr)]
                                             << Reg[rs(instr)];
                            break;
                        case I_SRLV:
                            Reg[rd(instr)] =
                            (unsigned) Reg[rt(instr)] >> Reg[rs(instr)];
                            break;
                        case I_SRAV:
                            Reg[rd(instr)] = Reg[rt(instr)]
                                             >> Reg[rs(instr)];
                            break;
                        case I_JR:
                            npc = Reg[rs(instr)];
                            break;
                        case I_JALR:
                            npc = Reg[rs(instr)];
                            Reg[rd(instr)] = xpc + 8;
                            break;

                        case I_SYSCALL:
                            SystemTrap();
                            break;
                        case I_BREAK:
                            SystemBreak();
                            break;

                        case I_MFHI:
                            Reg[rd(instr)] = HI;
                            break;
                        case I_MTHI:
                            HI = Reg[rs(instr)];
                            break;
                        case I_MFLO:
                            Reg[rd(instr)] = LO;
                            break;
                        case I_MTLO:
                            LO = Reg[rs(instr)];
                            break;

                        case I_MULT: {
                            int t1, t2;
                            int t1l, t1h, t2l, t2h;
                            int neg;

                            t1 = Reg[rs(instr)];
                            t2 = Reg[rt(instr)];
                            neg = 0;
                            if (t1 < 0) {
                                t1 = -t1;
                                neg = !neg;
                            }
                            if (t2 < 0) {
                                t2 = -t2;
                                neg = !neg;
                            }
                            LO = t1 * t2;
                            t1l = t1 & 0xFFFF;
                            t1h = (t1 >> 16) & 0xFFFF;
                            t2l = t2 & 0xFFFF;
                            t2h = (t2 >> 16) & 0xFFFF;
                            HI = t1h * t2h
                                 + (t1h * t2l >> 16) + (t2h * t1l >> 16);
                            if (neg) {
                                LO = ~LO;
                                HI = ~HI;
                                LO = LO + 1;
                                if (LO == 0)
                                    HI = HI + 1;
                            }
                            break;
                        }
                        case I_MULTU: {
                            int t1, t2;
                            int t1l, t1h, t2l, t2h;

                            t1 = Reg[rs(instr)];
                            t2 = Reg[rt(instr)];
                            t1l = t1 & 0xFFFF;
                            t1h = (t1 >> 16) & 0xFFFF;
                            t2l = t2 & 0xFFFF;
                            t2h = (t2 >> 16) & 0xFFFF;
                            LO = t1 * t2;
                            HI = t1h * t2h
                                 + (t1h * t2l >> 16) + (t2h * t1l >> 16);
                            break;
                        }
                        case I_DIV:
                            LO = Reg[rs(instr)] / Reg[rt(instr)];
                            HI = Reg[rs(instr)] % Reg[rt(instr)];
                            break;
                        case I_DIVU:
                            LO = (unsigned) Reg[rs(instr)]
                                 / (unsigned) Reg[rt(instr)];
                            HI = (unsigned) Reg[rs(instr)]
                                 % (unsigned) Reg[rt(instr)];
                            break;

                        case I_ADD:
                        case I_ADDU:
                            Reg[rd(instr)] = Reg[rs(instr)] + Reg[rt(instr)];
                            break;
                        case I_SUB:
                        case I_SUBU:
                            Reg[rd(instr)] = Reg[rs(instr)] - Reg[rt(instr)];
                            break;
                        case I_AND:
                            Reg[rd(instr)] = Reg[rs(instr)] & Reg[rt(instr)];
                            break;
                        case I_OR:
                            Reg[rd(instr)] = Reg[rs(instr)] | Reg[rt(instr)];
                            break;
                        case I_XOR:
                            Reg[rd(instr)] = Reg[rs(instr)] ^ Reg[rt(instr)];
                            break;
                        case I_NOR:
                            Reg[rd(instr)] = ~(Reg[rs(instr)]
                                               | Reg[rt(instr)]);
                            break;

                        case I_SLT:
                            Reg[rd(instr)] = Reg[rs(instr)] < Reg[rt(instr)];
                            break;
                        case I_SLTU:
                            Reg[rd(instr)] = (unsigned) Reg[rs(instr)]
                                             < (unsigned) Reg[rt(instr)];
                            break;
                        default:
                            unimplemented();
                            break;
                    }
                } break;

                case I_BCOND:
                {
                    switch (rt(instr))  // This field encodes the op.
                    {
                        case I_BLTZ:
                            if (Reg[rs(instr)] < 0)
                                npc = xpc + 4 + (immed(instr) << 2);
                            break;
                        case I_BGEZ:
                            if (Reg[rs(instr)] >= 0)
                                npc = xpc + 4 + (immed(instr) << 2);
                            break;

                        case I_BLTZAL:
                            Reg[31] = xpc + 8;
                            if (Reg[rs(instr)] < 0)
                                npc = xpc + 4 + (immed(instr) << 2);
                            break;
                        case I_BGEZAL:
                            Reg[31] = xpc + 8;
                            if (Reg[rs(instr)] >= 0)
                                npc = xpc + 4 + (immed(instr) << 2);
                            break;
                        default:
                            unimplemented();
                            break;
                    }

                } break;

                case I_J:
                    npc = (xpc & 0xf0000000) | ((instr & 0x03FFFFFF) << 2);
                    break;
                case I_JAL:
                    Reg[31] = xpc + 8;
                    npc = (xpc & 0xf0000000) | ((instr & 0x03FFFFFF) << 2);
                    break;
                case I_BEQ:
                    if (Reg[rs(instr)] == Reg[rt(instr)])
                        npc = xpc + 4 + (immed(instr) << 2);
                    break;
                case I_BNE:
                    if (Reg[rs(instr)] != Reg[rt(instr)])
                        npc = xpc + 4 + (immed(instr) << 2);
                    break;
                case I_BLEZ:
                    if (Reg[rs(instr)] <= 0)
                        npc = xpc + 4 + (immed(instr) << 2);
                    break;
                case I_BGTZ:
                    if (Reg[rs(instr)] > 0)
                        npc = xpc + 4 + (immed(instr) << 2);
                    break;
                case I_ADDI:
                    Reg[rt(instr)] = Reg[rs(instr)] + immed(instr);
                    break;
                case I_ADDIU:
                    Reg[rt(instr)] = Reg[rs(instr)] + immed(instr);
                    break;
                case I_SLTI:
                    Reg[rt(instr)] = Reg[rs(instr)] < immed(instr);
                    break;
                case I_SLTIU:
                    Reg[rt(instr)] = (unsigned) Reg[rs(instr)]
                                     < (unsigned) immed(instr);
                    break;
                case I_ANDI:
                    Reg[rt(instr)] = Reg[rs(instr)] & immed(instr);
                    break;
                case I_ORI:
                    Reg[rt(instr)] = Reg[rs(instr)] | immed(instr);
                    break;
                case I_XORI:
                    Reg[rt(instr)] = Reg[rs(instr)] ^ immed(instr);
                    break;
                case I_LUI:
                    Reg[rt(instr)] = instr << 16;
                    break;

                case I_LB:
                    Reg[rt(instr)] = CFetch(Reg[rs(instr)] + immed(instr));
                    break;
                case I_LH:
                    Reg[rt(instr)] = SFetch(Reg[rs(instr)] + immed(instr));
                    break;
                case I_LWL:
                    i = Reg[rs(instr)] + immed(instr);
                    Reg[rt(instr)] &= -1 >> 8 * (-i & 0x03);
                    Reg[rt(instr)] |= Fetch(i & 0xFFFFFFFC)
                                      << 8 * (i & 0x03);
                    break;
                case I_LW:
                    Reg[rt(instr)] = Fetch(Reg[rs(instr)] + immed(instr));
                    break;
                case I_LBU:
                    Reg[rt(instr)] = UCFetch(Reg[rs(instr)] + immed(instr));
                    break;
                case I_LHU:
                    Reg[rt(instr)] = USFetch(Reg[rs(instr)] + immed(instr));
                    break;
                case I_LWR:
                    i = Reg[rs(instr)] + immed(instr);
                    Reg[rt(instr)] &= -1 << 8 * (i & 0x03);
                    if ((i & 0x03) == 0)
                        Reg[rt(instr)] = 0;
                    Reg[rt(instr)] |=
                      (Fetch(i & 0xFFFFFFFC) >> 8 * (-i & 0x03));
                    break;

                case I_SB:
                    CStore(Reg[rs(instr)] + immed(instr), Reg[rt(instr)]);
                    break;
                case I_SH:
                    SStore(Reg[rs(instr)] + immed(instr), Reg[rt(instr)]);
                    break;
                case I_SWL:
                    fprintf(stderr, "sorry, no SWL yet.\n");
                    unimplemented();
                    break;
                case I_SW:
                    Store(Reg[rs(instr)] + immed(instr), Reg[rt(instr)]);
                    break;

                case I_SWR:
                    fprintf(stderr, "sorry, no SWR yet.\n");
                    unimplemented();
                    break;

                case I_LWC0: case I_LWC1:
                case I_LWC2: case I_LWC3:
                case I_SWC0: case I_SWC1:
                case I_SWC2: case I_SWC3:
                case I_COP0: case I_COP1:
                case I_COP2: case I_COP3:
                    fprintf(stderr, "Sorry, no coprocessors.\n");
                    exit(2);
                    break;

                default:
                    unimplemented();
                    break;
            }
        }

#ifdef DEBUG
        //printf(" %d(%X) = %d(%X) op  %d(%X)\n",
        //       Reg[rd], Reg[rd], op1, op1, op2, op2);
#endif
#if !FAST
        if (TRACE) {
            DumpAscii(instr, xpc);
            printf("\n");
            if (Regtrace)
                DumpReg();
        }
#endif
    }
}

/// Unimplemented.
static void
unimplemented(void)
{
    printf("Unimplemented instruction.\n");
    exit(2);
}

/// Not implemented yet.
static void
ny(void)
{
    printf("This opcode is not implemeted yet.\n");
    exit(2);
}


// Debug aids.

static inline int
RS(int i)
{
    return rs(i);
}

static inline int
RT(int i)
{
    return rt(i);
}

static inline int
RD(int i)
{
    return rd(i);
}

static inline int
IM(int i)
{
    return immed(i);
}

void
DumpReg(void)
{
    int j;

    printf(" 0:");
    for (j = 0; j < 8; j++)
        printf(" %08x", Reg[j]);
    printf("\n");
    printf(" 8:");
    for (; j < 16; j++)
        printf(" %08x", Reg[j]);
    printf("\n");
    printf("16:");
    for (; j < 24; j++)
        printf(" %08x", Reg[j]);
    printf("\n");
    printf("24:");
    for (; j < 32; j++)
        printf(" %08x", Reg[j]);
    printf("\n");
}

// 0 -> 0
// 1 -> 1
// 2 -> 1
// 3 -> 2
// 4 -> 2
// 5 -> 2
// 6 -> 2
// 7 -> 3
// 8 -> 3
// 9 -> 3  ...
// Treats all ints as unsigned numbers.
static int
ilog2(int i)
{
    int j, l;

    if (i == 0)
        return 0;
    j = 0;
    l = 1;
    if ((j = (i & 0xffff0000)) != 0) {
        i = j;
        l += 16;
    }
    if ((j = (i & 0xff00ff00)) != 0) {
        i = j;
        l += 8;
    }
    if ((j = (i & 0xf0f0f0f0)) != 0) {
        i = j;
        l += 4;
    }
    if ((j = (i & 0xcccccccc)) != 0) {
        i = j;
        l += 2;
    }
    if ((j = (i & 0xaaaaaaaa)) != 0) {
        i = j;
        l += 1;
    }
    return l;
}



#define NH   32
#define NNN  33

static int hists[NH][NNN];
int        hoflo[NH], htotal[NH];

static void
henters(int n, int hist)
{
    if (0 <= n && n < NNN)
        hists[hist][n]++;
    else
        hoflo[hist]++;
    htotal[hist]++;
}

void
hprint(void)
{
    double I;

    for (unsigned h = 0; h <= NH; h++)
        if (htotal[h] > 0) {
            printf("\nhisto %d:\n", h);
            I = 0.0;
            for (unsigned i = 0; i < NNN; i++) {
                I += hists[h][i];
                printf("%d\t%d\t%5.2f%%\t%5.2f%%\n",
                       i, hists[h][i],
                       (double) 100 * hists[h][i] / htotal[h],
                       (double) 100 * I / htotal[h]);
            }
            printf("oflo %d:\t%d/%d\t%5.2f%%\n",
                   h, hoflo[h], htotal[h],
                   (double) 100 * hoflo[h] / htotal[h]);
        }
}

int numadds = 1, numsubs = 1, numsuccesses, numcarries;
int addtable[33][33];
int subtable[33][33];

static char fmt[]  = "%6u";
static char fmt2[] = "------";

void
patable(int tab[33][33])
{
    printf("  |");
    for (unsigned j = 0; j < 33; j++)
        printf(fmt, j);
    printf("\n  |");
    for (unsigned j = 0; j < 33; j++)
        printf(fmt2);
    printf("\n");
    for (unsigned i = 0; i < 33; i++) {
        printf("%2d|", i);
        for (unsigned j = 0; j < 33; j++)
            printf(fmt, tab[i][j]);
        printf("\n");
    }
}

void
printstatistics(void)
{
    //printhist();
    //printf("numjmpls = %d / %d = %5.2f%%\n",
    //       numjmpls, arch1cycles, 100.0 * numjmpls / arch1cycles);
    //printf("numadds = %d, numsubs = %d, numcycles = %d, frac = %5.2f%%\n",
    //       numadds, numsubs,
    //       arch1cycles, (double) 100 * (numadds + numsubs) / arch1cycles);
    //printf("numsuccesses = %d (%5.2f%%) numcarries = %d\n",
    //       numsuccesses, 100.0 * numsuccesses / (numadds + numsubs),
    //       numcarries);

    //hprint();
    //printf("\nADD table:\n");
    //patable(addtable);
    //printf("\nSUB table:\n");
    //patable(subtable);
}


#define NNNN  64

static int hist[NNNN];

static void
henter(int n)
{
    if (0 <= n && n < NNNN)
        ++hist[n];
}

static void
printhist(void)
{
    for (unsigned i = 0; i < NNNN; ++i)
        printf("%d %d\n", i, hist[i]);
}
