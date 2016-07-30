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
