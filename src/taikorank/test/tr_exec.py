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
from subprocess import Popen, PIPE, STDOUT

# Set to true if something happen.
# Quite useful when taiko_ranking failed.
DEBUG = False

class TR_Exec:
    EXE = "taiko_ranking"
    OPT = {
        '+db':          0,
        '+ptro':        0,
        '+pyaml':       1,
        '+autoconvert': 0,
        '-score':       0,
        '-quick':       1,
        '-mods':        '__'
    }
    #########################################################
    @staticmethod
    def options(opt = {}):
        copy = TR_Exec.OPT.copy()
        copy.update(opt)
        s = " "
        #
        global_opt = []
        local_opt  = []
        for key, value in copy.items():
            if key[0] == '+':
                global_opt.append((key, value))
            else:
                local_opt.append((key, value))
        #
        for key, value in global_opt:
            s += key + " " + str(value) + " "
        for key, value in local_opt:
            s += key + " " + str(value) + " "
        return s
    #########################################################
    @staticmethod
    def cmd(cmd):
        if DEBUG:
            print("----------------CMD-----------------")
            print(cmd)
        p = Popen(cmd, shell=True, universal_newlines=True,
                  stdout=PIPE, stderr=PIPE, close_fds=True)
        return p.communicate()
    #
    @staticmethod
    def run(args, opt = {}):
        cmd = "%s %s %s" % (TR_Exec.EXE, TR_Exec.options(opt), args)
        return TR_Exec.cmd(cmd)
    #
    @staticmethod
    def debug_print(out, err):
        print("----------------OUT-----------------")
        print(str(out))
        print("----------------ERR-----------------")
        print(str(err))
    #
    @staticmethod
    def compute(args, opt = {}):
        out, err = TR_Exec.run(args, opt)
        #
        if DEBUG:
            TR_Exec.debug_print(out, err)
        #
        try:
            res = yaml.load(out)
        except yaml.parser.ParserError as e:
            print("Failed to parse %s output." % TR_Exec.EXE)
            print("Error: %s" % str(e))
            if not DEBUG:
                TR_Exec.debug_print(out, err)
            return []
        #
        if res == None:
            print("Error loading yaml")
            if not DEBUG:
                TR_Exec.debug_print(out, err)
            return []
        return res;
    #########################################################
