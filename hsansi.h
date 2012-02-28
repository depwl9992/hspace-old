// -----------------------------------------------------------------------
// $Id: hsansi.h,v 1.3 2006/04/04 12:41:36 mark Exp $
// -----------------------------------------------------------------------

/* ANSI control codes for various neat-o terminal effects

 * Some older versions of Ultrix don't appear to be able to
 * handle these escape sequences. If lowercase 'a's are being
 * stripped from @doings, and/or the output of the ANSI flag
 * is screwed up, you have the Ultrix problem.
 *
 * To fix the ANSI problem, try replacing the '\x1B' with '\033'.
 * To fix the problem with 'a's, replace all occurrences of '\a'
 * in the code with '\07'.
 *
 */

//! @brief ANSI sequences for generating colorized displays

#ifndef __HSANSI_H
#define __HSANSI_H

#define ANSI_BLACK_V    (30)
#define ANSI_RED_V      (31)
#define ANSI_GREEN_V    (32)
#define ANSI_YELLOW_V   (33)
#define ANSI_BLUE_V     (34)
#define ANSI_MAGENTA_V  (35)
#define ANSI_CYAN_V     (36)
#define ANSI_WHITE_V    (37)

#ifndef OLD_ANSI

#define BEEP_CHAR     '\a'
#define ESC_CHAR      '\x1B'

#define ANSI_BEGIN   "\x1B["

#define ANSI_NORMAL   "\x1B[0m"

#define ANSI_HILITE   "\x1B[1m"
#define ANSI_INVERSE  "\x1B[7m"
#define ANSI_BLINK    "\x1B[5m"
#define ANSI_UNDERSCORE "\x1B[4m"

#define ANSI_INV_BLINK         "\x1B[7;5m"
#define ANSI_INV_HILITE        "\x1B[1;7m"
#define ANSI_BLINK_HILITE      "\x1B[1;5m"
#define ANSI_INV_BLINK_HILITE  "\x1B[1;5;7m"

/* Foreground colors */

#define ANSI_BLACK      "\x1B[30m"
#define ANSI_RED        "\x1B[31m"
#define ANSI_GREEN      "\x1B[32m"
#define ANSI_YELLOW     "\x1B[33m"
#define ANSI_BLUE       "\x1B[34m"
#define ANSI_MAGENTA    "\x1B[35m"
#define ANSI_CYAN       "\x1B[36m"
#define ANSI_WHITE      "\x1B[37m"

/* Background colors */

#define ANSI_BBLACK     "\x1B[40m"
#define ANSI_BRED       "\x1B[41m"
#define ANSI_BGREEN     "\x1B[42m"
#define ANSI_BYELLOW    "\x1B[43m"
#define ANSI_BBLUE      "\x1B[44m"
#define ANSI_BMAGENTA   "\x1B[45m"
#define ANSI_BCYAN      "\x1B[46m"
#define ANSI_BWHITE     "\x1B[47m"

#else

#define BEEP_CHAR     '\07'
#define ESC_CHAR      '\033'

#define ANSI_NORMAL   "\033[0m"
#define ANSI_BEGIN    "\033["

#define ANSI_HILITE   "\033[1m"
#define ANSI_INVERSE  "\033[7m"
#define ANSI_BLINK    "\033[5m"
#define ANSI_UNDERSCORE "\033[4m"

#define ANSI_INV_BLINK         "\033[7;5m"
#define ANSI_INV_HILITE        "\033[1;7m"
#define ANSI_BLINK_HILITE      "\033[1;5m"
#define ANSI_INV_BLINK_HILITE  "\033[1;5;7m"

/* Foreground colors */

#define ANSI_BLACK      "\033[30m"
#define ANSI_RED        "\033[31m"
#define ANSI_GREEN      "\033[32m"
#define ANSI_YELLOW     "\033[33m"
#define ANSI_BLUE       "\033[34m"
#define ANSI_MAGENTA    "\033[35m"
#define ANSI_CYAN       "\033[36m"
#define ANSI_WHITE      "\033[37m"

/* Background colors */

#define ANSI_BBLACK     "\033[40m"
#define ANSI_BRED       "\033[41m"
#define ANSI_BGREEN     "\033[42m"
#define ANSI_BYELLOW    "\033[43m"
#define ANSI_BBLUE      "\033[44m"
#define ANSI_BMAGENTA   "\033[45m"
#define ANSI_BCYAN      "\033[46m"
#define ANSI_BWHITE     "\033[47m"

#endif

#define ANSI_END        "m"

#endif /* __HSANSI_H */
