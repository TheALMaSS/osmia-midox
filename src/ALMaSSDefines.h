/* 
*******************************************************************************************************
Copyright (c) 2011, Christopher John Topping, Aarhus University
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided
that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list of conditions and the
following disclaimer.
Redistributions in binary form must reproduce the above copyright notice, this list of conditions and 
the following disclaimer in the documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR 
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND 
FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR 
BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
********************************************************************************************************
*/

#ifndef __ALMaSSDEFINES
#define __ALMaSSDEFINES

#define _CRTDBG_MAP_ALLOC

//******************* GUI Defines ***************************
#define __MAPSIZE 950
#define __TITLEBMP "Title950.bmp" 
//#define __MAPSIZE 768
//#define __TITLEBMP "Title768.bmp" 
//#define __MAPSIZE 512
//#define __TITLEBMP "Title512.bmp" 
// ******************* End GUI Defines ***************************

// General defines
#define _cfgWarningOff


// Ecostack define
//#define ECOSTACK_BIOPESTICIDE
// Goose defines

#define __FOLLOW_FIELD


// Record farm events
#define __RECORDFARMEVENTS

/**
Prevents annoying compiler warnings
*/
#define __lgridsize 39

// Pesticide defines 

/** Rodenticide minimum ignore amount */
#define __RODENTICIDE_MAX_DIFFUSION 100 // The Maximum distance from bait we expect to detect poisoned mice

/** 
Added in Jan 08 to facilitate ecological theory simulations
*/
//#define __LAMBDA_RECORD  // Used to turn recording of the births and deaths on or off - this will cost time so only have it set if you want it!!!
//#define __CJTDebug_5

//#define __EXTSHRINKSHEIGHT // Switch to allow/prevent shrinkage of crop height when using extensive farms

// _____ SKYLARK DEFINES _____
//#define __SKPOM
//#define __KALOSCRAPES
//#define __KALONOSCRAPES

#define __SKTALLAVOIDENCE 81 // 71
//#define TEST_ISSUE_DEATH_WARRANT
//#define __HazelParry1
#define __TRAMLINES_ON
//#define __CJTDebug_4 // Bounds Errors
//#define __NoStriglingEffect
//#define __NoPigsOutEffect
//Comment out below to get the birds in their 'correct' positions (looks horrid)
//#define FOR_DEMONSTRATION
//#define __PESTICIDE_RA
//#define __PESTICIDE_RA_CHICK  // This is dependent on defining __PESTICIDE_RA
//#define __PESTICIDE_RA_ADULT  // This is dependent on defining __PESTICIDE_RA
// _____ END SKYLARK DEFINES _____

// _____ VOLE DEFINES _____
//#define __Mutation
//#define __USINGTRAPLINES
//#define __VoleStarvationDays
//#define __VOLEPESTICIDEON
#define __VOLERODENTICIDEON
#define __VOLE_SMALL_LANDSCAPE
//#define __VOLEOUTPUTTESTFILES
// _____ END VOLE DEFINES _____

// _____ BEETLE DEFINES _____
//#define __BEETLEPESTICIDE1
// _____ END BEETLE DEFINES _____

// _____ SPIDER DEFINES _____
//#define __SPIDER_PESTICIDE
// _____ END SPIDER DEFINES _____

