// -----------------------------------------------------------------------
// $Id: hsflags.h,v 1.5 2006/04/04 12:41:36 mark Exp $
// -----------------------------------------------------------------------

#if !defined(__HSFLAGS_H__)
#define __HSFLAGS_H__

// Flag definitions for PennMUSH
#ifdef PENNMUSH

#define THING_HSPACE_OBJECT			"HSPACE_OBJECT"
#define THING_HSPACE_CONSOLE		"HSPACE_CONSOLE"
#define THING_HSPACE_TERRITORY		"HSPACE_TERRITORY"
#define THING_HSPACE_C_LOCKED		"HSPACE_C_LOCKED"
#define THING_HSPACE_COMM			"HSPACE_COMM"
#define THING_HSPACE_SIM			"HSPACE_SIM"

#define ROOM_HSPACE_LANDINGLOC		"HSPACE_LANDINGLOC"
#define ROOM_HSPACE_UNIVERSE		"HSPACE_UNIVERSE"
#define ROOM_FLOATING				"FLOATING"

#define EXIT_HSPACE_HATCH			"HSPACE_HATCH"

#define PLAYER_HSPACE_ADMIN			"HSPACE_ADMIN"
#define PLAYER_CONNECT				"CONNECTED"

#define THING_HSPACE_SIM_CHAR		'^'
#define THING_HSPACE_OBJECT_CHAR	'@'
#define THING_HSPACE_CONSOLE_CHAR	'#'
#define THING_HSPACE_TERRITORY_CHAR	'$'
#define THING_HSPACE_C_LOCKED_CHAR	'%'
#define THING_HSPACE_COMM_CHAR		'&'

#define ROOM_HSPACE_LANDINGLOC_CHAR	'^'
#define ROOM_HSPACE_UNIVERSE_CHAR	'*'

#define EXIT_HSPACE_HATCH_CHAR		'('
#define PLAYER_HSPACE_ADMIN_CHAR	')'

#endif // PENNMUSH

#if defined(TM3) || defined(MUX)
// Map HSPACE flags onto the MARKER set of flags
#define HSPACE_HATCH            MARK_0
#define HSPACE_UNIVERSE         MARK_1
#define HSPACE_LANDINGLOC       MARK_2
#define HSPACE_SIM              MARK_3
#define HSPACE_C_LOCKED         MARK_4
#define HSPACE_TERRITORY        MARK_5
#define HSPACE_COMM             MARK_6
#define HSPACE_CONSOLE          MARK_7
#define HSPACE_OBJECT           MARK_8

/* HSpace uses Penn type-specific flags, we alias here */
#define EXIT_HSPACE_HATCH       HSPACE_HATCH
#define ROOM_HSPACE_UNIVERSE    HSPACE_UNIVERSE
#define ROOM_HSPACE_LANDINGLOC  HSPACE_LANDINGLOC
#define THING_HSPACE_SIM        HSPACE_SIM
#define THING_HSPACE_C_LOCKED   HSPACE_C_LOCKED
#define THING_HSPACE_TERRITORY  HSPACE_TERRITORY
#define THING_HSPACE_COMM       HSPACE_COMM
#define THING_HSPACE_CONSOLE    HSPACE_CONSOLE
#define THING_HSPACE_OBJECT     HSPACE_OBJECT

#endif // TM3 and MUX

#endif // __HSFLAGS_H__
