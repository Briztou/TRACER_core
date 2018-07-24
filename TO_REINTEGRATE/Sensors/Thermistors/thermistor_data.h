/*
  thermistor_data.h - Part of TRACER

  All data in this file is extracted from Marlin

  Copyright for data :

  Copyright (C) 2016 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
  (Marlin Based on Sprinter and grbl)
  Copyright (C) 2011 Camiel Gubbels / Erik van der Zalm

  Copyrignt for the adaptation :

  Copyright (c) 2017 Raphaël Outhier - reprap.org (all data in this file comes from the reprap website)

  TRACER is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  TRACER is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  aint32_t with TRACER.  If not, see <http://www.gnu.org/licenses/>.

*/

/*
 * This file_tree contains all data related to thermistors.
 *
 * All data in this file_tree is extracted from the file_tree thermistor_data.h in Marlin, all credits go to authors ! :-)
 * *
 * It comprises lookup table for each thermistor that the code can deal with, and the table size for each one.
 *
 * You may use macros provided here to populate the thermistor section of you configuration file_tree.
 *
 * To do so, you may use the next names
 *
 *  4,7K Pullup :
 *
 *  100k bed thermistor                             BED100K
 *  mendel-parts MENDEL_                            MENDEL
 *  10k thermistor T10K_                            T10K
 *  200k ATC Semitec 204GT-2                        SEMITEC_200_47
 *  100k ATC Semitec 104GT-2                        SEMITEC_100_47
 *  100k Epcos thermistor                           EPCOS100_47
 *  100k Honeywell 135-104LAG-J01                   HLAG100
 *  100k GE Sensing AL03006-58.2K-97-G1             GE100
 *  100k Honeywell 135-104LAF-J01                   HLAF100
 *  100k RS thermistor 198-961                      RS100
 *  QU-BD silicone bed QWG-104F-3950 thermistor     QUBD
 *  Hisens thermistor B25/50 =3950 +/-1%            HISENS
 *  PT100 with INA826 amp on Ultimaker v2.0         INA826
 *  Maker's Tool Works Kapton Bed Thermistor        KAPTON
 *  100k 0603 SMD Vishay NTCS0603E3104FXT           VISH100
 *  bqh2 stock thermistor                           BQH2
 *
 *  1K Pullup :
 *
 *  100k EPCOS 1K                                   EPCOS100_1K
 *  200k ATC Semitec 204GT-2                        SEMITEC_200
 *  100k ATC Semitec 104GT-2                        SEMITEC_100

 *
 *
 * You must write_data a line like the following in your config_files.h to enable a particular thermistor, named [NAME] :
 *
 * THEMRISTOR(index,  pin, NAME)
 *      where index is the thermistor index, pin is the pin it will read_data, and NAME is the thermistor name
 *
 */



//---------------------------------------------------4.7 KOhm Pullup----------------------------------------------------

//100k bed thermistor
#define BED100K_SIZE 61
#define BED100K_TABLE {\
  {23, 300},\
  {25, 295},\
  {27, 290},\
  {28, 285},\
  {31, 280},\
  {33, 275},\
  {35, 270},\
  {38, 265},\
  {41, 260},\
  {44, 255},\
  {48, 250},\
  {52, 245},\
  {56, 240},\
  {61, 235},\
  {66, 230},\
  {71, 225},\
  {78, 220},\
  {84, 215},\
  {92, 210},\
  {100, 205},\
  {109, 200},\
  {120, 195},\
  {131, 190},\
  {143, 185},\
  {156, 180},\
  {171, 175},\
  {187, 170},\
  {205, 165},\
  {224, 160},\
  {245, 155},\
  {268, 150},\
  {293, 145},\
  {320, 140},\
  {348, 135},\
  {379, 130},\
  {411, 125},\
  {445, 120},\
  {480, 115},\
  {516, 110},\
  {553, 105},\
  {591, 100},\
  {628, 95},\
  {665, 90},\
  {702, 85},\
  {737, 80},\
  {770, 75},\
  {801, 70},\
  {830, 65},\
  {857, 60},\
  {881, 55},\
  {903, 50},\
  {922, 45},\
  {939, 40},\
  {954, 35},\
  {966, 30},\
  {977, 25},\
  {985, 20},\
  {993, 15},\
  {999, 10},\
  {1004, 5},\
  {1008, 0}\
};




