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


from omp_hs import *
from omp_util import Int, Float

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
