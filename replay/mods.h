/*
 *  Copyright (©) 2015 Lucas Maugère, Thomas Mijieux
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef MODS_H
#define MODS_H

enum MODS {
    NOMOD		=	0,
    MOD_NOFAIL 		= 	(1 << 0),
    MOD_EASY  		=	(1 << 1),
    MOD_NOVIDEO		=	(1 << 2),
    MOD_HIDDEN		=	(1 << 3),
    MOD_HARDROCK	=	(1 << 4),
    MOD_SUDDENDEATH	=	(1 << 5),
    MOD_DOUBLETIME	=	(1 << 6),
    MOD_RELAX		=	(1 << 7),
    MOD_HALFTIME	=	(1 << 8),
    MOD_NIGHTCORE	=	(1 << 9),
    MOD_FLASHLIGHT	=	(1 << 10),
    MOD_AUTOPILOT	=	(1 << 13),
    MOD_PERFECT 	=	(1 << 14),
    
    MOD_4K		=	(1 << 15),
    MOD_5K		=	(1 << 16),
    MOD_6K		= 	(1 << 17),
    MOD_7K		= 	(1 << 18),
    MOD_8K		=	(1 << 19),

    MOD_KEYMOD		=	(MOD_4K | MOD_5K | MOD_6K | MOD_7K | MOD_8K),
    MOD_FADEIN		=	(1 << 20),
    MOD_RANDOM		=	(1 << 21),
    MOD_CINEMA		= 	(1 << 22),

    MOD_TARGETPRACTICE	=	(1 << 23),
    
    MOD_9K		=	(1 << 24),
    MOD_COOP		=	(1 << 25),

    MOD_3K		=	(1 << 27),
    MOD_2K		=	(1 << 28),
    MOD_1K		=	(1 << 29),

    MOD_1K_COOP		=	MOD_2K,
    MOD_2K_COOP		=	MOD_4K,
    MOD_3K_COOP		=	MOD_6K,
    MOD_4K_COOP		=	MOD_8K,

    MOD_5K_COOP		=	(MOD_5K | MOD_COOP),
    MOD_6K_COOP		=	(MOD_6K | MOD_COOP),
    MOD_7K_COOP		=	(MOD_7K | MOD_COOP),
    MOD_8K_COOP		=	(MOD_8K | MOD_COOP),
    MOD_9K_COOP		=	(MOD_9K | MOD_COOP)

};

#endif //MODS_H
