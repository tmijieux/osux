/*
 *  Copyright (©) 2015-2016 Lucas Maugère, Thomas Mijieux
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

#ifndef INTERPOLATION_H
#define INTERPOLATION_H

// ---- Exp Interpolation ----

// f(X1) == Y1 && f(X2) == Y2
// x is the parameter
#define EXP_2_PT(x, X1, Y1, X2, Y2)		\
    ((Y1) * exp(((x) - (X1)) /			\
		((X2) - (X1)) *			\
		log((Y2) / (Y1))))

// ---- Inverse ----

// f(X1) == Y1 && f(X2) == Y2
// x is the parameter
#define INV_2_PT(x, X1, Y1, X2, Y2)			\
    (1. /						\
     ((x) /						\
      ((Y1) *						\
       ((X1) +						\
	(((Y2) - (Y1)) / ((X1) * (Y1) - (X2) * (Y2)))))	\
      +							\
      (((Y2) - (Y1)) / ((X1) * (Y1) - (X2) * (Y2)))))

// ---- Lagrangian Interpolation ----

// f(X1) == Y1 && f(X2) == Y2
// x is the parameter
#define POLY_2_PT(x, X1, Y1, X2, Y2)				\
    ((Y1) + ((((Y2) - (Y1)) / ((X1) - (X2)))) * ((X1) - (x)))

// f(X1) == Y1 && f(X2) == Y2 && f(X3) == Y3
// x is the parameter
#define POLY_3_PT(x, X1, Y1, X2, Y2, X3, Y3)	\
    ((Y1) * ((((x) - (X2)) / ((X1) - (X2))) *	\
	     (((x) - (X3)) / ((X1) - (X3))))	\
     +						\
     (Y2) * ((((x) - (X1)) / ((X2) - (X1))) *	\
	     (((x) - (X3)) / ((X2) - (X3))))	\
     +						\
     (Y3) * ((((x) - (X1)) / ((X3) - (X1))) *	\
	     (((x) - (X2)) / ((X3) - (X2)))))

// ---- Scaling ----
/*
// increase or decrease scaling
// x is the parameter
// POW is the power used
// (x == Y) => (f(x) == Y)
#define SCALING(x, POW, Y)			\
(pow(x, POW) / pow(Y, POW - 1))
// ^ work weirdly
*/
#endif //INTERPOLATION_H
