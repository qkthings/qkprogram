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
#include "qk_system.h"

void packet_calc_header_length(qk_packet *packet)
{
  packet->header_lenght = SIZE_FLAGS_CTRL + SIZE_CODE + SIZE_ID;
}


uint64_t packet_get_value(uint8_t byteCount, uint16_t *idx, qk_packet *packet)
{
  uint16_t j, i = *idx;
  uint64_t value = 0;

  for(j = 0; j < byteCount; j++)
    value += (uint64_t)(((uint64_t)(packet->payload[i++]) & 0xFF) << (8*j)); // LSB first

  *idx = i;

  switch(byteCount) // truncate
  {
  case 1:
      value &= 0xFF;
      break;
  case 2:
      value &= 0xFFFF;
      break;
  case 4:
      value &= 0xFFFFFFFF;
      break;
  }
  return value;
}

void packet_get_string(char *buf, uint16_t count, uint16_t *idx, qk_packet *packet)
{
  uint16_t j, i = *idx;
  char c;

  memset(buf, '\0', count+1);
  for(j = 0; j < count; j++) {
    if(j+1 < packet->payload_lenght) {
      c = (char)packet->payload[i++];
      if(c != '\0' && (c < 32 || c > 126))
        c = ' ';
      buf[j] = c;
    }
    else
      break;
  }
  *idx = i;
}


