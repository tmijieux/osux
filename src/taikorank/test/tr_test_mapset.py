#!/bin/python3

# Copyright (©) 2015-2016 Lucas Maugère, Thomas Mijieux
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#    http://www.apache.org/licenses/LICENSE-2.0
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import sys
import os
import logging

from tr_exec import TR_Exec
from tr_type import TR_Type_Util
from tr_test import TR_Tester
from tr_test_stars import TR_Test_Stars
from tr_models import TR_Map, TR_Map_List

########################################################

class TR_Test_Mapset(TR_Test_Stars):
    def __init__(self, ht):
        ht['expected'] = '\n'
        ht['field']    = 'final_star'
        ht['mods']     = ['__']
        TR_Test_Stars.__init__(self, ht)
    #
    def sort_maps(self, list):
        return list.sort_by(self.field, True)
    #
    def check_one_mod(self, mod):
        self.expected = self.create_expected(mod)
        return TR_Test_Stars.check_one_mod(self, mod)
    #
    def create_expected(self, mod):
        expected = []
        res = self.res[mod].copy()
        for i in range(0, len(res)):
            name = res[i].difficulty
            res[i].type = TR_Type_Util.get_tr_type(name)
        res = sorted(res, key=lambda k: k.type.rank, reverse=True)
        l = []
        for map in res:
            l.append(str(map))
        return l

########################################################

class TR_Tester_Mapset(TR_Tester):
    MAX_DEPTH = 3
    #
    def __init__(self, argv):
        TR_Tester.__init__(self, argv)
    #
    def test_dir(self, path):
        data = {}
        data['path'] = path
        test = TR_Test_Mapset(data)
        err, tot = test.main()
        self.errors += err
        self.total  += tot
    #
    def test_recursive(self, arg, depth):
        if depth > self.MAX_DEPTH:
            return
        osu_files = []
        for f in os.listdir(arg):
            path = arg + f + '/'
            if os.path.isdir(path):
                self.test_recursive(path, depth + 1)
            elif f.endswith(".osu"):
                osu_files.append(f)
        if len(osu_files) > 2:
            self.test_dir('"' + arg + '"')
    #
    def test(self, arg):
        self.test_recursive(arg, 0)
        return (self.errors, self.total)

if __name__ == "__main__":
    logging.basicConfig(level = logging.WARNING,
                        format = '%(message)s')
    TR_Tester_Mapset(sys.argv).main()
