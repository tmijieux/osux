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

import abc
import yaml
from colors import Colors

########################################################

class TR_Exception(Exception):
    pass

########################################################

class TR_Tester:
    __metaclass__ = abc.ABCMeta
    ########################################################
    def __init__(self, argv):
        self.argv = argv[1:]
        self.errors = 0 # local error for one test
        self.total  = 0 # ^ ~same
        self._errors = 0 # global error on all test so far, don't use
        self._total  = 0 # ^ ~same
    ########################################################
    def main(self):
        if not self.argv:
            print("No test file!")
            return False
        #
        for arg in self.argv:
            try:
                print(Colors.blue("Test on %s:" % arg))
                err, tot = self.test(arg)
                self._errors += err
                self._total  += tot
            except TR_Exception as e:
                print(str(e))
        #
        print("")
        if self._errors == 0:
            print(Colors.green("All test files passed! %d/%d" %
                    (self._total - self._errors, self._total)))
        else:
            print(Colors.fail("Some test files failed! %d/%d" %
                    (self._total - self._errors, self._total)))
        #
        return self._errors == 0;
    ########################################################
    @abc.abstractmethod
    def test(self, arg):
        """
        Abstract method to implement.
        Result must be a tuple (errors, total).
        """
        raise TR_Exception("Abstract method not implemented")
    ########################################################
    def test_iterate(self, data, constructor, method):
        for x in data:
            test = constructor(x)
            if test.do:
                print("")
                err, tot = method(test)
                self.errors += err
                self.total  += tot
    ########################################################

########################################################

class TR_Tester_Yaml(TR_Tester):
    __metaclass__ = abc.ABCMeta
    ########################################################
    def __init__(self, argv):
        TR_Tester.__init__(self, argv)
    ########################################################
    def test(self, filepath):
        with open(filepath) as f:
            data = yaml.load(f)
            return self.test_yaml(data)
    ########################################################
    @abc.abstractmethod
    def test_yaml(self, data):
        """
        Abstract method to implement. Basicely create a TR_test class
        and call main. Result must be a tuple (errors, total).
        """
        raise TR_Exception("Abstract method not implemented")
    ########################################################


########################################################
########################################################
########################################################

class TR_Test_Yaml:
    __metaclass__ = abc.ABCMeta
    ########################################################
    @staticmethod
    def get_if_exists(key, ht, default):
        return ht[key] if key in ht else default
    ########################################################
    def __init__(self, ht):
        self.name     = ht['path']
        self.expected = ht['expected']
        self.do = self.get_if_exists('do', ht, True);
        #
        if ht['path'][-5:] != "*.osu":
            self.cmd = ht['path'] + "*.osu"
        else:
            self.cmd = ht['path']
    ########################################################
    def main(self):
        print(Colors.blue("Starting test '%s'" % (self.name)))
        #
        self.compute()
        if not self.expected:
            print(Colors.warning("No data, dumping"))
            self.dump()
            return (0, 0)
        #
        errors, total = self.compare()
        if errors == 0:
            print(Colors.green("Test '%s' passed! %d/%d" % (self.name, total - errors, total)))
        else:
            print(Colors.fail("Test '%s' failed! %d/%d" % (self.name, total - errors, total)))
        return (errors, total)
    ########################################################
    @abc.abstractmethod
    def dump(self):
        """
        Abstract method to implement.
        Dump the data as it should be expected.
        """
        raise TR_Exception("Abstract method not implemented")
    ########################################################
    @abc.abstractmethod
    def compute(self):
        """
        Abstract method to implement for computing maps.
        """
        raise TR_Exception("Abstract method not implemented")
    ########################################################
    @abc.abstractmethod
    def compare(self):
        """
        Abstract method to implement for checking results.
        Result must be a tuple (errors, total).
        """
        raise TR_Exception("Abstract method not implemented")
    ########################################################

########################################################

class TR_Test_Str_List(TR_Test_Yaml):
    __metaclass__ = abc.ABCMeta
    ########################################################
    def __init__(self, ht):
        TR_Test_Yaml.__init__(self, ht)
        if self.expected:
            self.expected = self.expected.split("\n")[:-1]
    ########################################################
    def check_same_len(self, res):
        if not self.expected:
            return
        if len(self.expected) != len(res):
            print(Colors.fail("Incorrect list length"))
            print("expected: %d" % (len(self.expected)))
            print("got:      %d" % (len(res)))
            print(str(self.expected))
            print(str(res))
            raise TR_Exception("Incorrect list length")
