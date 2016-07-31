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

from colors import Colors
from tr_exec import TR_Exec
from tr_test import TR_Tester_Yaml, TR_Test_Str_List

class TR_Test_Autoconvert(TR_Test_Str_List):
    MAX_LENGTH = 16
    #########################################################
    def __init__(self, ht):
        TR_Test_Str_List.__init__(self, ht)
        self.unit = ht['unit']
        self.opt  = {'ptro': 1, 'autoconvert': 1}
    #########################################################
    def compute(self):
        res = TR_Exec.compute(self.cmd, self.opt)
        self.res = sorted(res['maps'], key=lambda k: k['difficulty'])
    #########################################################
    S_OK   = "%s - %%s"       % (Colors.green("OK"))
    S_FAIL = "%s - %%s {%%s}" % (Colors.fail("<>"))
    def check(self):
        self.check_same_len(self.res)
        #
        errors = 0
        zipped = zip(self.expected, self.res);
        for expected, res in zipped:
            s = self.str_brief(res)
            if s == expected:
                print(self.S_OK   % (s))
            else:
                print(self.S_FAIL % (s, Colors.warning(expected)))
                errors += 1
        return errors
    #########################################################
    def str_brief(self, x):
        s = ""
        for i in range(0, len(x['objects'])):
            if i != 0:
                space = (x['objects'][i]['offset'] -
                         x['objects'][i-1]['offset']) / self.unit
                s += "_" * (int(space) - 1)
            s += x['objects'][i]['type']
        #
        s += " " * (self.MAX_LENGTH - len(s))
        return "%s%s[%s]" % (s, x['title'], x['difficulty'])
    #########################################################
    def dump(self):
        for x in self.res:
            print(self.str_brief(x))
    #########################################################
    def compare(self):
        errors = self.check()
        total  = len(self.expected)
        return (errors, total)
    #########################################################

########################################################

class TR_Tester_Autoconvert(TR_Tester_Yaml):
    ########################################################
    def __init__(self, argv):
        TR_Tester_Yaml.__init__(self, argv)
    ########################################################
    def test_yaml(self, data):
        if 'tests' in data:
            self.test_iterate(data['tests'],
                              TR_Test_Autoconvert,
                              TR_Test_Autoconvert.main)
        return (self.errors, self.total)
    ########################################################

########################################################

if __name__ == "__main__":
    TR_Tester_Autoconvert(sys.argv).main()
