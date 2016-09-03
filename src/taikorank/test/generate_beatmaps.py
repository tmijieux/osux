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
import yaml
import re
from subprocess import Popen, PIPE, STDOUT
from colors import Colors

DEBUG = False

##################################################

def propagate(func):
    def _decorator(self, *args, **kwargs):
        for g in self.get_leaves():
            func(g, *args, **kwargs)
        return self
    return _decorator

##################################################

class Dir_Generator:
    CMD = "taiko_generator"
    OPT = ""
    #
    def __init__(self, dir):
        self.up = None
        self.dir = self.clean_dirname(dir)
        self.generators = []
    #
    def count(self):
        return 0
    #
    def count_generated(self):
        count = self.count()
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
        Dir_Generator.cmd_mkdir(self.path)
        print("Generating in '%s'" % self.path)
        self.generate()
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
        p = Popen(cmd, shell=True, universal_newlines=True,
                  stdout=PIPE, stderr=PIPE, close_fds=True)
        return p.communicate()
    #
    @staticmethod
    def cmd_mkdir(path):
        return Dir_Generator.cmd("mkdir -p %s" % path)
    #
    def cmd_tg(self, arg):
        cmd = "%s %s -d %s %s" % (self.CMD, self.OPT, self.path, arg)
        return Dir_Generator.cmd(cmd)
    #
    @staticmethod
    def clean_pattern(pattern):
        return pattern.replace(" ", "")
    #
    @staticmethod
    def clean_dirname(dirname):
        dirname = dirname.replace(" ", "_")
        if not dirname.endswith("/"):
            dirname += "/"
        return dirname

##################################################

# Sort ranges from hardest to easiest
class Ranges():
    mods_HR = ['HDFL', 'HRFL', 'FL', 'HR', 'HD', '__', 'EZ']
    mods_HD = ['HDFL', 'HRFL', 'FL', 'HD', 'HR', '__', 'EZ']
    bpm  = range(400, 25, -25)
    abpm = range(400, 100, -25)
    hide_bpm  = range(640, 160, -80)
    slow_abpm = range(10, 100, 10)
    abpm_mods_HR = range(350, 180, -10)
    abpm_mods_HD = range(180, 120, -10)
    obj  = [2**i for i in range(10, 1, -1)]
    od   = [i/2. for i in range(20, 0, -1)]
    rand = range(30, 0, -5)
    # Spaces can be used in patterns, they will be removed
    patterns = {}
    patterns['d_spread'] = {}
    patterns['d_spread']['by_2'] = [
        'dd______',
        'd_d_____',
        'd__d____',
        'd___d___',
    ]
    patterns['d_spread']['by_3'] = [
        'ddd_____',
        'd_d_d___',
        'd__d__d_',
    ]
    patterns['d_spread']['by_4'] = [
        'dddd____',
        'ddd_d___',
        'dd__dd__',
        'd_d_d_d_',
    ]
    patterns['d_spread']['by_5'] = [
        'ddddd___',
        'ddd_d_d_',
        'dd_dd_d_',
    ]
    patterns['d_spread']['by_6'] = [
        'dddddd__',
        'ddddd_d_',
        'dddd_dd_',
        'ddd_ddd_',
    ]
    patterns['d_spread']['start'] = [
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
        'dddd kkkk',
        'ddkk ddkk',
        'dkdk dkdk',
        'dddd dddd',
    ]
    patterns['streams'] = {}
    patterns['streams']['k$d_0~4'] = [
        'kdddd',
        'kddd',
        'kdd',
        'kd',
        'k',
    ]
    patterns['streams']['k$d_5~15'] = [
        'kddd dd',
        'kddd ddd',
        'kddd dddd',
        'kddd dddd d',
        'kddd dddd dd',
        'kddd dddd ddd',
        'kddd dddd dddd',
        'kddd dddd dddd d',
        'kddd dddd dddd dd',
        'kddd dddd dddd ddd',
        'kddd dddd dddd dddd',
    ]
    patterns['streams']['$d$k_0~5'] = [
        'dddd d    kkkk k',
        'dddd      kkkk',
        'ddd       kkk',
        'dd        kk',
        'd         k',
        'd',
    ]
    patterns['streams']['$d$k_6~8'] = [
        'dddd dd   kkkk kk',
        'dddd ddd  kkkk kkk',
        'dddd dddd kkkk kkkk',
    ]
    patterns['streams']['base_8'] = [
        'ddkd kkdk',
        'dddd kkkk',
        'ddkk ddkk',
        'dddd dddd',
    ]
    patterns['streams']['base_12'] = [
        'ddd ddk ddd kkk',
        'ddd ddk',
        'ddk',
        'd',
    ]
    patterns['patterns'] = {}
    patterns['patterns']['7'] = [
        'ddkd kkd_ ____ ____',
        'dddd kkd_ ____ ____',
        'dddd ddk_ ____ ____',
        'dddd ddd_ ____ ____',
    ]
    patterns['patterns']['6'] = [
        'ddkd dk__ ____ ____',
        'dddd dk__ ____ ____',
        'dddd dd__ ____ ____',
    ]
    patterns['patterns']['5'] = [
        'dkdd k___ ____ ____',
        'dddd k___ ____ ____',
        'dddd d___ ____ ____',
    ]
    patterns['patterns']['4'] = [
        'ddkk ____ ____ ____',
        'dddd ____ ____ ____',
    ]
    patterns['patterns']['3'] = [
        'ddk_ ____ ____ ____',
        'ddd_ ____ ____ ____',
    ]
    patterns['ddkd_reduction'] = [
        'ddkd ddkd',
        'ddkd ddk_',
        'ddkd d_k_',
        'ddk_ ddk_',
        'd_k_ ddk_',
        'd_k_ d_k_',
        'd_k_ d___',
        'd___ d___',
        'd___ ____',
    ]

