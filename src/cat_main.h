#ifndef _CAT_MAIN_
#define _CAT_MAIN_

#include <stdio.h>
#include <unistd.h>//getopt()
#include <inttypes.h>
#include <stdlib.h>//atoi()
#include <string.h>//strtok_r()

#include <cosc.h>
#include <association_app.h>
#include <allocation_app_l3cat.h>

/*
 * show current cos config
 * */
void show_cosc_conf();

/*
 * parse configure from command line
 * */
int parse_cosc(int argc, char** argv);

/*
 * init cos config and core bind
 * */
void init_cosc();

/*
 * configure all cos configure core bind based on core_mask and llc_mask
 * @core_mask : enabled CPU mask 
 * @llc_mask : enabled LLC mask, which must be contiguous 
 * */
int parse_core_mask(uint32_t core_mask, uint32_t llc_mask);

#endif
