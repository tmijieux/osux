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
import logging
from colors import Colors

########################################################

class TR_Exception(Exception):
    pass

########################################################

class TR_Tester:
    __metaclass__ = abc.ABCMeta
    #
    def __init__(self, argv):
        self.argv = argv[1:]
        self.errors = 0 # local error for one test
        self.total  = 0 # ^ ~same
        self._errors = 0 # global error on all test so far, don't use
        self._total  = 0 # ^ ~same
    #
    def main(self):
        if not self.argv:
            logging.error("No test file!")
            return False
        #
        for arg in self.argv:
            try:
                logging.info(Colors.blue("Test on %s:" % arg))
                err, tot = self.test(arg)
                self._errors += err
                self._total  += tot
            except TR_Exception as e:
                logging.error(str(e))
        #
        logging.info("")
        if self._errors == 0:
            logging.error(Colors.green("All test files passed! %d/%d" %
                          (self._total - self._errors, self._total)))
        else:
            logging.error(Colors.fail("Some test files failed! %d/%d" %
                          (self._total - self._errors, self._total)))
        #
        return self._errors == 0;
    #
    @abc.abstractmethod
    def test(self, arg):
        """
        Abstract method to implement.
        Result must be a tuple (errors, total).
        """
        raise TR_Exception("Abstract method not implemented")
    #
    def test_iterate(self, data, constructor, method):
        for x in data:
            test = constructor(x)
            logging.info("")
            err, tot = method(test)
            self.errors += err
            self.total  += tot

########################################################

class TR_Tester_Yaml(TR_Tester):
    __metaclass__ = abc.ABCMeta
    #
    def __init__(self, argv):
        TR_Tester.__init__(self, argv)
    #
    def test(self, filepath):
        with open(filepath) as f:
            data = yaml.load(f)
            return self.test_yaml(data)
    #
    @abc.abstractmethod
    def test_yaml(self, data):
        """
        Abstract method to implement. Basicely create a TR_test class
        and call main. Result must be a tuple (errors, total).
        """
        raise TR_Exception("Abstract method not implemented")
    #

########################################################

class TR_Test:
    __metaclass__ = abc.ABCMeta
    #
    def __init__(self, name):
        self.name = name
    #
    @staticmethod
    def get_if_exists(key, ht, default):
        return ht[key] if key in ht else default
    #
    def print_start(self):
        logging.info(Colors.blue("Starting test '%s'" % (self.name)))
    #
    def print_result(self, errors, total):
        args = (self.name, total - errors, total)
        if errors == 0:
            logging.warning(Colors.green("Test '%s' passed! %d/%d" % args))
        else:
            logging.warning(Colors.fail("Test '%s' failed! %d/%d" % args))
    #
    def main(self):
        self.print_start()
        self.compute()
        errors, total = self.compare()
        self.print_result(errors, total)
        return (errors, total)
    #
    @abc.abstractmethod
    def compute(self):
        """
        Abstract method to implement for computing maps.
        """
        raise TR_Exception("Abstract method not implemented")
    #
    @abc.abstractmethod
    def compare(self):
        """
        Abstract method to implement for checking results.
        Result must be a tuple (errors, total).
        """
        raise TR_Exception("Abstract method not implemented")    

########################################################

class TR_Test_Expected(TR_Test):
    __metaclass__ = abc.ABCMeta
    #
    def __init__(self, ht):
        TR_Test.__init__(self, ht['path'])
        self.expected = ht['expected']
        if ht['path'][-5:] != "*.osu":
            self.cmd = ht['path'] + "*.osu"
        else:
            self.cmd = ht['path']
        if self.expected and isinstance(self.expected, str):
            self.expected = self.expected.split("\n")[:-1]
    #
    def check_has_expected(self):
        if not self.expected:
            self.print_start()
            logging.warning(Colors.warning("No data, dumping"))
            logging.warning(self.dump_str())
            return False
        return True
    #
    def main(self):
        if self.check_has_expected():
            return TR_Test.main(self)
        else:
            return (0, 0)
    #
    @abc.abstractmethod
    def dump_str(self):
        """
        Abstract method to implement.
        Dump the data as it should be expected.
        """
        raise TR_Exception("Abstract method not implemented")
    #
    def check_same_len(self, res):
        if not self.expected:
            return
        if len(self.expected) != len(res):
            logging.error(Colors.fail("Incorrect list length"))
            logging.error("expected: %d" % (len(self.expected)))
            logging.error("got:      %d" % (len(res)))
            logging.error(str(self.expected))
            logging.error(str(res))
            raise TR_Exception("Incorrect list length")

