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

import yaml
import sys

from colors import Colors
from levenshtein import Levenshtein
from tr_exec import TR_Exec
from tr_test import TR_Tester, TR_Test_Str_List

class TR_Test_Stars(TR_Test_Str_List):
    def __init__(self, ht):
        TR_Test_Str_List.__init__(self, ht)
        #
        self.field = self.get_if_exists('field', ht, 'final_star')
        self.mods  = self.get_if_exists('mods',  ht, ['__'])
        self.computed = False
    #########################################################
    def combine(self, tests):
        res = []
        for key in self.name.split(" "):
            res.extend(tests[key].res)
        self.res = sorted(res, key=lambda k: k['stars'][self.field],
                          reverse=True)
        self.computed = True
    #########################################################
    def str_full(self, data):
        return ("%s\t(%.4g\t%.4g\t%.4g\t%.4g\t%.4g)\t%s" %
                (data['stars'][self.field],
                 data['stars']['final_star'],
                 data['stars']['density_star'],
                 data['stars']['reading_star'],
                 data['stars']['pattern_star'],
                 data['stars']['accuracy_star'],
                 self.str_brief(data)))
    #########################################################
    def str_brief(self, data):
        s = ("%s[%s]" % (data['title'], data['difficulty']))
        if data['mods']:
            return s + "(%s)" % (data['mods'])
        else:
            return s
    #########################################################
    def compute_one_mod(self, mod):
        res = TR_Exec.compute(self.cmd, {'mods': mod})
        return res['maps']
    #########################################################
    def compute(self):
        if self.computed:
            return
        #
        res = []
        for mod in self.mods:
            res.extend(self.compute_one_mod(mod))
        #
        self.res = sorted(res, key=lambda k: k['stars'][self.field], 
                          reverse=True)
        self.computed = True
    #########################################################
    S_OK   = "%s - %%s"       % (Colors.green("OK"))
    S_FAIL = "%s - %%s {%%s}" % (Colors.fail("<>"))
    def check(self):
        self.check_same_len(self.res)
        #
        brief = []
        full  = []
        for test in self.res:
            brief.append(self.str_brief(test))
            full.append(self.str_full(test))
        #
        zipped = zip(self.expected, brief, full)
        for s_map, s_brief, s_full in zipped:
            if s_map == s_brief:
                print(self.S_OK % (s_full))
            else:
                print(self.S_FAIL % (s_full, Colors.warning(s_map)))
        #
        return Levenshtein(self.expected, brief).dist()
    #########################################################
    def dump(self):
        for x in self.res:
            print(self.str_brief(x))
    #########################################################
    def compare(self):
        errors = self.check()
        total  = len(self.expected)
        return (errors, total)
    ########################################################

########################################################

class TR_Tester_Stars(TR_Tester):
    ########################################################
    def __init__(self, argv):
        TR_Tester.__init__(self, argv)
        self.errors = 0
        self.total  = 0
        self.tests  = {}
    ########################################################
    def test_basic(self, data):
        for x in data:
            test = TR_Test_Stars(x)
            self.tests[test.name] = test
            if test.do:
                print("")
                err, tot = test.main() 
                self.errors += err
                self.total  += tot
    ########################################################
    def test_combined(self, data):
        for x in data:
            test = TR_Test_Stars(x)
            self.tests[test.name] = test
            if test.do:
                print("")
                err, tot = test.combine() 
                self.errors += err
                self.total  += tot
    ########################################################
    def test(self, data):
        if 'tests' in data:
            self.test_basic(data['tests'])
        if 'combined' in data:
            self.test_combined(data['combined'])
        #
        return (self.errors, self.total)
    ########################################################

if __name__ == "__main__":
    TR_Tester_Stars(sys.argv).main()
