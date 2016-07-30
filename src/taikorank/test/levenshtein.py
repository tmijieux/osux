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

import numpy as np

class Levenshtein:
    # with lists
    def __init__(self, l1, l2):
        self.l1 = [None]
        self.l2 = [None]
        self.l1.extend(l1)
        self.l2.extend(l2)
        self.m = np.zeros((len(self.l1), len(self.l2)))
        self.compute()
    #########################################################
    def compute(self):
        self.m[0][0] = 0
        for i in range(1, len(self.l1)):
            self.m[i][0] = i
        for j in range(1, len(self.l2)):
            self.m[j][0] = j
        #
        for i in range(1, len(self.l1)):
            for j in range(1, len(self.l2)):
                if self.l1[i] == self.l2[j]:
                    val = self.m[i-1][j-1] + 0
                else:
                    val = self.m[i-1][j-1] + 1
                self.m[i][j] = min(self.m[i-1][j] + 1,
                                   self.m[i][j-1] + 1,
                                   val)
    #########################################################
    def dist(self):
        return self.m[len(self.l1)-1][len(self.l2)-1]
    #########################################################
