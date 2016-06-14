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

from subprocess import Popen, PIPE, STDOUT
import yaml
import sys

# Set to true if something happen.
# Quite useful when taiko_ranking failed.
DEBUG=False

class Colors:
    BLUE    = '\033[94m'
    WARNING = '\033[93m'
    GREEN   = '\033[92m'
    FAIL    = '\033[91m'
    END     = '\033[0m'
    #########################################################
    @staticmethod
    def green(s):
        return Colors.GREEN + s + Colors.END
    #
    @staticmethod
    def blue(s):
        return Colors.BLUE + s + Colors.END
    #
    @staticmethod
    def fail(s):
        return Colors.FAIL + s + Colors.END
    #
    @staticmethod
    def warning(s):
        return Colors.WARNING + s + Colors.END
    #########################################################

class TR_exec:
    EXE = "taiko_ranking"
    OPT = "-db 0 -ptro 0 -pyaml 1 -quick 1 -score 0"
    #########################################################
    @staticmethod
    def cmd(cmd):
        p = Popen(cmd, shell=True, universal_newlines=True,
                  stdout=PIPE, stderr=PIPE, close_fds=True)
        return p.communicate()
    #
    @staticmethod
    def run(args):
        cmd = "%s %s %s" % (TR_exec.EXE, TR_exec.OPT, args)
        return TR_exec.cmd(cmd)
    #########################################################

class TR_test:
    def __init__(self, ht):
        if 'do' in ht:
            self.do = ht['do']
        else:
            self.do = True
        #
        if 'field' in ht:
            self.field = ht['field']
        else:
            self.field = 'final_star'
        #
        self.path = ht['path']
        if self.path[-5:] != "*.osu":
            self.cmd = self.path + "*.osu"
        else:
            self.cmd = self.path
        #
        self.maps = ht['maps']
        self.computed = False
    #########################################################
    def combine(self, tests):
        res = []
        for key in self.path.split(" "):
            res.extend(tests[key].res)
        self.res = sorted(res, key=lambda k: k['stars'][self.field],
                          reverse=True)
        self.computed = True
    #########################################################
    def str_full(self, data):
        return ("%s\t(%.4g\t%.4g\t%.4g\t%.4g\t%.4g)\t%s[%s]" % 
                (data['stars'][self.field], 
                 data['stars']['final_star'], 
                 data['stars']['density_star'], 
                 data['stars']['reading_star'], 
                 data['stars']['pattern_star'], 
                 data['stars']['accuracy_star'],
                 data['title'], data['difficulty']))
    #########################################################
    def str_brief(self, data):
        return ("%s[%s]" % (data['title'], data['difficulty']))
    #########################################################
    def compute(self):
        out, err = TR_exec.run(self.cmd)
        #
        if DEBUG:
            print(str(out))
            print(str(err))
        #
        res = yaml.load(out)
        if res == None:
            print("Error loading yaml", str(err))
            return
        #
        res = res['maps']
        self.res = sorted(res, key=lambda k: k['stars'][self.field],
                            reverse=True)
    #########################################################
    def compare(self):
        self.computed = True
        STR_OK   = "%s - %%s"       % (Colors.green("OK"))
        STR_FAIL = "%s - %%s {%%s}" % (Colors.fail("<>"))
        errors = 0
        maps = self.maps.split("\n")[:-1]
        if len(maps) != len(self.res):
            print(Colors.fail("Incorrect list length"))
            print("maps(%d) res(%d)" % (len(maps), len(self.res)))
            return 1
        zipped = zip(maps, self.res)
        for origin, test in zipped:
            s_brief = self.str_brief(test)
            s_full  = self.str_full(test)
            if origin == s_brief:
                print(STR_OK % (s_full))
            else:
                print(STR_FAIL % (s_full, Colors.warning(origin)))
                errors += 1
        return errors
    #########################################################
    def dump_brief(self):
        for x in self.res:
            print(self.str_brief(x))
    #########################################################
    def dump_full(self):
        for x in self.res:
            print(self.str_full(x))
    ########################################################
    def main(self):
        print(Colors.blue("Starting test '%s' on '%s'" %
                          (self.path, self.field)))
        #
        if not self.computed:
            self.compute()
        errors = 0
        if not self.maps:
            print(Colors.warning("No data, dumping."))
            self.dump_brief()
        else:
            errors = self.compare()
        #
        if errors == 0:
            print(Colors.green("Test '%s' passed!" % self.path))
        else:
            print(Colors.fail("Test '%s' failed! errors: %d" %
                              (self.path, errors)))
        return errors
    ########################################################

def test_file(filepath):
    print(Colors.blue("Test on %s:" % filepath))
    with open(filepath) as f:
        data = yaml.load(f)
    #
    errors = 0
    tests = {}
    for x in data['tests']:
        test = TR_test(x)
        tests[test.path] = test
        if test.do:
            print("")
            errors += test.main()
    #
    if 'combined' in data:
        for x in data['combined']:
            test = TR_test(x)
            tests[test.path] = test
            if test.do:
                print("")
                test.combine(tests)
                test.main()
    #
    print("")
    if errors == 0:
        print(Colors.green("All test passed!"))
    else:
        print(Colors.fail("Some test failed! errors: %s" % errors))

########################################################

if __name__ == "__main__":
    if (len(sys.argv) < 2):
        print("No test file!")
        sys.exit(0)
    for filepath in sys.argv[1:]:
        test_file(filepath)
