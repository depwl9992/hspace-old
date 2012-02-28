// -----------------------------------------------------------------------
// $Id: hsconf.cpp,v 1.10 2006/04/27 15:49:52 mark Exp $
// -----------------------------------------------------------------------

#include <stdlib.h>
#include <cstring>

#include "hstypes.h"
#include "hsutils.h"
#include "hsconf.h"

CHSConf HSCONF;

void HSCfInt(HS_INT32 *, HS_INT8 *);
void HSCfStr(HS_INT32 *, HS_INT8 *);
void HSCfFloat(HS_INT32 * des, HS_INT8 * value);

HCONF hspace_conflist[] = {
    {(HS_INT8 *) "admin_server", HSCfInt, &HSCONF.admin_server},
    {(HS_INT8 *) "admin_server_port", HSCfInt, &HSCONF.admin_server_port},
    {(HS_INT8 *) "admin_server_threaded", HSCfInt, 
        &HSCONF.admin_server_threaded},
    {(HS_INT8 *) "afterburn_engage", HSCfStr, 
        (HS_INT32 *) HSCONF.afterburn_engage},
    {(HS_INT8 *) "afterburn_disengage", HSCfStr,
     (HS_INT32 *) HSCONF.afterburn_disengage},
    {(HS_INT8 *) "afterburn_ratio", HSCfInt,
     &HSCONF.afterburn_ratio},
    {(HS_INT8 *) "afterburn_fuel_ratio", HSCfInt,
     &HSCONF.afterburn_fuel_ratio},
    {(HS_INT8 *) "afterworld", HSCfInt,
     &HSCONF.afterworld},
    {(HS_INT8 *) "asteroid_size_multiplier", HSCfInt,
     &HSCONF.asteroid_size_multiplier},
    {(HS_INT8 *) "autostart", HSCfInt,
     (HS_INT32 *) & HSCONF.autostart},
    {(HS_INT8 *) "autozone", HSCfInt,
     (HS_INT32 *) & HSCONF.autozone},
    {(HS_INT8 *) "begin_descent", HSCfStr,
     (HS_INT32 *) HSCONF.begin_descent},
    {(HS_INT8 *) "classdb", HSCfStr,
     (HS_INT32 *) HSCONF.classdb},
    {(HS_INT8 *) "computer_activating", HSCfStr,
     (HS_INT32 *) HSCONF.computer_activating},
    {(HS_INT8 *) "cycle_interval", HSCfInt,
     &HSCONF.cyc_interval},
    {(HS_INT8 *) "damage_repair_time", HSCfInt,
     &HSCONF.damage_repair_time},
    {(HS_INT8 *) "detectdist", HSCfInt,
     &HSCONF.detectdist},
    {(HS_INT8 *) "end_jump", HSCfStr,
     (HS_INT32 *) HSCONF.end_jump},
    {(HS_INT8 *) "engine_forward", HSCfStr,
     (HS_INT32 *) HSCONF.engine_forward},
    {(HS_INT8 *) "engine_reverse", HSCfStr,
     (HS_INT32 *) HSCONF.engine_reverse},
    {(HS_INT8 *) "engines_activating", HSCfStr,
     (HS_INT32 *) HSCONF.engines_activating},
    {(HS_INT8 *) "engines_cut", HSCfStr,
     (HS_INT32 *) HSCONF.engines_cut},
    {(HS_INT8 *) "engines_offline", HSCfStr,
     (HS_INT32 *) HSCONF.engines_offline},
    {(HS_INT8 *) "forbid_puppets", HSCfInt,
     &HSCONF.forbid_puppets},
    {(HS_INT8 *) "fuel_ratio", HSCfInt,
     (HS_INT32 *) & HSCONF.fuel_ratio},
    {(HS_INT8 *) "identdist", HSCfInt,
     &HSCONF.identdist},
    {(HS_INT8 *) "jumpers_cut", HSCfStr,
     (HS_INT32 *) HSCONF.jumpers_cut},
    {(HS_INT8 *) "jump_speed_multiplier", HSCfInt,
     &HSCONF.jump_speed_multiplier},
    {(HS_INT8 *) "landing_msg", HSCfStr,
     (HS_INT32 *) HSCONF.landing_msg},
    {(HS_INT8 *) "life_activating", HSCfStr,
     (HS_INT32 *) HSCONF.life_activating},
    {(HS_INT8 *) "life_cut", HSCfStr,
     (HS_INT32 *) HSCONF.life_cut},
    {(HS_INT8 *) "lift_off", HSCfStr,
     (HS_INT32 *) HSCONF.lift_off},
    {(HS_INT8 *) "log_commands", HSCfInt,
     (HS_INT32 *) & HSCONF.log_commands},
    {(HS_INT8 *) "max_board_dist", HSCfInt,
     &HSCONF.max_board_dist},
    {(HS_INT8 *) "max_dock_dist", HSCfInt,
     &HSCONF.max_dock_dist},
    {(HS_INT8 *) "max_dock_size", HSCfInt,
     &HSCONF.max_dock_size},
    {(HS_INT8 *) "max_drop_dist", HSCfInt,
     &HSCONF.max_drop_dist},
    {(HS_INT8 *) "max_gate_dist", HSCfInt,
     &HSCONF.max_gate_dist},
    {(HS_INT8 *) "max_land_speed", HSCfInt,
     &HSCONF.max_land_speed},
    {(HS_INT8 *) "max_sensor_range", HSCfInt,
     &HSCONF.max_sensor_range},
    {(HS_INT8 *) "nebula_size_multiplier", HSCfInt,
     &HSCONF.nebula_size_multiplier},
    {(HS_INT8 *) "notify_shipscan", HSCfInt,
     &HSCONF.notify_shipscan},
    {(HS_INT8 *) "objectdb", HSCfStr,
     (HS_INT32 *) HSCONF.objectdb},
    {(HS_INT8 *) "picture_dir", HSCfStr,
     (HS_INT32 *) HSCONF.picture_dir},
    {(HS_INT8 *) "reactor_activating", HSCfStr,
     (HS_INT32 *) HSCONF.reactor_activating},
    {(HS_INT8 *) "reactor_offline", HSCfStr,
     (HS_INT32 *) HSCONF.reactor_offline},
    {(HS_INT8 *) "ship_jumps", HSCfStr,
     (HS_INT32 *) HSCONF.ship_jumps},
    {(HS_INT8 *) "seconds_to_drop", HSCfInt,
     &HSCONF.seconds_to_drop},
    {(HS_INT8 *) "sensors_cut", HSCfStr,
     (HS_INT32 *) HSCONF.sensors_cut},
    {(HS_INT8 *) "sense_hypervessels", HSCfInt,
     &HSCONF.sense_hypervessels},
    {(HS_INT8 *) "ship_is_docked", HSCfStr,
     (HS_INT32 *) HSCONF.ship_is_docked},
    {(HS_INT8 *) "ship_is_docking", HSCfStr,
     (HS_INT32 *) HSCONF.ship_is_docking},
    {(HS_INT8 *) "ship_is_undocking", HSCfStr,
     (HS_INT32 *) HSCONF.ship_is_undocking},
    {(HS_INT8 *) "ship_is_jumping", HSCfStr,
     (HS_INT32 *) HSCONF.ship_is_jumping},
    {(HS_INT8 *) "space_wiz", HSCfInt,
        &HSCONF.space_wiz},
    {(HS_INT8 *) "speed_decrease", HSCfStr,
     (HS_INT32 *) HSCONF.speed_decrease},
    {(HS_INT8 *) "speed_halt", HSCfStr,
     (HS_INT32 *) HSCONF.speed_halt},
    {(HS_INT8 *) "speed_increase", HSCfStr,
     (HS_INT32 *) HSCONF.speed_increase},
    {(HS_INT8 *) "territorydb", HSCfStr,
     (HS_INT32 *) HSCONF.territorydb},
    {(HS_INT8 *) "thrusters_activating", HSCfStr,
     (HS_INT32 *) HSCONF.thrusters_activating},
    {(HS_INT8 *) "unit_name", HSCfStr,
     (HS_INT32 *) HSCONF.unit_name},
    {(HS_INT8 *) "univdb", HSCfStr,
     (HS_INT32 *) HSCONF.univdb},
    {(HS_INT8 *) "use_comm_objects", HSCfInt,
        &HSCONF.use_comm_objects},
    {(HS_INT8 *) "use_two_fuels", HSCfInt,
     (HS_INT32 *) &HSCONF.use_two_fuels},
    {(HS_INT8 *) "warp_exponent", HSCfFloat,
        (HS_INT32*) &HSCONF.warp_exponent},
    {(HS_INT8 *) "warp_constant", HSCfInt,
        (HS_INT32*) &HSCONF.warp_constant},
    {(HS_INT8 *) "weapondb", HSCfStr,
     (HS_INT32 *) HSCONF.weapondb},
    {NULL, NULL, NULL}
};