// _____ HARE DEFINES _____
/* For doxygen commenting uncomment the lines below */
/**
If set then starvation of young stages is prevented - a debug and testing feature (see Hare_Young::st_Foraging & Hare_Female::DoLactation)
*/
// #define __NOSTARVE
/**
Dispersal increased mortality (see Hare_Juvenile::Step, Hare_Male::Step, & Hare_Female::Step)
*/
// #define __DISPERSALDDM
/**
Simulated disease based on density. If the density becomes too high the female may become sterile (see Hare_Juvenile::Step, Hare_Male::Step, & Hare_Female::Step)
*/
// #define __DISEASEDDM
/**
As DISEASEDDM but death resulted - this version works only on adult stages. (see Hare_Male::BeginStep &  Hare_Female::BeginStep)
*/
// #define __DISEASEDDMORTALITY  // Needs the one above defined too.
/**
Used last years mean experienced density to determine response to interference (see Hare_Male::BeginStep,  Hare_Female::BeginStep, Hare_Male::st_Foraging &  Hare_Female::st_Foraging)
*/
// #define __DISEASEDDM2 
/**
Used last years experienced density (running average) to determine response to density (see Hare_Male::BeginStep,  Hare_Female::BeginStep, Hare_Male::st_Foraging &  Hare_Female::st_Foraging)
*/
// #define __DISEASEDDM3 // Running average instead of annual DDEP
/**
This can be used to apply a global mortality rate to the population - used in testing (see THare_Population_Manager::DoFirst)
*/
// #define __EXTRAPOPMORT
/** 
If set - the default, then stochastic sterility is applied. Sterility is normally an emergent property only. (see THare_Population_Manager::DoFirst)
*/
// #define __STERILITY
/**
Adult mortality scaled inversely with size (see Hare_Male::BeginStep &  Hare_Female::BeginStep)
*/
// #define __SIZERELATEDDEATH
/**
A stochastic factor was applied to the density-dependent interference parameter  (see THare_Population_Manager::DoFirst)
*/
// #define __VARIABLEDD
/**
The mean quality of food varied on an annual basis (see THare_Population_Manager::DoFirst)
*/
// #define __YEARLYVARIABLEFOODQUALITY
/**
Based density-dependent effects on last years density on the same day (see THare_Population_Manager::DoFirst)
*/
// #define __DELAYEDDD
/**
No density-dependent effects below a threshold (see Hare_Infant::BeginStep,  Hare_Young::BeginStep, & Hare_Juvenile::BeginStep )
*/
// #define __THRESHOLD_DD
/**
Juveniles had no density dependent effects below a threshold  (see Hare_Juvenile::st_Foraging)
*/
// #define __THRESHOLD_DDJ
/**
The addition of stochasticity to background survival probabilities (see THare_Population_Manager::DoFirst)
*/
// #define __MORTSTOCHASTICITY
/**
Debug define, prevents juvenile starvation (see Hare_Juvenile::st_Developing)
*/
// #define __NOJUVSTARVE
/**
Females had increased mortality if reproducing (see Hare_Female::st_ReproBehaviour)
*/
// #define __REPROMORTCHANCE
/** 
If set this prevents reproduction by adults that have a weight of less than a fixed value. The value is an imput parameter passed in Hare_All.cpp::cfg_hare_minimum_breeding_weight (see Hare_Female::UpdateOestrous)
*/
// #define __MINREPWEIGHT
/**
The strength of density dependence is higher on the young, scaled by a scalar relative to adult density-dependence effects (see Hare_Juvenile::st_Foraging)
*/
// #define __SCALINGYOUNGDDM
/**
If used then there is a increased chance of predation during dispersal
*/
// #define __DISPMORTALITY
/**
Density dependent mortality on young stages (see Hare_Infant::BeginStep, Hare_Young::BeginStep, Hare_Juvenile::Foraging, Hare_Male::Foraging, & Hare_Female::Foraging)
*/
// #define __DDEPMORT
/**
If set this prevents carrying a negative energy deficity once positive energy conditions are found - used for testing (see Hare_Female::st_Developing)
*/
// #define __NOKNOCKONENERGYEFFECT
/* Doxygen to here */

/* ******   THESE DEFINES ARE IN USE - SWITCH ON FOR COMPILING AND SWITCH OFF DOXYGEN ABOVE!!! ***** */
/** 
If set this prevents reproduction by adults that have a weight of less than a fixed value. The value is an imput parameter passed in Hare_All.cpp::cfg_hare_minimum_breeding_weight
*/
#define __MINREPWEIGHT
/** 
If set this causes the hare to die if it does not acheive a set proportion ofits optimal weight by this age. This proportion becomes an input variable passed in Hare_All.cpp::cfg_min_growth_attain
*/
#define __MINGROWTHATTAIN
/**
A probability of starving to death if the hare lost body weight (see  Hare_Male::st_Developing & Hare_Female::st_Developing)
*/
#define __ADULT_WT_STARVE_CHANCE
//#define __SIZERELATEDDEATH2
//#define __LOWFATRELATEDDEATH
//#define __Perfectfood  // Use to prevent any variations in vegetation quality below perfection
//#define __Hare1950s  // Use to remove access problems in crops
/**
Define to generate output related to litter sizes and numbers (see Hare_Female::GiveBirth)
*/
#define __saveLitterInfo
//#define __saveEnergyInfo
/**	Binary power to divide by to get density grids	*/
#define __DENSITYSHIFT 9
/**	Debug only	*/
//#define __BOUNDSCHECK
// _____ END HARE DEFINES _____