//mendel-parts
#define MENDEL_SIZE 28
#define MENDEL_TABLE {\
  {1, 864},\
  {21, 300},\
  {25, 290},\
  {29, 280},\
  {33, 270},\
  {39, 260},\
  {46, 250},\
  {54, 240},\
  {64, 230},\
  {75, 220},\
  {90, 210},\
  {107, 200},\
  {128, 190},\
  {154, 180},\
  {184, 170},\
  {221, 160},\
  {265, 150},\
  {316, 140},\
  {375, 130},\
  {441, 120},\
  {513, 110},\
  {588, 100},\
  {734, 80},\
  {856, 60},\
  {938, 40},\
  {986, 20},\
  {1008, 0},\
  {1018, -20}\
};

//10k thermistor
#define T10K_SIZE 20
#define T10K_TABLE {\
  {1, 430},\
  {54, 137},\
  {107, 107},\
  {160, 91},\
  {213, 80},\
  {266, 71},\
  {319, 64},\
  {372, 57},\
  {425, 51},\
  {478, 46},\
  {531, 41},\
  {584, 35},\
  {637, 30},\
  {690, 25},\
  {743, 20},\
  {796, 14},\
  {849, 7},\
  {902, 0},\
  {955, -11},\
  {1008, -35}\
};


//200k bed thermistor
//200k ATC Semitec 204GT-2
//Verified by linagee. Source: http://shop.arcol.hu/static/datasheets/thermistors.pdf
// Calculated using 4.7kohm pullup, voltage divider math, and manufacturer provided temp/resistance

#define SEMITEC_200_47_SIZE 32
#define SEMITEC_200_47_TABLE {\
  {1, 848},\
  {30, 300},\
  {34, 290},\
  {39, 280},\
  {46, 270},\
  {53, 260},\
  {63, 250},\
  {74, 240},\
  {87, 230},\
  {104, 220},\
  {124, 210},\
  {148, 200},\
  {176, 190},\
  {211, 180},\
  {252, 170},\
  {301, 160},\
  {357, 150},\
  {420, 140},\
  {489, 130},\
  {562, 120},\
  {636, 110},\
  {708, 100},\
  {775, 90},\
  {835, 80},\
  {884, 70},\
  {924, 60},\
  {955, 50},\
  {977, 40},\
  {993, 30},\
  {1004, 20},\
  {1012, 10},\
  {1016, 0}\
};


//100k ATC Semitec 104GT-2 (Used in ParCan)
// Verified by linagee. Source: http://shop.arcol.hu/static/datasheets/thermistors.pdf
// Calculated using 4.7kohm pullup, voltage divider math, and manufacturer provided temp/resistance

#define SEMITEC_100_47_SIZE 32
#define SEMITEC_100_47_TABLE {\
  {1, 713},\
  {17, 300},\
  {20, 290},\
  {23, 280},\
  {27, 270},\
  {31, 260},\
  {37, 250},\
  {43, 240},\
  {51, 230},\
  {61, 220},\
  {73, 210},\
  {87, 200},\
  {106, 190},\
  {128, 180},\
  {155, 170},\
  {189, 160},\
  {230, 150},\
  {278, 140},\
  {336, 130},\
  {402, 120},\
  {476, 110},\
  {554, 100},\
  {635, 90},\
  {713, 80},\
  {784, 70},\
  {846, 60},\
  {897, 50},\
  {937, 40},\
  {966, 30},\
  {986, 20},\
  {1000, 10},\
  {1010, 0}\
};