CHSConf::CHSConf(void):m_bConfigLoaded(false)
{
    // Defaults for string parameters
    strcpy_s(afterburn_disengage,
           "The ship shakes as the afterburners are disengaged.");
    strcpy_s(afterburn_engage,
           "The ship shudders as the afterburners are engaged.");
    strcpy_s(begin_descent, "The ship shudders as the drop rockets engage.");
    strcpy_s(classdb, "space/classdb");
    strcpy_s(computer_activating,
           "Computer terminals all around power up as the systems is powered.");
    strcpy_s(end_jump, "The ship reverberates as it drops out of hyperspace.");
    strcpy_s(engines_activating,
           "You feel a sudden rumble as the engines activate.");
    strcpy_s(engines_cut,
           "The incessant hum from the ship's engines suddenly fades away.");
    strcpy_s(engine_forward,
           "You feel a sudden force as the main engines engage.");
    strcpy_s(engines_offline, "Engines are currently offline.");
    strcpy_s(engine_reverse,
           "You feel a sudden force as the reverse thrusters engage.");
    strcpy_s(jumpers_cut,
           "You hear a large powerdrop as the jump drives power down.");
    strcpy_s(landing_msg,
           "The ship sways and bumps as it makes contact with the ground.");
    strcpy_s(life_activating,
           "A gentle hum can be heard as life support is activated.");
    strcpy_s(life_cut,
           "The air suddenly becomes stale as life support loses power.");
    strcpy_s(lift_off,
           "The ship sways and bumps as it lifts from the surface.");
    strcpy_s(objectdb, "space/objectdb");
    strcpy_s(picture_dir, "space/pics");
    strcpy_s(reactor_activating,
           "Lights around you flicker on as the main reactor powers up.");
    strcpy_s(reactor_offline, "Main reactor is currently offline.");
    strcpy_s(sensors_cut, "");
    strcpy_s(speed_decrease, "You feel the ship begin to slow.");
    strcpy_s(speed_halt,
           "You feel a slight force as the ship glides to a halt ...");
    strcpy_s(speed_increase, "You feel a sudden jerk as the ship speeds up.");
    strcpy_s(ship_is_docked, "Ship is currently docked.");
    strcpy_s(ship_is_docking,
           "The ship is currently in docking procedures ...");
    strcpy_s(ship_is_jumping,
           "The ship is currently travelling in hyperspace.");
    strcpy_s(ship_is_undocking, "The ship is currently undocking ...");
    strcpy_s(ship_jumps, "The ship hums loudly as it enters into hyperspace.");
    strcpy_s(territorydb, "space/territorydb");
    strcpy_s(thrusters_activating,
           "A loud roar is heard as the steering thrusters activate.");
    strcpy_s(unit_name, "hm");
    strcpy_s(univdb, "space/univdb");


    // Setup defaults for non-string parameters
    admin_server = 0;
    admin_server_port = 4202;
    admin_server_threaded = 0;
    afterworld = 0;
    afterburn_fuel_ratio = 7;
    afterburn_ratio = 2;
    asteroid_size_multiplier = 100;
    autostart = 1;
    autozone = 1;
    cyc_interval = 1;
    space_wiz = 1;
    damage_repair_time = 60;
    detectdist = 500;
    forbid_puppets = 1;
    fuel_ratio = 200;
    identdist = 100;
    jump_speed_multiplier = 5;
    log_commands = 0;
    max_sensor_range = 9999;
    max_land_speed = 2000;
    max_dock_dist = 5;
    max_dock_size = 4;
    max_drop_dist = 20;
    max_gate_dist = 5;
    nebula_size_multiplier = 100;
    notify_shipscan = 1;
    seconds_to_drop = 30;
    sense_hypervessels = 1;
    use_comm_objects = 1;
    use_two_fuels = 1;
    warp_exponent = 3.333334;
    warp_constant = 299793;
}

