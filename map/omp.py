#!/usr/bin/env python2
# -*- encoding: utf-8 -*-

#  Copyright (©) 2015 Lucas Maugère, Thomas Mijieux
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.

import sys, re
from omp_ho import parse_hitobject
from omp_tp import parse_timingpoint
from omp_coco import parse_colour
from omp_sb import parse_event
from omp_util import Int, Float

def parse(osufilename):
    data = {}
    f = open(osufilename);
    ver = f.readline()
    m = re.match(u"^(\u00EF\u00BB\u00BF)?osu file format v([\d]+)\r\n", ver)
    if m:
        if m.group(1):
            data['BOM'] = True
        else:
            data['BOM'] = False
        data['version'] = int(m.group(2))
    else:
        print "Cannot determine \"{0}\" format!".format(osufilename)
        return

    for line in f:
        line = line.rstrip()
        a = re.match(r"^\[(.*)\]$", line)
        if a:
            current_section = a.group(1)
            if current_section in ["TimingPoints", "HitObjects",
                                   "Events", "Colours"]: 
                data[current_section] = [] # need list
            else: 
                data[current_section] = {} # need dict
            continue
        
        if line == "" or (line[0] =='/' and line[1] == '/'):
            continue # ignore comments or empty lines
        
        try:
            current_section
        except:
            current_section = None

        if current_section is not None:
            
            if current_section == "HitObjects":
                data[current_section].append(parse_hitobject(line))
                
            elif current_section == "TimingPoints":
                data[current_section].append(parse_timingpoint(line))
                
            elif current_section == "Events":
                ev = parse_event(line)
                if ev not in [None, {}]:
                    # this check is necesseary because the function build the
                    # object over calls (a single sprite can be splitted on
                    # multiple lines)
                    data[current_section].append(ev)
                    
            elif current_section == "Colours":
                data[current_section].append(parse_colour(line))

            else:
                m = re.match(r"^ *([^ ]*) *: *(.*) *$", line)
                if m:
                    keyword, value = m.group(1), m.group(2)
                    if keyword in  ["Bookmarks", "Tags"]:
                        value = map(Int, value.split(","))
                    value = Float(value)
                    data[current_section][keyword] = value

                    
    return data


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print "error"
        sys.exit(1)

    data = parse(sys.argv[1])
    import pprint
    pp = pprint.PrettyPrinter(indent=0)
    pp.pprint(data)