// 100k Epcos thermistor, 4.7K pullup
#define EPCOS100_47_SIZE 38
#define EPCOS100_47_TABLE {\
  {1, 350},\
  {28, 250},\
  {31, 245},\
  {35, 240},\
  {39, 235},\
  {42, 230},\
  {44, 225},\
  {49, 220},\
  {53, 215},\
  {62, 210},\
  {71, 205},\
  {78, 200},\
  {94, 190},\
  {102, 185},\
  {116, 170},\
  {143, 160},\
  {183, 150},\
  {223, 140},\
  {270, 130},\
  {318, 120},\
  {383, 110},\
  {413, 105},\
  {439, 100},\
  {484, 95},\
  {513, 90},\
  {607, 80},\
  {664, 70},\
  {781, 60},\
  {810, 55},\
  {849, 50},\
  {914, 45},\
  {914, 40},\
  {935, 35},\
  {954, 30},\
  {970, 25},\
  {978, 22},\
  {1008, 3},\
  {1023, 0}\
};


// 100k Honeywell 135-104LAG-J01
#define HLAG100_SIZE 58
#define HLAG100_TABLE {\
  {1, 941},\
  {19, 362},\
  {37, 299},\
  {55, 266},\
  {73, 245},\
  {91, 229},\
  {109, 216},\
  {127, 206},\
  {145, 197},\
  {163, 190},\
  {181, 183},\
  {199, 177},\
  {217, 171},\
  {235, 166},\
  {253, 162},\
  {271, 157},\
  {289, 153},\
  {307, 149},\
  {325, 146},\
  {343, 142},\
  {361, 139},\
  {379, 135},\
  {397, 132},\
  {415, 129},\
  {433, 126},\
  {451, 123},\
  {469, 121},\
  {487, 118},\
  {505, 115},\
  {523, 112},\
  {541, 110},\
  {559, 107},\
  {577, 105},\
  {595, 102},\
  {613, 99},\
  {631, 97},\
  {649, 94},\
  {667, 92},\
  {685, 89},\
  {703, 86},\
  {721, 84},\
  {739, 81},\
  {757, 78},\
  {775, 75},\
  {793, 72},\
  {811, 69},\
  {829, 66},\
  {847, 62},\
  {865, 59},\
  {883, 55},\
  {901, 51},\
  {919, 46},\
  {937, 41},\
  {955, 35},\
  {973, 27},\
  {991, 17},\
  {1009, 1},\
  {1023, 0}\
};


// 100k Honeywell 135-104LAF-J01
// R0 = 100000 Ohm
// T0 = 25 Â°C
// Beta = 3974
// R1 = 0 Ohm
// R2 = 4700 Ohm
#define HLAF100_SIZE 141
#define HLAF100_TABLE {\
  {35, 300},\
  {51, 270},\
  {54, 265},\
  {58, 260},\
  {59, 258},\
  {61, 256},\
  {63, 254},\
  {64, 252},\
  {66, 250},\
  {67, 249},\
  {68, 248},\
  {69, 247},\
  {70, 246},\
  {71, 245},\
  {72, 244},\
  {73, 243},\
  {74, 242},\
  {75, 241},\
  {76, 240},\
  {77, 239},\
  {78, 238},\
  {79, 237},\
  {80, 236},\
  {81, 235},\
  {82, 234},\
  {84, 233},\
  {85, 232},\
  {86, 231},\
  {87, 230},\
  {89, 229},\
  {90, 228},\
  {91, 227},\
  {92, 226},\
  {94, 225},\
  {95, 224},\
  {97, 223},\
  {98, 222},\
  {99, 221},\
  {101, 220},\
  {102, 219},\
  {104, 218},\
  {106, 217},\
  {107, 216},\
  {109, 215},\
  {110, 214},\
  {112, 213},\
  {114, 212},\
  {115, 211},\
  {117, 210},\
  {119, 209},\
  {121, 208},\
  {123, 207},\
  {125, 206},\
  {126, 205},\
  {128, 204},\
  {130, 203},\
  {132, 202},\
  {134, 201},\
  {136, 200},\
  {139, 199},\
  {141, 198},\
  {143, 197},\
  {145, 196},\
  {147, 195},\
  {150, 194},\
  {152, 193},\
  {154, 192},\
  {157, 191},\
  {159, 190},\
  {162, 189},\
  {164, 188},\
  {167, 187},\
  {170, 186},\
  {172, 185},\
  {175, 184},\
  {178, 183},\
  {181, 182},\
  {184, 181},\
  {187, 180},\
  {190, 179},\
  {193, 178},\
  {196, 177},\
  {199, 176},\
  {202, 175},\
  {205, 174},\
  {208, 173},\
  {212, 172},\
  {215, 171},\
  {219, 170},\
  {237, 165},\
  {256, 160},\
  {300, 150},\
  {351, 140},\
  {470, 120},\
  {504, 115},\
  {538, 110},\
  {552, 108},\
  {566, 106},\
  {580, 104},\
  {594, 102},\
  {608, 100},\
  {622, 98},\
  {636, 96},\
  {650, 94},\
  {664, 92},\
  {678, 90},\
  {712, 85},\
  {745, 80},\
  {758, 78},\
  {770, 76},\
  {783, 74},\
  {795, 72},\
  {806, 70},\
  {818, 68},\
  {829, 66},\
  {840, 64},\
  {850, 62},\
  {860, 60},\
  {870, 58},\
  {879, 56},\
  {888, 54},\
  {897, 52},\
  {905, 50},\
  {924, 45},\
  {940, 40},\
  {955, 35},\
  {967, 30},\
  {970, 29},\
  {972, 28},\
  {974, 27},\
  {976, 26},\
  {978, 25},\
  {980, 24},\
  {982, 23},\
  {984, 22},\
  {985, 21},\
  {987, 20},\
  {995, 15},\
  {1001, 10},\
  {1006, 5},\
  {1010, 0},\
};


