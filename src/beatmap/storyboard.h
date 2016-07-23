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

#ifndef STORYBOARD_H
#define STORYBOARD_H

struct storyboard {
    /* //Background and Video events */
    

    
    /* //    0,0,"BG.jpg" */
    
    /* //Break Periods */
    /* struct break_periods bp; */
    /* // 2,77678,88564 */
    /* // 2,154553,165828 */
    
    /* //Storyboard Layer 0 (Background) */
    /* struct event *l0; */
    
    /* //Storyboard Layer 1 (Fail) */
    /* struct event *l1; */
    
    /* //Storyboard Layer 2 (Pass) */
    /* struct event *l2; */
    
    /* //Storyboard Layer 3 (Foreground) */
    /* struct event *l3; */
    
    /* //Storyboard Sound Samples */
    /* struct sb_sound_sample sss; */
    
    /* //Background Colour Transformations */
    /* struct sb_colour_transform sct; */
    /* //3,100,163,162,255 */
	void *dummy; // must have at least one member
};

#endif //STORYBOARD_H
