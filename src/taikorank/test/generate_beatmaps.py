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

import os
from colors import Colors

DEBUG = False

##################################################

class Dir_Generator:
    CMD = "taiko_generator"
    OPT = "-q" if not DEBUG else ""
    #
    def __init__(self, dir):
        self.count = 0
        self.up = None
        self.dir = self.clean_dirname(dir)
        self.generators = []
    #
    def count_generated(self):
        count = self.count
        for g in self.generators:
            count += g.count_generated()
        return count
    #
    def add(self, list):
        for g in list:
            g.up = self
        self.generators.extend(list)
        return self
    #
    def get_leaves(self):
        if self.generators:
            leaves = []
            for g in self.generators:
                leaves.extend(g.get_leaves())
            return leaves
        else:
            return [self]
    #
    def get_path(self):
        dir = self.dir
        g = self
        while g.up:
            g = g.up
            dir = g.dir + dir
        return dir
    #
    # Override to generate maps. By default call children.
    def generate(self):
        for g in self.generators:
            g.main()
    #
    def main(self):
        self.path = self.get_path()
        Dir_Generator.cmd("mkdir -p %s" % self.path)
        print("Generating in '%s'" % self.path)
        self.generate()
    #
    def tg_cmd(self, arg):
        self.count += 1
        cmd = "%s %s -d %s %s" % (self.CMD, self.OPT, self.path, arg)
        Dir_Generator.cmd(cmd)
    #
    def __str__(self):
        res = "In\t%s\n" % self.get_path()
        if self.generators:
            res += "Generators:\n"
            for g in self.generators:
                res += str(g)
        res += "End of\t%s\n" % self.get_path()
        return res
    #
    @staticmethod
    def cmd(cmd):
        if DEBUG:
            print(Colors.blue("Debug: '%s'" % cmd))
        return os.system(cmd)
    #
    @staticmethod
    def clean_pattern(pattern):
        return pattern.replace(" ", "")
    @staticmethod
    def clean_dirname(dirname):
        return dirname.replace(" ", "_")

##################################################

class Ranges():
    bpm = range(10, 400, 10)
    obj = [2**i for i in range(1, 10)]
    od  = [i/2. for i in range(0, 20)]
    svm = [i/4. for i in range(2, 8)]
    patterns = {}
    patterns['group'] = {}
    patterns['group']['by_2'] = [
        'dd______',
        'd_d_____',
        'd__d____',
        'd___d___',
    ]
    patterns['group']['by_3'] = [
        'ddd_____',
        'd_d_d___',
        'd__d__d_',
    ]
    patterns['group']['by_4'] = [
        'dddd____',
        'd_d_d_d_',
        'dd__dd__',
        'ddd_d___',
    ]
    patterns['group']['by_5'] = [
        'ddddd___',
        'ddd_d_d_',
        'dd_dd_d_',
    ]
    patterns['group']['by_6'] = [
        'dddddd__',
        'ddd_ddd_',
        'dddd_dd_',
        'ddddd_d_',
    ]
    patterns['group']['start'] = [
        'dddddddd',
        'ddddddd_',
        'dddddd__',
        'ddddd___',
        'dddd____',
        'ddd_____',
        'dd______',
        'd_______',
    ]
    patterns['strain'] = [
        'dddd dddd',
        'dkdk dkdk',
        'ddkk ddkk',
        'dddd kkkk',
    ]

##################################################

def propagate(func):
    def _decorator(self, *args, **kwargs):
        if not self.generators:
            func(self, *args, **kwargs)
        else:
            for g in self.get_leaves():
                func(g, *args, **kwargs)
        return self
    return _decorator

class Map_Generator(Dir_Generator):
    def __init__(self, dir, prefix = ''):
        super(Map_Generator, self).__init__(dir)
        self.prefix = prefix
    #
    def tg_cmd(self, arg):
        super(Map_Generator, self).tg_cmd(self.prefix + arg)
    #
    @propagate
    def _with(self, prefix):
        self.prefix += prefix
    #
    @propagate
    def _on(self, opt_format, list):
        l = lambda: [self.tg_cmd(opt_format % x) for x in list]
        self.generate = l
    #
    @propagate
    def _by(self, dir_format, list, with_func):
        self.add([
            with_func(Map_Generator(dir_format % x, self.prefix), x)
            for x in list
        ])
    #
    def with_pattern(self, pattern):
        pattern = super(Map_Generator, self).clean_pattern(pattern)
        return self._with("-p %s " % pattern)
    def with_bpm(self, bpm):
        return self._with("-b %d " % bpm)
    def with_obj(self, obj):
        return self._with("-n %d " % obj)
    def with_od(self, od):
        return self._with("-o %g " % od)
    def with_svm(self, svm):
        return self._with("-s %g " % svm)
    #
    def on_bpm(self, list = Ranges.bpm):
        return self._on("-b %d ", list)
    def on_pattern(self, list):
        return self._on("-p %s ", list)
    def on_obj(self, list = Ranges.obj):
        return self._on("-n %d ", list)
    def on_od(self, list = Ranges.od):
        return self._on("-o %g ", list)
    def on_svm(self, list = Ranges.svm):
        return self._on("-s %g ", list)
    #
    def by_bpm(self, list = Ranges.bpm):
        return self._by("%d_bpm/", list, Map_Generator.with_bpm)
    def by_pattern(self, list):
        return self._by("%s/", list, Map_Generator.with_pattern)
    def by_obj(self, list = Ranges.obj):
        return self._by("%d_obj/", list, Map_Generator.with_obj)
    def by_od(self, list = Ranges.od):
        return self._by("OD%g/", list, Map_Generator.with_od)
    def by_svm(self, list = Ranges.svm):
        return self._by("svm%g/", list, Map_Generator.with_svm)

##################################################
##################################################
##################################################
##################################################

density_g = Dir_Generator("density/").add([
    Map_Generator("strain/").by_pattern(Ranges.patterns['strain']).by_obj().on_bpm(),
    Dir_Generator("by_group/").add([
        Map_Generator(name).on_pattern(list)
        for name, list in Ranges.patterns['group'].items()
    ]),
])

##################################################

reading_g = Dir_Generator("reading/").add([
])

##################################################

pattern_g = Dir_Generator("pattern/").add([
])

##################################################

accuracy_g = Dir_Generator("accuracy/").add([
    Map_Generator("od/").by_pattern(Ranges.patterns['strain']).on_od(),
])

##################################################

other_g = Dir_Generator("other/").add([
])

##################################################

main_g = Dir_Generator("maps_generated/")
main_g.add([
    density_g,
    reading_g,
    pattern_g,
    accuracy_g,
    other_g,
])

##################################################

if __name__ == '__main__':
    main_g.main()
    print("%d beatmaps created!" % main_g.count_generated())