// 100k GE Sensing AL03006-58.2K-97-G1 (4.7k pullup)
#define GE100_SIZE 31
#define GE100_TABLE {\
  {1, 936},\
  {36, 300},\
  {71, 246},\
  {106, 218},\
  {141, 199},\
  {176, 185},\
  {211, 173},\
  {246, 163},\
  {281, 155},\
  {316, 147},\
  {351, 140},\
  {386, 134},\
  {421, 128},\
  {456, 122},\
  {491, 117},\
  {526, 112},\
  {561, 107},\
  {596, 102},\
  {631, 97},\
  {666, 92},\
  {701, 87},\
  {736, 81},\
  {771, 76},\
  {806, 70},\
  {841, 63},\
  {876, 56},\
  {911, 48},\
  {946, 38},\
  {981, 23},\
  {1005, 5},\
  {1016, 0}\
};


// 100k RS thermistor 198-961 (4.7k pullup)
#define RS100_SIZE 31
#define RS100_TABLE {\
  {1, 929},\
  {36, 299},\
  {71, 246},\
  {106, 217},\
  {141, 198},\
  {176, 184},\
  {211, 173},\
  {246, 163},\
  {281, 154},\
  {316, 147},\
  {351, 140},\
  {386, 134},\
  {421, 128},\
  {456, 122},\
  {491, 117},\
  {526, 112},\
  {561, 107},\
  {596, 102},\
  {631, 97},\
  {666, 91},\
  {701, 86},\
  {736, 81},\
  {771, 76},\
  {806, 70},\
  {841, 63},\
  {876, 56},\
  {911, 48},\
  {946, 38},\
  {981, 23},\
  {1005, 5},\
  {1016, 0}\
};



// QU-BD silicone bed QWG-104F-3950 thermistor
#define QUBD_SIZE 50
#define QUBD_TABLE {\
  {1, 938},\
  {31,314},\
  {41,290},\
  {51,272},\
  {61,258},\
  {71,247},\
  {81,237},\
  {91,229},\
  {101, 221},\
  {111, 215},\
  {121, 209},\
  {131, 204},\
  {141, 199},\
  {151, 195},\
  {161, 190},\
  {171, 187},\
  {181, 183},\
  {191, 179},\
  {201, 176},\
  {221, 170},\
  {241, 165},\
  {261, 160},\
  {281, 155},\
  {301, 150},\
  {331, 144},\
  {361, 139},\
  {391, 133},\
  {421, 128},\
  {451, 123},\
  {491, 117},\
  {531, 111},\
  {571, 105},\
  {611, 100},\
  {641, 95},\
  {681, 90},\
  {711, 85},\
  {751, 79},\
  {791, 72},\
  {811, 69},\
  {831, 65},\
  {871, 57},\
  {881, 55},\
  {901, 51},\
  {921, 45},\
  {941, 39},\
  {971, 28},\
  {981, 23},\
  {991, 17},\
  {1001,9},\
  {1021,-27}\
};



