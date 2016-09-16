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
from tr_test import TR_Tester_Yaml, TR_Test
from tr_models import TR_Map_List

class Pool:
    def __init__(self, path, dir, field):
        self.name = dir
        self.path = path + dir
        self.cmd  = self.path + "*.osu"
        self.field = field
    #
    def compute(self):
        logging.info("Computing " + self.path)
        yaml = TR_Exec.compute(self.cmd)
        res = TR_Map_List.from_yaml(yaml)
        self.res = res.sort_by(self.field)
        l = self.res.get_lambda_eval(self.field)
        self.min = l(self.res[-1])
        self.max = l(self.res[0])
    #
    def __str__(self):
        return "%s [%g, %g]" % (self.name, self.min, self.max)

########################################################

class TR_Test_Pool(TR_Test):
    def __init__(self, ht):
        TR_Test.__init__(self, ht['path'])
        self.path = ht['path']
        self.field = self.get_if_exists('field', ht, "final_star")
        self.pools = []
        for dir in ht['pools']:
            self.pools.append(Pool(self.path, dir, self.field))
    #
    def compute(self):
        for pool in self.pools:
            pool.compute()
    #
    def compare(self):
        total = 0
        for pool in self.pools:
            total += len(pool.res)
        errors = 0
        for i in range(0, len(self.pools)-1):
            p0 = self.pools[i]
            p1 = self.pools[i+1]
            min = p0.min
            max = p1.max
            ge = p0.res.filter_le(self.field, max)
            le = p1.res.filter_ge(self.field, min)
            errors += len(ge) + len(le)
            logging.warning(Colors.blue("Maps between %s and %s" % (str(p0), str(p1))))
            logging.warning(ge.str_full(self.field, False))
            logging.warning(le.str_full(self.field, False))
        return (errors, total)

########################################################

class TR_Tester_Pool(TR_Tester_Yaml):
    def __init__(self, argv):
        TR_Tester_Yaml.__init__(self, argv)
    #
    def test_yaml(self, data):
        if 'tests' in data:
            self.test_iterate(data['tests'],
                              TR_Test_Pool,
                              TR_Test_Pool.main)
        return (self.errors, self.total)

########################################################

if __name__ == "__main__":
    logging.basicConfig(level = logging.DEBUG,
                        format = '%(message)s')
    TR_Tester_Pool(sys.argv).main()
