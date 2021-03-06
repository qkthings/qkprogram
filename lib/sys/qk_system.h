/*
 * QkThings LICENSE
 * The open source framework and modular platform for smart devices.
 * Copyright (C) 2014 <http://qkthings.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef QK_SYSTEM_H
#define QK_SYSTEM_H

#define QK_VERSION_MAJOR     0
#define QK_VERSION_MINOR     1
#define QK_VERSION_PATCH     0


#if defined( QKPROGRAM_DEBUG)
  #include <stdio.h>
  #define QKPROGRAM_PRINT(...) printf(__VA_ARGS__)
#else
  #define QKPROGRAM_PRINT(...)
#endif

/*****************************************************************************
 *      Settings
 *****************************************************************************/
#include "qk_settings.h"

/*****************************************************************************
 *      Program (public)
 *****************************************************************************/
#include "qk_program.h"


/*****************************************************************************
 *      HAL (Hardware Abstraction Layer)
 *****************************************************************************/
#include "hal_p.h"

/*****************************************************************************
 *      Core (private)
 *****************************************************************************/
//#include "../core/qk_utils.h"
#include "qk_packet_p.h"
#include "qk_protocol_p.h"
#include "qk_fragment_p.h"

#include "qk_core_p.h"
#include "qk_comm_p.h"
#include "qk_device_p.h"
#include "qk_board_p.h"


#endif /* QK_SYSTEM_H */
