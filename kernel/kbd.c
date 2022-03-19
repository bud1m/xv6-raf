#include "kbd.h"
#include "defs.h"
#include "types.h"
#include "x86.h"

int kbdgetc(void) {
    static uint shift;
    static uchar *charcode[4] = {normalmap, shiftmap, ctlmap, ctlmap};
    uint st, data, c;

    st = inb(KBSTATP);
    if ((st & KBS_DIB) == 0) return -1;
    data = inb(KBDATAP);

    // ulazi samo kad je home
    if (data == 0xE0) {
        // shift setuje 6 bit na 1 (tj kao da je escape pritisnut)
        shift |= E0ESC;
        return 0;
    }
    // ulazi ako nije prinisnut karakter
    else if (data & 0x80) {
        // Key released
        data = (shift & E0ESC ? data : data & 0x7F);
        shift &= ~(shiftcode[data] | E0ESC);
        return 0;
    } else if (shift & E0ESC) {
        // Last character was an E0 escape; or with 0x80
        data |= 0x80;
        shift &= ~E0ESC;
    }

    shift |= shiftcode[data];
    shift ^= togglecode[data];

    c = charcode[shift & (CTL | SHIFT)][data];

    // ulazi u mod
    if (c == 'C' && (shift & (SHIFT | ALT))) {
        c = SPECIAL_CODE('C');
    }

    // izlazi iz moda
    if (c == 'P' && (shift & (SHIFT | ALT))) {
        c = SPECIAL_CODE('P');
    }

    if (shift & CAPSLOCK) {
        if ('a' <= c && c <= 'z')
            c += 'A' - 'a';
        else if ('A' <= c && c <= 'Z')
            c += 'a' - 'A';
    }
    return c;
}

void kbdintr(void) { consoleintr(kbdgetc); }