// Hisens thermistor B25/50 =3950 +/-1%
#define HISENS_SIZE 23
#define HISENS_TABLE {\
  { 20.04, 300 },\
  { 23.19, 290 },\
  { 26.71, 280 },\
  { 31.23, 270 },\
  { 36.52, 260 },\
  { 42.75, 250 },\
  { 50.68, 240 },\
  { 60.22, 230 },\
  { 72.03, 220 },\
  { 86.84, 210 },\
  { 102.79, 200 },\
  { 124.46, 190 },\
  { 151.02, 180 },\
  { 182.86, 170 },\
  { 220.72, 160 },\
  { 316.96, 140 },\
  { 447.17, 120 },\
  { 590.61, 100 },\
  { 737.31, 80 },\
  { 857.77, 60 },\
  { 939.52, 40 },\
  { 986.03, 20 },\
  { 1008.7, 0}\
};


// PT100 with INA826 amp on Ultimaker v2.0 electronics
/* The PT100 in the Ultimaker v2.0 electronics has a high sample value for a high temperature.
This does not match the normal thermistor behaviour so we need to set the following defines */
#define INA826_SIZE 49
#define INA826_TABLE {\
  {0 ,0},\
  {227 ,1},\
  {236 ,10},\
  {245 ,20},\
  {253 ,30},\
  {262 ,40},\
  {270 ,50},\
  {279 ,60},\
  {287 ,70},\
  {295 ,80},\
  {304 ,90},\
  {312 ,100},\
  {320 ,110},\
  {329 ,120},\
  {337 ,130},\
  {345 ,140},\
  {353 ,150},\
  {361 ,160},\
  {369 ,170},\
  {377 ,180},\
  {385 ,190},\
  {393 ,200},\
  {401 ,210},\
  {409 ,220},\
  {417 ,230},\
  {424 ,240},\
  {432 ,250},\
  {440 ,260},\
  {447 ,270},\
  {455 ,280},\
  {463 ,290},\
  {470 ,300},\
  {478 ,310},\
  {485 ,320},\
  {493 ,330},\
  {500 ,340},\
  {507 ,350},\
  {515 ,360},\
  {522 ,370},\
  {529 ,380},\
  {537 ,390},\
  {544 ,400},\
  {614 ,500},\
  {681 ,600},\
  {744 ,700},\
  {805 ,800},\
  {862 ,900},\
  {917 ,1000},\
  {968 ,1100}\
};




// Maker's Tool Works Kapton Bed Thermistor
// ./createTemperatureLookup.py --r0=100000 --t0=25 --r1=0 --r2=4700 --beta=3950
// r0: 100000
// t0: 25
// r1: 0 (parallel with rTherm)
// r2: 4700 (series with rTherm)
// beta: 3950
// min adc: 1 at 0.0048828125 V
// max adc: 1023 at 4.9951171875 V
#define KAPTON_SIZE 72
#define KAPTON_TABLE {\
  {51, 272},\
  {61, 258},\
  {71, 247},\
  {81, 237},\
  {91, 229},\
  {101, 221},\
  {131, 204},\
  {161, 190},\
  {191, 179},\
  {231, 167},\
  {271, 157},\
  {311, 148},\
  {351, 140},\
  {381, 135},\
  {411, 130},\
  {441, 125},\
  {451, 123},\
  {461, 122},\
  {471, 120},\
  {481, 119},\
  {491, 117},\
  {501, 116},\
  {511, 114},\
  {521, 113},\
  {531, 111},\
  {541, 110},\
  {551, 108},\
  {561, 107},\
  {571, 105},\
  {581, 104},\
  {591, 102},\
  {601, 101},\
  {611, 100},\
  {621, 98},\
  {631, 97},\
  {641, 95},\
  {651, 94},\
  {661, 92},\
  {671, 91},\
  {681, 90},\
  {691, 88},\
  {701, 87},\
  {711, 85},\
  {721, 84},\
  {731, 82},\
  {741, 81},\
  {751, 79},\
  {761, 77},\
  {771, 76},\
  {781, 74},\
  {791, 72},\
  {801, 71},\
  {811, 69},\
  {821, 67},\
  {831, 65},\
  {841, 63},\
  {851, 62},\
  {861, 60},\
  {871, 57},\
  {881, 55},\
  {891, 53},\
  {901, 51},\
  {911, 48},\
  {921, 45},\
  {931, 42},\
  {941, 39},\
  {951, 36},\
  {961, 32},\
  {981, 23},\
  {991, 17},\
  {1001, 9},\
  {1008, 0},\
};



