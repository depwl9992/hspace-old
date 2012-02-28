// -----------------------------------------------------------------------
// $Id: hscmds.h,v 1.4 2007/08/19 17:41:27 grapenut_org Exp $
// -----------------------------------------------------------------------

#ifndef __HSCMDS_INCLUDED__
#define __HSCMDS_INCLUDED__

// Command permissions
//! Only wizards are allowed to use this command
#define HCP_WIZARD	0x1
//! Any object can use this command
#define HCP_ANY		0x2
//! Only God (#1) can use this command
#define HCP_GOD		0x4

//! Prototype command declaration macro
#define HSPACE_COMMAND_PROTO(x)	void x(int, char *, char *);
//! Command macro prototype
#define HSPACE_COMMAND_HDR(x)	void x(int player, char *arg_left, char *arg_right)

//! A command entry in the array
typedef struct
{
    char *key;
    void (*func) (int, char *, char *);
    int perms;
} HSPACE_COMMAND;

extern HSPACE_COMMAND hsSpaceCommandArray[];
extern HSPACE_COMMAND hsNavCommandArray[];
extern HSPACE_COMMAND hsEngCommandArray[];
extern HSPACE_COMMAND hsConCommandArray[];
extern HSPACE_COMMAND *hsFindCommand(const char *, HSPACE_COMMAND *);

#endif
