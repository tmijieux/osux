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
from osux_util import Int, Float

################################################################################

# HITSOUND

HO_CIRCLE   = 1 << 0
HO_SLIDER   = 1 << 1
HO_NEWCOMBO = 1 << 2
HO_SPINNER  = 1 << 3

################################################################################

# COLOR

def parse_colour(line):
    line = line.split(":")
    map(str.rstrip, line)
    line[1] = map(Int, line[1].split(","))
    return line


################################################################################

# HITOBJECT

def parse_hitobject(line):
    ho = {}
    v = line.split(",")
    mm = lambda x,y: {x:Float(y)}
    k = map(mm, ["x", "y", "offset", "type", "hs.sample"], v[:5])
    map(ho.update, k)
    wc = lambda x: Int(x) & ~HO_NEWCOMBO & 0x0F

    if wc(ho["type"]) is HO_CIRCLE:
        pass
    elif wc(ho["type"]) is HO_SPINNER:
        ho["spi.end_offset"] = int(v[5])
    elif wc(ho["type"]) is HO_SLIDER:
        a = v[5].split("|")
        ll = lambda x: map(int, x.split(":"))
        k = []
        map(k.append, map(ll, a[1:]))
        ho.update({
            'sli.repeat': Int(v[6]),
            'sli.length': Float(v[7]),
            'sli.type': ord(a[0]),
            'sli.coord': k
        })

        ho["sli.hs.additional"] = False
        for vv in v[8:]:
            if '|' in vv and ':' in vv: # slider st/st_additional
                ho["sli.hs.additional"] = True
                s = vv.split("|")
                hh = lambda x: map(Int, x.split(":"))
                ho["sli.hs.st_add"] = map(hh, s)
            elif '|' in vv and ':' not in vv: # slider sample
                ho["sli.hs.additional"] = True
                s = vv.split("|")
                ho["sli.hs.sample"] = map(Int, s)

    if ":" in v[-1] and "|" not in v[-1]: # additionals
        u = v[-1].split(":")
        nn = lambda x,y: {x:Int(y)}
        ho['hs.additional'] = True
        map(ho.update,
             map(nn ,["hs.st", "hs.st_additional", "hs.sample_set_index",
                      "hs.volume", "hs.sfx_filename"], u))
    else:
        ho['hs.additional'] = False
    return ho

################################################################################

# TIMING POINT

def parse_timingpoint(line):
    line =  line.split(',')
    return {
        'offset':         float( line[0]), # WARNING: THIS IS FLOAT!
        'mpb' :           float( line[1]),
        'time_signature': int(   line[2]),
        'sample_type':    int(   line[3]),
        'sample_set':     int(   line[4]),
        'volume':         int(   line[5]),
        'uninherited':bool( int( line[6])),
        'kiai':       bool( int( line[7]))
        }


################################################################################

# STORYBOARD

SB_CURRENT_EVENT = {}

def parse_event(line):
    global SB_CURRENT_EVENT
    RETURN_EVENT = None
    
    line = line.split(",")
    
    if line[0] in ["Sprite", "Animation", "0", "1", "2", "3"]:
        RETURN_EVENT = SB_CURRENT_EVENT
        SB_CURRENT_EVENT = { "header": map(Int, line), "value": [] }
    else:
        SB_CURRENT_EVENT["value"].append(map(Int, line))
        
    return RETURN_EVENT

################################################################################

# GLOBAL

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
                m = map(str.strip, line.split(":", 1))
                if len(m) == 2:
                    keyword, value = m[0], m[1]
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


