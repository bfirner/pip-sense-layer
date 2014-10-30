/*
 * Copyright (c) 2014 Bernhard Firner and InPoint Systems
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301, USA
 * or visit http://www.gnu.org/licenses/gpl-2.0.html
 */

/*******************************************************************************
 * @file offlinecache.h
 * Cache SampleData structures when they cannot be sent to the aggregator.
 * Get them back later when connectivity with the aggregator is restored.
 *
 * @author Bernhard Firner
 ******************************************************************************/

#include <owl/sensor_aggregator_protocol.hpp>
#include <string>
#include <fstream>

#ifndef __OFFLINE_CACHE__
#define __OFFLINE_CACHE__

//The template requires a base file name for the cache file and a maximum size
//of the cache, in bytes
class OfflineCache {
  private:
    uint64_t bytes_written;
    std::ofstream outfile;
    std::ifstream infile;
    std::string filename;
    uint64_t max_size;

  public:
    //Constructor
    OfflineCache(std::string& filename, uint64_t max_size);

    //Cache packets to later send to the aggregator
    void cachePacket(SampleData& sd);

    //Get the next cached sample
    SampleData getCachedPacket();
};

#endif // __OFFLINE_CACHE__ not defined

