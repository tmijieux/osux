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

from itertools import chain

# Sort ranges from hardest to easiest
class Ranges:
    mods_HR = ['HDFL', 'HRFL', 'FL', 'HR', 'HD', '__', 'EZ']
    mods_HD = ['HDFL', 'HRFL', 'FL', 'HD', 'HR', '__', 'EZ']
    bpm_large  = range(400,  25, -25)
    bpm_normal = range(240, 120, -10)
    bpm_high   = range(480, 240, -20)
    abpm = range(400, 100, -25)
    hide_bpm  = range(640, 160, -80)
    slow_abpm = range(10, 100, 10)
    abpm_mods_HR = chain(range(350, 190, -10), range(190, 179, -1))
    abpm_mods_HD = chain(range(179, 170, -1),  range(170, 120, -10))
    obj  = [2**i for i in range(10, 1, -1)]
    od   = [i/2. for i in range(20, 0, -1)]
    rand = range(30, 0, -5)
    #
    @staticmethod
    def _ggm_state(state, step, first, second):
        ggm = {
            first: state,
            second: 0,
            'step': step
        }
        state1 = range(0, state + step, step)
        state2 = [0] * len(state1)
        if first == 'good':
            ggm['zip'] = [[s1, s2] for s1, s2 in zip(state1, state2)]
        elif first == 'miss':
            ggm['zip'] = [[s2, s1] for s1, s2 in zip(state1, state2)]
        return ggm
    #
    @staticmethod
    def ggm_miss(miss, step):
        return Ranges._ggm_state(miss, step, 'miss', 'good')
    @staticmethod
    def ggm_good(good, step):
        return Ranges._ggm_state(good, step, 'good', 'miss')
    #
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
    patterns['streams']['kd+_change'] = [
        'kdkddkddd'
        'kdkdd'
        'kdd'
        'kd',
        'k',
    ]
    patterns['streams']['kd+_0~4'] = [
        'kdddd',
        'kddd',
        'kdd',
        'kd',
        'k',
    ]
    patterns['streams']['kd+_5~15'] = [
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
    patterns['streams']['d+k+_0~5'] = [
        'dddd d    kkkk k',
        'dddd      kkkk',
        'ddd       kkk',
        'dd        kk',
        'd         k',
        'd',
    ]
    patterns['streams']['d+k+_6~8'] = [
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
    patterns['stream_compression'] = {}
    patterns['stream_compression']['ddkd_kkdk'] = [
        'd_d_ k_d_ k_k_ d_k_',
        'dd__ kd__ kk__ dk__',
        'ddkd ____ kkdk ____',
        'ddkd kkdk ____ ____',
    ]
    patterns['stream_compression']['dddd_kkkk'] = [
        'd_d_ d_d_ k_k_ k_k_',
        'dd__ dd__ kk__ kk__',
        'dddd ____ kkkk ____',
        'dddd kkkk ____ ____',
    ]
    patterns['patterns'] = {}
    patterns['patterns']['7'] = [
        'ddkd kkd_ ____ ____',
        'dddd kkd_ ____ ____',
        'dddd ddk_ ____ ____',
        'dddd ddd_ ____ ____',
    ]
    patterns['patterns']['6'] = [
        'dkdd dk__ ____ ____',
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
    patterns['splits'] = {}
    patterns['splits']['3_ddk'] = [
        'ddk',
        'ddk_',
        'ddk_ _',
        'ddk_ __',
        'ddk_ ___',
        'ddk_ ____',
    ]
    patterns['splits']['4_ddkk'] = [
        'ddkk',
        'ddkk _',
        'ddkk __',
        'ddkk ___',
        'ddkk ____',
    ]
    patterns['splits']['5_ddkkd'] = [
        'ddkk d',
        'ddkk d_',
        'ddkk d__',
        'ddkk d___',
    ]
    patterns['splits']['5_ddkkdk'] = [
        'ddkk dk',
        'ddkk dk_',
        'ddkk dk__',
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
