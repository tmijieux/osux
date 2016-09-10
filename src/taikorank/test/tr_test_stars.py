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
from levenshtein import Levenshtein
from tr_models import TR_Map_List
from tr_exec import TR_Exec
from tr_test import TR_Tester_Yaml, TR_Test_Str_List, TR_Exception

class TR_Test_Stars(TR_Test_Str_List):
    def __init__(self, ht):
        TR_Test_Str_List.__init__(self, ht)
        self.field = self.get_if_exists('field', ht, 'final_star')
        self.mods  = self.get_if_exists('mods',  ht, ['__'])
        self.ggm   = self.get_if_exists('ggm',   ht, [(0, 0)])
        self.merge_mods = self.get_if_exists('merge_mods', ht, True)
        self.computed = False
    #
    def combine(self, tests):
        res = {}
        for key in self.name.split(" "):
            for mod in tests[key].res.keys():
                if mod not in res:
                    res[mod] = TR_Map_List()
                res[mod].extend(tests[key].res[mod])
        for mod in res.keys():
            res[mod] = res[mod].sort_by(self.field)
        self.res = res
        self.computed = True
    #
    def compute(self):
        if self.computed:
            return
        res = {}
        for mod in self.mods:
            res[mod] = TR_Map_List()
            for ggm in self.ggm:
                tmp = TR_Exec.compute(self.cmd, {'+plast_score_only': 1, '-mods': mod, '-ggm': "%d %d" % ggm})
                res[mod].extend(TR_Map_List.from_yaml(tmp))
        if self.merge_mods:
            l = TR_Map_List()
            for key in res:
                l.extend(res[key])
            self.res = {'__': l}
        else:
            self.res = res
        map_str = lambda map: map.str_brief(self.merge_mods)
        for key in self.res:
            self.res[key] = self.res[key].sort_by(self.field, True, self.expected, map_str)
        self.computed = True
    #
    S_OK   = "%s - %%s"       % (Colors.green("OK"))
    S_FAIL = "%s - %%s {%%s}" % (Colors.fail("<>"))
    def check_one_mod(self, mod):
        self.check_same_len(self.res[mod])
        brief = []
        full  = []
        for map in self.res[mod]:
            brief.append(map.str_brief(self.merge_mods))
            full.append(map.str_full(self.field, self.merge_mods))

        errors = Levenshtein(self.expected, brief).dist()
        zipped = zip(self.expected, brief, full)
        if errors:
            logging.warning(self.diff_str(zipped))
        else:
            logging.info(self.diff_str(zipped))
        return errors
    #
    def diff_str(self, zipped):
        l = []
        for s_map, s_brief, s_full in zipped:
            if s_map == s_brief:
                l.append(self.S_OK % (s_full))
            else:
                l.append(self.S_FAIL % (s_full, Colors.warning(s_map)))
        return '\n' + '\n'.join(l)
    #
    def dump_str(self):
        l = []
        for map in self.res['__']:
            l.append(map.str_brief(self.merge_mods))
        return '\n' + '\n'.join(l)
    #
    def compare(self):
        errors = 0
        for mod in self.res.keys():
            if not self.merge_mods:
                logging.info(Colors.blue("Test on mod '%s'" % mod))
            errors += self.check_one_mod(mod)
        total  = len(self.expected) * len(self.res.keys())
        return (errors, total)

########################################################

class TR_Tester_Stars(TR_Tester_Yaml):
    def __init__(self, argv):
        TR_Tester_Yaml.__init__(self, argv)
        self.tests = {}
    #
    def test_basic(self, data):
        for x in data:
            test = TR_Test_Stars(x)
            self.tests[test.name] = test
            if test.do:
                logging.info("")
                err, tot = test.main()
                self.errors += err
                self.total  += tot
    #
    def test_combined(self, data):
        for x in data:
            test = TR_Test_Stars(x)
            self.tests[test.name] = test
            if test.do:
                logging.info("")
                test.combine(self.tests)
                err, tot = test.main()
                self.errors += err
                self.total  += tot
    #
    def test_yaml(self, data):
        if 'tests' in data:
            self.test_basic(data['tests'])
        if 'combined' in data:
            self.test_combined(data['combined'])
        return (self.errors, self.total)

if __name__ == "__main__":
    logging.basicConfig(level = logging.WARNING,
                        format = '%(message)s')
    TR_Tester_Stars(sys.argv).main()
