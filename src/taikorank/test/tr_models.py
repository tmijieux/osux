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

class TR_Stars():
    def __init__(self, dst, rdg, ptr, acc, fin):
        self.density  = dst
        self.reading  = rdg
        self.pattern  = ptr
        self.accuracy = acc
        self.final    = fin
        self.density_star  = dst
        self.reading_star  = rdg
        self.pattern_star  = ptr
        self.accuracy_star = acc
        self.final_star    = fin
    #
    @classmethod
    def from_yaml(cls, yaml):
        return cls(yaml['density_star'],
                   yaml['reading_star'],
                   yaml['pattern_star'],
                   yaml['accuracy_star'],
                   yaml['final_star'])

############################################################

class TR_Map():
    def __init__(self, title, artist, difficulty, mods, stars):
        self.title  = title
        self.artist = artist
        self.difficulty = difficulty
        self.mods  = mods
        self.stars = stars
    #
    def str_full(self, field, with_mods):
        return ("%s\t(%.4g\t%.4g\t%.4g\t%.4g\t%.4g)\t%s" %
                (getattr(self.stars, field),
                 self.stars.final,
                 self.stars.density,
                 self.stars.reading,
                 self.stars.pattern,
                 self.stars.accuracy,
                 self.str_brief(with_mods)))
    #
    def str_brief(self, with_mods):
        s = str(self)
        if self.mods and with_mods:
            return s + "(%s)" % self.mods
        else:
            return s
    #
    def __str__(self):
        return "%s[%s]" % (self.title, self.difficulty)
    #
    @classmethod
    def from_yaml(cls, yaml):
        return cls(yaml['title'],
                   yaml['artist'],
                   yaml['difficulty'],
                   yaml['mods'],
                   TR_Stars.from_yaml(yaml['stars']))

############################################################

class TR_Map_List(list):
    def sort_by(self, field, reverse = True):
        if len(self) == 0:
            return TR_Map_List()
        if hasattr(self[0], field):
            l = lambda map: getattr(map, field)
        else:
            l = lambda map: getattr(map.stars, field)
        return sorted(self, key = l, reverse = reverse)
    #
    def __str__(self):
        res = ""
        for map in self:
            res += str(map) + '\n'
        return res
    #
    @classmethod
    def from_yaml(cls, yaml, subcls = TR_Map):
        l = cls()
        for map in yaml:
            l.append(subcls.from_yaml(map))
        return l

############################################################

class TR_Object:
    def __init__(self, offset, type, stars):
        self.offset = offset
        self.type   = type
        self.stars  = stars
    #
    @classmethod
    def from_yaml(cls, yaml):
        return cls(yaml['offset'],
                   yaml['type'],
                   TR_Stars.from_yaml(yaml['stars']))

############################################################

class TR_Map_With_Objects(TR_Map):
    def __init__(self, title, artist, difficulty, mods, stars, objects):
        TR_Map.__init__(self, title, artist, difficulty, mods, stars)
        self.objects = objects
    #
    def str_obj(self, unit, max_length):
        s = ""
        for i in range(0, len(self.objects)):
            if i != 0:
                space = (self.objects[i].offset - 
                         self.objects[i-1].offset) / unit
                s += "_" * (int(space) - 1)
            s += self.objects[i].type
        s += " " * (max_length - len(s))
        return s + str(self)
    #
    @classmethod
    def from_yaml(cls, yaml):
        l = []
        for obj in yaml['objects']:
            l.append(TR_Object.from_yaml(obj))
        return cls(yaml['title'],
                   yaml['artist'],
                   yaml['difficulty'],
                   yaml['mods'],
                   TR_Stars.from_yaml(yaml['stars']), l)
