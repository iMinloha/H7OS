/*------------------------------------------------------------------------*/
/* Unicode support functions for FatFs (R0.12c)                           */
/* Code page 437 (U.S. OEM)                                               */
/*------------------------------------------------------------------------*/

#include "../ff.h"

#if _USE_LFN != 0

/* OEM <-> Unicode translation tables are omitted for now.
   ASCII subset (0-127) passes through directly, which is sufficient
   for English file names. Full CP437 tables can be added later. */

WCHAR ff_convert (WCHAR src, UINT dir)
{
    /* dir: 0 = Unicode -> OEM, 1 = OEM -> Unicode */
    if (src < 0x80) return src;  /* ASCII subset: 1:1 mapping */
    /* Non-ASCII characters: pass through for now */
    return src;
}

WCHAR ff_wtoupper (WCHAR chr)
{
    if (chr >= 'a' && chr <= 'z') return chr - 0x20;  /* a-z -> A-Z */
    return chr;
}

#endif /* _USE_LFN */
