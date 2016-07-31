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

########################################################

class TR_Type:
    __metaclass__ = abc.ABCMeta
    ########################################################
    def __init__(self, name, rank, include, exclude = []):
        self.name = name
        self.rank = rank
        self.include = include
        self.exclude = exclude
    ########################################################
    def is_tr_type(self, name):
        name = name.lower()
        #
        for s in self.exclude:
            if s in name:
                return False
        #
        for s in self.include:
            if s in name:
                return True
        #
        return False
    ########################################################
    def __str__(self):
        return "%s (%d)" % (self.name, self.rank)
    ########################################################

########################################################

class TR_Type_Util:
    TYPES = {}
    NONE_TYPE = TR_Type('None', -1, [])
    ########################################################
    @staticmethod
    def add_tr_type(name, rank, include, exclude = []):
        TR_Type_Util.TYPES[name] = TR_Type(name, rank, include, exclude)
    ########################################################
    @staticmethod
    def get_tr_type(name):
        for key in TR_Type_Util.TYPES:
            if TR_Type_Util.TYPES[key].is_tr_type(name):
                return TR_Type_Util.TYPES[key]
        return TR_Type_Util.NONE_TYPE
    ########################################################

kantan_include    = ['kantan']
futsuu_include    = ['futsuu', 'futsu', 'fuutsu', 'fuutsuu']
muzukashi_include = ['muzukashi', 'muzukashii', 'muzu']
oni_include       = ['oni']
inner_oni_include = ['inner']
ura_oni_include   = ['ura', 'fatal', 'rampage', 'hell', 'wereoni']

TR_Type_Util.add_tr_type('Kantan',    0, kantan_include)
TR_Type_Util.add_tr_type('Futsuu',    1, futsuu_include,    oni_include)
TR_Type_Util.add_tr_type('Muzukashi', 2, muzukashi_include)
TR_Type_Util.add_tr_type('Oni',       3, oni_include,       inner_oni_include + ura_oni_include)
TR_Type_Util.add_tr_type('Inner Oni', 4, inner_oni_include)
TR_Type_Util.add_tr_type('Ura Oni',   5, ura_oni_include,   muzukashi_include)

def TR_Type_test():
    def assert_tr_type(name, expected):
        assert TR_Type_Util.get_tr_type(name).name == expected
    #
    assert_tr_type("kantan", 'Kantan')
    assert_tr_type("My KanTAN", 'Kantan')
    #
    assert_tr_type("Futsuu", 'Futsuu')
    assert_tr_type("futsu", 'Futsuu')
    assert_tr_type("fuutsu", 'Futsuu')
    #
    assert_tr_type("Muzukashi", 'Muzukashi')
    assert_tr_type("Cqfuj's muzu", 'Muzukashi')
    assert_tr_type("Ura Muzukashi", 'Muzukashi')
    #
    assert_tr_type("Oni", 'Oni')
    assert_tr_type("Oni (futsuu)", 'Oni')
    assert_tr_type("oni (tatsujin)", 'Oni')
    #
    assert_tr_type("Inner Oni", 'Inner Oni')
    assert_tr_type("Nardo's inner", 'Inner Oni')
    #
    assert_tr_type("Ura Oni", 'Ura Oni')
    assert_tr_type("Fatal Oni", 'Ura Oni')
    #
    print("Test done.")

if __name__ == "__main__":
    TR_Type_test()