// _____ PARTRIDGE DEFINES _____
  #define MOVE_QUAL_HIST_SIZE 5
//#define __PAR_DEBUG
#define __RAIN_HURTS
//#define __NESTPOSITIONS_OUT
//#define __PAR_DEBUG2
//#define __PARDEBUG4
//#define __PAR_PCDEBUG
//#define MOVE_EVAL_KLUDGE
/**  \brief For 1950s simulations  */
//#define __P1950s // Simulates 1950s with all access granted
#define MIN_MATH_DIST_SQ 0.001
//#define max(a,b) ((a)>(b)?(a):(b))  // means if a>b then a otherwise b
#define DEG2RAD (3.14159265/180)
#define TWO_PI 6.28318530718
#define PAR_NEST
// _____ END PARTRIDGE DEFINES _____

// _____ START OLIVE MOTH DEFINES _____
//#define __OLIVEMOTH
//#define __OLIVEMOTH_INPUT
//#define __OLIVEMOTH_DEBUG
//#define __OLIVEMOTH_OUTPUT
// _____ END OLIVE MOTH DEFINES _____

// _____ START GOOSE DEFINES _____
//#define __LINKGOOSESCARETODISTANCE
// _____ END GOOSE DEFINES _____

#define __POLLINERA_TKTD
//#define __POLLINERA_TKTD_DEBUG

// ____ START ROE DEER DEFINES ____
/** \brief Turns on or off debug code in roe deer */
#define __ROEDEER_DEBUG
// ____  END ROE DEER DEFINES  ____


// _____ START RABBIT DEFINES _____
//#define  __RABBITBREEDINGSEASONCHECK
//#define __RABBITDEBUG2
//#define __RABBITMEANPESTICIDECONC
// _____ END RABBIT DEFINES _____

// _____ START NEWT DEFINES _____
#define __RECORDNEWTMETAMORPHOSIS
// _____ END NEWT DEFINES _____

// _____ START OSMIA DEFINES _____
//#define __RECORDOSMIAEGGPRODUCTION //for debugging purposes
//#define __CJT_OSMIAFIXEDRESOURCE //for debugging purposes
//#define __OSMIARECORDFORAGE
//#define __OSMIATESTING //for calibration purposes, this will record 
// _____ END OSMIA DEFINES _____

// _____ START BOMBUS DEFINES _____
//#define __RECORDBOMBUSLARVAE
//#define __RECORDBOMBUSCALIBRATION
//#define __BOMBUS_PESTICIDE
// _____ END BOMBUS DEFINES _____

// ____ START FLOWER TEST DEFEINES____
//#define __FLOWERTESTING
// ____ END FLOWER TEST DEFEINES____

// ____ START APHID TEST DEFEINES____
//#define APHID_DEBUG
//#define APHID_BIOMASS_DEBUG
//#define __APHID_CALIBRATION
// ____ END APHID TEST DEFEINES____

// ____ START APISRAM TEST DEFEINES____
//#define __APISRAM_FORAGE_DEBUG
//#define __APISRAM_SCOUT_DEBUG
// ____ END APISRAM TEST DEFEINES____

////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////    GLOBAL  ENUMS    ////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
// These enums are used by a number of code files, and collecting them here makes it easer to
// get an overview of them.

/**
\brief The enumeration lists all possible goose species - of which currently there are three.
*/
enum GooseSpecies
{
	gs_Pinkfoot = 0,
	gs_Barnacle,
	gs_Greylag,
	gs_foobar
};

/** used to define 8 directions - this is the x dimension */
constexpr int g_vector_x[8] = { 0, 1, 1, 1, 0,-1,-1,-1 };
/** used to define 8 directions - this is the y dimension */
constexpr int g_vector_y[8] = { -1,-1, 0, 1, 1, 1, 0,-1 };
/** used to define 16 directions - this is the x dimension */
constexpr int g_vector16_x[16] = { 0,1,2,2,2,2,2,1,0,-1,-2,-2,-2,-2,-2, -1 };
/** used to define 16 directions - this is the y dimension */
constexpr int g_vector16_y[16] = { -2,-2,-2, -1, 0,1,2,2,2,2,2,1,0,-1,-2,-2 };


#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288
#endif

/* Landscape and farm defines */
//#define __CALCULATE_MANAGEMENT_NORMS

#endif
