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
import logging

from colors import Colors
from tr_exec import TR_Exec
from tr_test import TR_Tester_Yaml, TR_Test_Expected
from tr_models import TR_Map_List, TR_Map_With_Objects

class TR_Test_Autoconvert(TR_Test_Expected):
    MAX_LENGTH = 16
    #
    def __init__(self, ht):
        TR_Test_Expected.__init__(self, ht)
        self.unit = ht['unit']
        self.opt  = {'+ptro': 1, '+autoconvert': 1}
    #
    def compute(self):
        tmp = TR_Exec.compute(self.cmd, self.opt)
        tmp = TR_Map_List.from_yaml(tmp, TR_Map_With_Objects)
        self.res = tmp.sort_by('difficulty', False)
    #
    S_OK   = "%s - %%s"       % (Colors.green("OK"))
    S_FAIL = "%s - %%s {%%s}" % (Colors.fail("<>"))
    def check(self):
        self.check_same_len(self.res)
        errors = 0
        zipped = zip(self.expected, self.res);
        for expected, map in zipped:
            s = map.str_obj(self.unit, self.MAX_LENGTH)
            if s == expected:
                logging.info(self.S_OK   % (s))
            else:
                logging.info(self.S_FAIL % (s, Colors.warning(expected)))
                errors += 1
        return errors
    #
    def dump_str(self):
        l = []
        for map in self.res:
            l.append(map.str_obj(self.unit, self.MAX_LENGTH))
        return '\n' + '\n'.join(l)
    #
    def compare(self):
        errors = self.check()
        total  = len(self.expected)
        return (errors, total)

########################################################

class TR_Tester_Autoconvert(TR_Tester_Yaml):
    def __init__(self, argv):
        TR_Tester_Yaml.__init__(self, argv)
    #
    def test_yaml(self, data):
        if 'tests' in data:
            self.test_iterate(data['tests'],
                              TR_Test_Autoconvert,
                              TR_Test_Autoconvert.main)
        return (self.errors, self.total)

########################################################

if __name__ == "__main__":
    logging.basicConfig(level = logging.WARNING,
                        format = '%(message)s')
    TR_Tester_Autoconvert(sys.argv).main()