##################################################

class Map_Generator(Dir_Generator):
    def __init__(self, dir, prefix = ''):
        super(Map_Generator, self).__init__(dir)
        self.prefix = prefix
        self.maps = []
        self.ht = {}
    #
    @classmethod
    def from_parent(cls, dir, parent):
        g = cls(dir)
        g.prefix = parent.prefix
        return g
    #
    def __str__(self):
        s = "In '%s'\n" % self.get_path()
        s += "prefix: %s\n" % self.prefix
        for g in self.generators:
            s += str(g)
        return s
    #
    def count(self):
        return len(self.maps)
    #
    def cmd_tg(self, arg):
        out, err = super(Map_Generator, self).cmd_tg(self.prefix + arg)
        match = re.search("Output file: '.*/(.*)'", out)
        if not match:
            raise Exception("Output is not matching: '%s'\nErr: %s" % (out, err))
        filename = match.group(1)
        match = re.search("Test - (.*) \(.*\) (\[.*\]).osu", filename)
        if not match:
            raise Exception("Title is not matching: '%s'" % filename)
        title = match.group(1)
        diff  = match.group(2)
        self.maps.append(title + diff)
        return out, err
    #
    @propagate
    def on_this(self):
        self.generate = lambda: self.cmd_tg("")
    #
    @propagate
    def _with(self, prefix):
        self.prefix += prefix
    #
    @propagate
    def _on(self, opt_format, list):
        l = lambda: [self.cmd_tg(opt_format % x) for x in list]
        self.generate = l
    #
    @propagate
    def _by(self, dir_format, list, with_func):
        self.add([
            with_func(Map_Generator.from_parent(dir_format % x, self), x)
            for x in list
        ])
    #
    @propagate
    def _on_each(self, dict, on_func):
        self.add([
            on_func(Map_Generator.from_parent(name, self), list)
            for name, list in dict.items()
        ])
    #
    def with_pattern(self, pattern):
        pattern = super(Map_Generator, self).clean_pattern(pattern)
        return self._with("-p %s " % pattern)
    def with_bpm(self, bpm):
        return self._with("-b %d " % bpm)
    def with_abpm(self, abpm):
        return self._with("-a %d " % abpm)
    def with_obj(self, obj):
        return self._with("-n %d " % obj)
    def with_od(self, od):
        return self._with("-o %g " % od)
    def with_rand(self, rand):
        return self._with("-r %d " % rand)
    #
    def on_bpm(self, list = Ranges.bpm):
        return self._on("-b %d ", list)
    def on_abpm(self, list = Ranges.abpm):
        return self._on("-a %d ", list)
    def on_pattern(self, list):
        list = [super(Map_Generator, self).clean_pattern(x) for x in list]
        return self._on("-p %s ", list)
    def on_obj(self, list = Ranges.obj):
        return self._on("-n %d ", list)
    def on_od(self, list = Ranges.od):
        return self._on("-o %g ", list)
    def on_rand(self, list = Ranges.rand):
        return self._on("-r %d ", list)
    #
    def by_bpm(self, list = Ranges.bpm):
        return self._by("%d_bpm/", list, Map_Generator.with_bpm)
    def by_abpm(self, list = Ranges.abpm):
        return self._by("%d_abpm/", list, Map_Generator.with_abpm)
    def by_pattern(self, list):
        return self._by("%s/", list, Map_Generator.with_pattern)
    def by_obj(self, list = Ranges.obj):
        return self._by("%d_obj/", list, Map_Generator.with_obj)
    def by_od(self, list = Ranges.od):
        return self._by("OD%g/", list, Map_Generator.with_od)
    def by_rand(self, list = Ranges.rand):
        return self._by("rand_%d/", list, Map_Generator.with_rand)
    #
    def on_each_pattern(self, dict):
        return self._on_each(dict, Map_Generator.on_pattern)
    #
    @staticmethod
    def mods_str(mods):
        if mods == '__':
            return ""
        return "(" + " ".join(re.findall('..', mods)) + " )"
    #
    def expected(self):
        self.ht['path'] = self.get_path() + "*.osu"
        if not('mods' in self.ht and self.ht['merge_mods']):
            self.ht['expected'] = self.maps.copy()
        else:
            self.ht['expected'] = []
            for mods in self.ht['mods']:
                maps = [map + Map_Generator.mods_str(mods) for map in self.maps]
                self.ht['expected'].extend(maps)
        return self.ht
    #
    @propagate
    def expected_field(self, field):
        self.ht['field'] = field
    #
    @propagate
    def expected_with_unmerged_mods(self, mods):
        self.ht['mods']       = mods
        self.ht['merge_mods'] = False
    #
    @propagate
    def expected_with_merged_mods(self, mods):
        self.ht['mods']       = mods
        self.ht['merge_mods'] = True