//100k 0603 SMD Vishay NTCS0603E3104FXT (4.7k pullup) (calibrated for Makibox hot bed)
#define VISH100_SIZE 30
#define VISH100_TABLE {\
  {35, 180},\
  {211, 140},\
  {233, 135},\
  {261, 130},\
  {290, 125},\
  {328, 120},\
  {362, 115},\
  {406, 110},\
  {446, 105},\
  {496, 100},\
  {539, 95},\
  {585, 90},\
  {629, 85},\
  {675, 80},\
  {718, 75},\
  {758, 70},\
  {793, 65},\
  {822, 60},\
  {841, 55},\
  {875, 50},\
  {899, 45},\
  {926, 40},\
  {946, 35},\
  {962, 30},\
  {977, 25},\
  {987, 20},\
  {995, 15},\
  {1001, 10},\
  {1010, 0},\
  {1023, -40},\
};



// bqh2 stock thermistor
#define BQH2_SIZE 31
#define BQH2_TABLE {\
   {   22 , 300 },\
   {   24 , 295 },\
   {   25 , 290 },\
   {   27 , 285 },\
   {   29 , 280 },\
   {   32 , 275 },\
   {   34 , 270 },\
   {   37 , 265 },\
   {   40 , 260 },\
   {   43 , 255 },\
   {   46 , 250 },\
   {   50 , 245 },\
   {   54 , 240 },\
   {   59 , 235 },\
   {   64 , 230 },\
   {   70 , 225 },\
   {   76 , 220 },\
   {   83 , 215 },\
   {   90 , 210 },\
   {   99 , 205 },\
   {  108 , 200 },\
   {  118 , 195 },\
   {  129 , 190 },\
   {  141 , 185 },\
   {  154 , 180 },\
   {  169 , 175 },\
   {  185 , 170 },\
   {  203 , 165 },\
   {  222 , 160 },\
   {  243 , 155 },\
   {  266 , 150 },\
   {  290 , 145 },\
   {  317 , 140 },\
   {  346 , 135 },\
   {  376 , 130 },\
   {  408 , 125 },\
   {  442 , 120 },\
   {  477 , 115 },\
   {  513 , 110 },\
   {  551 , 105 },\
   {  588 , 100 },\
   {  626 ,  95 },\
   {  663 ,  90 },\
   {  699 ,  85 },\
   {  735 ,  80 },\
   {  768 ,  75 },\
   {  800 ,  70 },\
   {  829 ,  65 },\
   {  856 ,  60 },\
   {  881 ,  55 },\
   {  903 ,  50 },\
   {  922 ,  45 },\
   {  939 ,  40 },\
   {  954 ,  35 },\
   {  966 ,  30 },\
   {  977 ,  25 },\
   {  986 ,  20 },\
   {  994 ,  15 },\
   { 1000 ,  10 },\
   { 1005 ,   5 },\
   { 1009 ,   0 }\
};


//---------------------------------------------------4.7 KOhm Pullup----------------------------------------------------



