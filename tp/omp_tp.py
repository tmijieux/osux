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