##################################################
##################################################
##################################################

density_g = Map_Generator("density/").add([
    Map_Generator("strain/")
        .by_pattern(Ranges.patterns['strain'])
        .on_bpm(),
    Map_Generator("d_spread/")
        .by_bpm()
        .on_each_pattern(Ranges.patterns['d_spread']),
])
density_g.expected_field('density_star')

##################################################

reading_g = Map_Generator("reading/").add([
    Map_Generator("hide/")
        .with_pattern('d')
        .with_abpm(160)
        .on_bpm(Ranges.hide_bpm),
    Map_Generator("no_hide/")
        .with_pattern('d___')
        .with_bpm(40)
        .on_abpm(),
    Map_Generator("mods/").add([
        Map_Generator("mods_HR/")
            .with_pattern('d___')
            .with_bpm(40)
            .by_abpm(Ranges.abpm_mods_HR)
            .expected_with_merged_mods(Ranges.mods_HR)
            .on_this(),
        Map_Generator("mods_HD/")
            .with_pattern('d___')
            .with_bpm(40)
            .by_abpm(Ranges.abpm_mods_HD)
            .expected_with_merged_mods(Ranges.mods_HD)
            .on_this(),
    ]),
])
reading_g.expected_field('reading_star')

##################################################

pattern_g = Map_Generator("pattern/").add([
    Map_Generator("streams/")
        .on_each_pattern(Ranges.patterns['streams']),
    Map_Generator("patterns/")
        .on_each_pattern(Ranges.patterns['patterns']),
])
pattern_g.expected_field('pattern_star')

##################################################

accuracy_g = Map_Generator("accuracy/").add([
    Map_Generator("od/")
        .with_pattern('d')
        .by_rand()
        .on_od(),
    Map_Generator("rand/")
        .with_pattern('d')
        .by_od()
        .on_rand(),
    Map_Generator("slow/")
        .with_pattern('d')
        .with_bpm(40)
        .on_abpm(Ranges.slow_abpm),
])
accuracy_g.expected_field('accuracy_star')

##################################################

other_g = Map_Generator("other/").add([
    Map_Generator("obj/")
        .by_bpm()
        .on_obj()
        .with_pattern('d'),
    Map_Generator("ddkd_reduction/")
        .by_bpm()
        .on_pattern(Ranges.patterns['ddkd_reduction']),
])
other_g.expected_field('final_star')

##################################################

main_g = Dir_Generator("maps_generated/").add([
    density_g,
    reading_g,
    pattern_g,
    accuracy_g,
    other_g,
])

##################################################

COMMENTS = [
    "File generated by '" + __file__ + "'",
    "Tests for 'tr_test_stars.py'"
]
COMMENTS = ''.join(['# ' + line + '\n' for line in COMMENTS])

def expected_list(generator):
    l = []
    for g in generator.get_leaves():
        if isinstance(g, Map_Generator):
            l.append(g.expected())
    return l

def create_expected(generator, path):
    ht = {'tests' : []}
    for g in generator.generators:
        ht['tests'].extend(expected_list(g))
    with open(path, "w+") as f:
        f.write(COMMENTS)
        f.write(yaml.dump(ht))

##################################################

if __name__ == '__main__':
    main_g.main()
    create_expected(main_g,     "yaml/test_generated_main.yaml")
    create_expected(density_g,  "yaml/test_generated_density.yaml")
    create_expected(reading_g,  "yaml/test_generated_reading.yaml")
    create_expected(pattern_g,  "yaml/test_generated_pattern.yaml")
    create_expected(accuracy_g, "yaml/test_generated_accuracy.yaml")
    create_expected(other_g,    "yaml/test_generated_other.yaml")
    print("%d beatmaps created!" % main_g.count_generated())
