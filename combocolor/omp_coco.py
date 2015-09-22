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


from omp_util import Int, Float

def parse_colour(line):
    line = line.split(":")
    map(str.rstrip, line)
    line[1] = map(Int, line[1].split(","))
    return line