/*
 * Places a string from the configuration file into its storage location.
 */
void HSCfStr(HS_INT32 * des, HS_INT8 * value)
{
    strcpy((HS_INT8 *) des, value);
}

/*
 * Places a float from the configuration file into its storage location.
 */
void HSCfFloat(HS_INT32 * des, HS_INT8 * value)
{
    double fval;

    fval = atof(value);
    *((HS_FLOAT32 *) des) = fval;
}
/*
 * Places an HS_INT32 from the configuration file into its storage location.
 */
void HSCfInt(HS_INT32 * des, HS_INT8 * value)
{
    HS_INT32 ival;

    ival = atoi(value);
    *des = ival;
}

/* 
 * Takes an option string loaded from the config file and
 * figures out where to put it in hs_options.
 */
HS_BOOL8 CHSConf::InputOption(HS_INT8 * option, HS_INT8 * value)
{
    HCONF *cfptr;

    for (cfptr = hspace_conflist; cfptr->name; cfptr++)
    {
        if (!_stricmp(option, cfptr->name))
        {
            cfptr->func(cfptr->des, value);
            return true;
        }
    }
    return false;
}

/* 
 * Opens the HSpace configuration file, loading in option strings and
 * sending them to hspace_input_option().
 */
HS_BOOL8 CHSConf::LoadConfigFile(HS_INT8 * lpstrPath)
{
    FILE *fp;
    HS_INT8 tbuf[1024];
    HS_INT8 tbuf2[256];
    HS_INT8 option[256];
    HS_INT8 value[1024];
    HS_INT8 *ptr, *ptr2;

    hs_log((HS_INT8 *) "LOADING: HSpace configuration file.");

    /*
     * Open the configuration file for reading
     */
    fopen_s(&fp, lpstrPath, "r");
    if (!fp)
    {
        hs_log((HS_INT8 *)
               "ERROR: Unable to open hspace configuration file.");
        return false;
    }

    /*
     * Read the entire file in.  Parse lines that have something in them
     * and don't begin with a '#'
     */
    while (fgets(tbuf, 256, fp))
    {
        /*
         * Truncate at the newline
         */
        if ((ptr = strchr(tbuf, '\n')) != NULL)
            *ptr = '\0';
        if ((ptr = strchr(tbuf, '\r')) != NULL)
            *ptr = '\0';

        /*
         * Strip leading spaces
         */
        ptr = tbuf;
        while (*ptr == ' ')
            ptr++;

        /*
         * Determine if the line is valid
         */
        if (!*ptr || *ptr == '#')
            continue;

        /*
         * Parse out the option and value
         */
        ptr2 = option;
        for (; *ptr && *ptr != ' ' && *ptr != '='; ptr++)
        {
            *ptr2 = *ptr;
            ptr2++;
        }
        *ptr2 = '\0';
        if (!*ptr)
        {
            sprintf_s(tbuf2, "ERROR: Invalid configuration at option: %s",
                    option);
            hs_log(tbuf2);
            continue;
        }
        ptr2 = value;
        while (*ptr && (*ptr == ' ' || *ptr == '='))
            ptr++;
        for (; *ptr; ptr++)
        {
            *ptr2 = *ptr;
            ptr2++;
        }
        *ptr2 = '\0';
        if (!InputOption(option, value))
        {
            sprintf_s(tbuf2, "ERROR: Invalid config option \"%s\"", option);
            hs_log(tbuf2);
        }
    }
    fclose(fp);

    m_bConfigLoaded = true;
    return true;
}