// 100k EPCOS (WITH 1kohm RESISTOR FOR PULLUP, R9 ON SANGUINOLOLU! NOT FOR 4.7kohm PULLUP! THIS IS NOT NORMAL!)
// Verified by linagee.
// Calculated using 1kohm pullup, voltage divider math, and manufacturer provided temp/resistance
// Advantage: Twice the resolution and better linearity from 150C to 200C
#define EPCOS100_1K_SIZE 53
#define EPCOS100_1K_TABLE {\
  {1, 350},\
  {190, 250},\
  {203, 245},\
  {217, 240},\
  {232, 235},\
  {248, 230},\
  {265, 225},\
  {283, 220},\
  {302, 215},\
  {322, 210},\
  {344, 205},\
  {366, 200},\
  {390, 195},\
  {415, 190},\
  {440, 185},\
  {467, 180},\
  {494, 175},\
  {522, 170},\
  {551, 165},\
  {580, 160},\
  {609, 155},\
  {638, 150},\
  {666, 145},\
  {695, 140},\
  {722, 135},\
  {749, 130},\
  {775, 125},\
  {800, 120},\
  {823, 115},\
  {845, 110},\
  {865, 105},\
  {884, 100},\
  {901, 95},\
  {917, 90},\
  {932, 85},\
  {944, 80},\
  {956, 75},\
  {966, 70},\
  {975, 65},\
  {982, 60},\
  {989, 55},\
  {995, 50},\
  {1000, 45},\
  {1004, 40},\
  {1007, 35},\
  {1010, 30},\
  {1013, 25},\
  {1015, 20},\
  {1017, 15},\
  {1018, 10},\
  {1019, 5},\
  {1020, 0},\
  {1021, -5}\
};




// 200k ATC Semitec 204GT-2 (WITH 1kohm RESISTOR FOR PULLUP, R9 ON SANGUINOLOLU! NOT FOR 4.7kohm PULLUP! THIS IS NOT NORMAL!)
// Verified by linagee. Source: http://shop.arcol.hu/static/datasheets/thermistors.pdf
// Calculated using 1kohm pullup, voltage divider math, and manufacturer provided temp/resistance
// Advantage: More resolution and better linearity from 150C to 200C
#define SEMITEC_200_1K_SIZE 32
#define SEMITEC_200_1K_TABLE {\
  {1, 500},\
  {125, 300},\
  {142, 290},\
  {162, 280},\
  {185, 270},\
  {211, 260},\
  {240, 250},\
  {274, 240},\
  {312, 230},\
  {355, 220},\
  {401, 210},\
  {452, 200},\
  {506, 190},\
  {563, 180},\
  {620, 170},\
  {677, 160},\
  {732, 150},\
  {783, 140},\
  {830, 130},\
  {871, 120},\
  {906, 110},\
  {935, 100},\
  {958, 90},\
  {976, 80},\
  {990, 70},\
  {1000, 60},\
  {1008, 50},\
  {1013, 40},\
  {1017, 30},\
  {1019, 20},\
  {1021, 10},\
  {1022, 0}\
};




// 100k ATC Semitec 104GT-2 (Used on ParCan) (WITH 1kohm RESISTOR FOR PULLUP, R9 ON SANGUINOLOLU! NOT FOR 4.7kohm PULLUP! THIS IS NOT NORMAL!)
// Verified by linagee. Source: http://shop.arcol.hu/static/datasheets/thermistors.pdf
// Calculated using 1kohm pullup, voltage divider math, and manufacturer provided temp/resistance
// Advantage: More resolution and better linearity from 150C to 200C
#define SEMITEC_100_1K_SIZE 32

#define SEMITEC_100_1K_TABLE {\
  {1, 500},\
  {76, 300},\
  {87, 290},\
  {100, 280},\
  {114, 270},\
  {131, 260},\
  {152, 250},\
  {175, 240},\
  {202, 230},\
  {234, 220},\
  {271, 210},\
  {312, 200},\
  {359, 190},\
  {411, 180},\
  {467, 170},\
  {527, 160},\
  {590, 150},\
  {652, 140},\
  {713, 130},\
  {770, 120},\
  {822, 110},\
  {867, 100},\
  {905, 90},\
  {936, 80},\
  {961, 70},\
  {979, 60},\
  {993, 50},\
  {1003, 40},\
  {1010, 30},\
  {1015, 20},\
  {1018, 10},\
  {1020, 0}};


