// -----------------------------------------------------------------------
// $Id: hsconf.h,v 1.9 2006/04/27 15:49:52 mark Exp $
// -----------------------------------------------------------------------

#ifndef __HSCONF_INCLUDED__
#define __HSCONF_INCLUDED__

//! The configuration file for HSpace lives here off the game directory
#define HSPACE_CONFIG_FILE "space/hspace.cnf"

#include "hstypes.h"

// A listing in the configuration array
typedef struct
{
    HS_INT8 *name;
    void (*func) (HS_INT32 *, HS_INT8 *);
    HS_INT32 *des;
} HCONF;

//! Configuration storage class
class CHSConf
{
  public:

    //! Setup default values
    CHSConf();

    //! Load data from the specified configuration file
    HS_BOOL8 LoadConfigFile(HS_INT8 *lpstrPath);

    //! Boolean marker indicating if the config file has been loaded
    HS_BOOL8 m_bConfigLoaded;

    // Attributes
    HS_INT32 admin_server;
    HS_INT32 admin_server_port;
    HS_INT32 admin_server_threaded;
    HS_INT32 afterburn_fuel_ratio;
    HS_INT32 afterburn_ratio;
    HS_DBREF afterworld;
    HS_INT32 asteroid_size_multiplier;
    HS_INT32 autostart;
    HS_INT32 autozone;
    HS_INT8  classdb[256];
    HS_INT32 cyc_interval;
    HS_DBREF space_wiz;
    HS_INT32 damage_repair_time;
    HS_INT32 detectdist;
    HS_INT32 forbid_puppets;
    HS_INT32 fuel_ratio;
    HS_INT32 identdist;
    HS_INT32 jump_speed_multiplier;
    HS_INT32 log_commands;
    HS_INT32 max_sensor_range;
    HS_INT32 max_land_speed;
    HS_INT32 max_dock_dist;
    HS_INT32 max_dock_size;
    HS_INT32 max_drop_dist;
    HS_INT32 max_board_dist;
    HS_INT32 max_gate_dist;
    HS_INT32 nebula_size_multiplier;
    HS_INT32 notify_shipscan;
    HS_INT8  objectdb[256];
    HS_INT8  picture_dir[256];
    HS_INT32 seconds_to_drop;
    HS_INT32 sense_hypervessels;
    HS_INT8  territorydb[256];
    HS_INT8  unit_name[32];
    HS_INT8  univdb[256];
    HS_INT32 use_comm_objects;
    HS_INT32 use_two_fuels;
    HS_FLOAT32 warp_exponent;
    HS_INT32 warp_constant;
    HS_INT8  weapondb[256];

    /* Messages */
    HS_INT8 afterburn_disengage[256];
    HS_INT8 afterburn_engage[256];
    HS_INT8 begin_descent[256];
    HS_INT8 computer_activating[256];
    HS_INT8 end_jump[256];
    HS_INT8 engines_activating[256];
    HS_INT8 engines_cut[256];
    HS_INT8 engine_forward[256];
    HS_INT8 engines_offline[256];
    HS_INT8 engine_reverse[256];
    HS_INT8 ship_is_jumping[256];
    HS_INT8 landing_msg[256];
    HS_INT8 life_activating[256];
    HS_INT8 life_cut[256];
    HS_INT8 lift_off[256];
    HS_INT8 reactor_activating[256];
    HS_INT8 reactor_offline[256];
    HS_INT8 jumpers_cut[256];
    HS_INT8 sensors_cut[256];
    HS_INT8 ship_is_docked[256];
    HS_INT8 ship_is_docking[256];
    HS_INT8 ship_is_undocking[256];
    HS_INT8 ship_jumps[256];
    HS_INT8 speed_decrease[256];
    HS_INT8 speed_halt[256];
    HS_INT8 speed_increase[256];
    HS_INT8 thrusters_activating[256];

    HS_BOOL8 InputOption(HS_INT8 *, HS_INT8 *);

};

extern CHSConf HSCONF;

#endif
