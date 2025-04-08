#pragma once
#include <ltbase.h>

struct INUPACKED HaliX86IdtPointer
{
    UINT16 limit;
    UINT32 base;
};

struct INUPACKED HaliX86InterruptDescriptor
{
    UINT16 baseLow;
    UINT16 selector;
    BYTE zero;
    BYTE typeAttributes;
    UINT16 baseHigh;
};

struct INUPACKED HaliX86InterruptFrame
{
    BYTE fpu_state[108];
    UINT32 gs, fs, es, ds;
    UINT32 edi, esi, ebp, esp, ebx, edx, ecx, eax;
    UINT32 cr0, cr2, cr3, cr4;
    UINT32 dr0, dr1, dr2, dr3, dr6, dr7;
    UINT32 int_no, err_code;
    UINT32 eip, cs, eflags, user_esp, user_ss;
};

struct INUPACKED HaliX86GdtEntry
{
    UINT16 limiteLow;
    UINT16 baseLow;
    BYTE baseMiddle;
    BYTE access;
    BYTE granularity;
    BYTE baseHigh;
};

struct INUPACKED HaliX86GdtPointer
{
    UINT16 limit;
    UINT32 base;
};

struct INUPACKED HaliX86Tss
{
    UINT32 prevTss;
    UINT32 esp0;
    UINT32 ss0;
    UINT32 esp1;
    UINT32 ss1;
    UINT32 esp2;
    UINT32 ss2;
    UINT32 cr3;
    UINT32 eip;
    UINT32 eflags;
    UINT32 eax;
    UINT32 ecx;
    UINT32 edx;
    UINT32 ebx;
    UINT32 esp;
    UINT32 ebp;
    UINT32 esi;
    UINT32 edi;
    UINT32 es;
    UINT32 cs;
    UINT32 ss;
    UINT32 ds;
    UINT32 fs;
    UINT32 gs;
    UINT32 ldt;
    UINT16 trap;
    UINT16 iomap;
};

VOID HaliIsrHandler(struct HaliX86InterruptFrame* descriptor);
VOID HaliIrqHandler(struct HaliX86InterruptFrame* descriptor);

VOID HaliX86InitializeGdt();
VOID HaliX86InitializeInterrupts();
VOID HaliX86InitializeIdt();
VOID HaliX86IntSetIsr(BYTE index, UINT32 base, UINT16 selector, BYTE flags);
VOID HaliX86LoadIdtPointer(struct HaliX86IdtPointer *ptr);
VOID HaliX86SetGdtEntry(const INT32 index, const UINT32 base, const UINT32 limit, const BYTE access, const BYTE granularity);