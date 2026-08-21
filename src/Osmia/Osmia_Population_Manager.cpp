/*
*******************************************************************************************************
Copyright (c) 2017, Christopher John Topping, Aarhus University
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
/** \file Osmia_Population_Manager.cpp
Version of  September 2019 \n
By Chris J. Topping \n \n
*/

//---------------------------------------------------------------------------

#include <string.h>
#include <iostream>
#include <fstream>
#include<vector>
#include <chrono>
#pragma warning( push )
#pragma warning( disable : 4100)
#pragma warning( disable : 4127)
#pragma warning( disable : 4244)
#pragma warning( disable : 4267)
#pragma warning( pop ) 
#include "../BatchALMaSS/ALMaSS_Setup.h"
#include "../ALMaSSDefines.h"
#include "../Landscape/ls.h"
#include "../BatchALMaSS/PopulationManager.h"
#include "../BatchALMaSS/AOR_Probe.h"
#include "../Osmia/Osmia.h"
#include "../Osmia/Osmia_Population_Manager.h"

//---------------------------------------------------------------------------------------
/** \brief Monthly pollen thresholds, 12 quantity then 12 quality. Units mg/m2  and unitless */
static CfgArray_Double cfg_OsmiaPollenThresholds("OSMIA_POLLEN_THRESHOLDS", CFG_CUSTOM, 24, vector<double> {
	1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
		1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0
});

/** \brief Monthly nectar thresholds, 12 quantity then 12 quality. Units mj/m2  and mg/l */
static CfgArray_Double cfg_OsmiaNectarThresholds("OSMIA_NECTAR_THRESHOLDS", CFG_CUSTOM, 24, vector<double> {
	1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
		1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0
});


/** \brief Montly mortalities for TTypeOfOsmiaParasitoids parasitoids. Each set of 12 are the mortalities matching the order in the enum class TTypeOfOsmiaParasitoids */
static CfgArray_Double cfg_OsmiaParasDailyMort("OSMIA_PARAS_DAILYMORT", CFG_CUSTOM, 24, vector<double> {
	1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
		1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0
});
/** \brief Coefficients of the prepupal development rate function.
*
* The Formal Model (Ziolkowska et al. 2023, FMJ 4: e102102, p.12) specifies prepupal duration
* as a quadratic in temperature with an optimum at 22 C, at which the maximal developmental
* speed of @c cfg_OsmiaPrepupaDevelTotalDays is reached:
*
*     duration(T) = OSMIA_PREPUPADEVELDAYS * q(T) / q(Topt)
*     q(T)        = a*T^2 + b*T + c
*
* so the daily development rate accumulated by Osmia_Prepupa::st_Develop is
*
*     rate(T)     = q(Topt) / q(T)
*
* These coefficients replace the former OSMIA_PREPUPALDEVELRATES lookup table, which was this
* same function reciprocated, normalised at 22 C and sampled at integer degrees. Fitting a
* quadratic to the reciprocal of that table recovers these coefficients with a maximum
* absolute residual of 1.9e-8, and reconstructing the table as q(22)/q(T) reproduces every
* published entry to within 9e-10.
*
* Only the ratios a:b:c are identified; the function is normalised at Topt.
*
* The discriminant b^2-4ac = -0.0597 < 0 with a > 0, so q(T) > 0 for every real T (minimum
* 0.9982 at T = 22.35 C). The reciprocal is therefore always finite and no clamping of the
* temperature is required. Osmia_Population_Manager::Init() checks both conditions, since the
* coefficients can be overridden from a configuration file. */
static CfgFloat cfg_OsmiaPrepupalRateA("OSMIA_PREPUPALRATE_A", CFG_CUSTOM,  0.0149431912);
/** @brief Linear coefficient B in the dimensionless prepupal rate quadratic q(T). */
static CfgFloat cfg_OsmiaPrepupalRateB("OSMIA_PREPUPALRATE_B", CFG_CUSTOM, -0.6679153638);
/** @brief Constant coefficient C in the dimensionless prepupal rate quadratic q(T). */
static CfgFloat cfg_OsmiaPrepupalRateC("OSMIA_PREPUPALRATE_C", CFG_CUSTOM,  8.4616334666);
/** \brief Temperature at which the prepupal rate function is normalised to 1.0, i.e. the stated
* thermal optimum. Note that the analytic vertex of the parabola is at 22.35 C; the Formal Model
* quotes the rounded 22 C, and the published rate table was normalised there. */
static CfgFloat cfg_OsmiaPrepupalRateTOpt("OSMIA_PREPUPALRATE_TOPT", CFG_CUSTOM, 22.0);
/** \brief The starting number of Osmia InCocoons */
static CfgInt cfg_OsmiaStartNo("OSMIA_STARTNOS", CFG_CUSTOM, 50000);
/** \brief Dispersal rates for TTypeOfOsmiaParasitoids parasitoids. Each entry is the dispersal rate for the parasitoid type matching the order in the enum class TTypeOfOsmiaParasitoids */
static CfgArray_Double cfg_OsmiaParasDispersal("OSMIA_PARAS_DISPERSAL", CFG_CUSTOM, static_cast<unsigned>(TTypeOfOsmiaParasitoids::topara_foobar) - 1, vector<double> { 0.001, 0.0001 });
/** \brief Starting numbers for TTypeOfOsmiaParasitoids parasitoids. Each entry is the max and min starting numbers per sub-population for the parasitoid type matching the order in the enum class TTypeOfOsmiaParasitoids */
static CfgArray_Double cfg_OsmiaParasStartHighLow("OSMIA_PARAS_STARTHIGHLOW", CFG_CUSTOM, 2*(static_cast<unsigned>(TTypeOfOsmiaParasitoids::topara_foobar) - 1), vector<double> { 2.0, 1.0, 2.0, 1.0});
/** \brief The size class step for Osmia female mass */
CfgFloat cfg_OsmiaAdultMassCategoryStep("OSMIA_ADULTMASSCLASSSTEP", CFG_CUSTOM, 10.0);
/** \brief Input file for Osmia nest density per LE */
static CfgStr cfg_OsmiaNestByLE_Datafile("OSMIA_NESTBYLEDATAFILE", CFG_CUSTOM, "OsmiaNestsByHabitat.txt");
/** \brief The daily background mortality rate for females */
static CfgFloat cfg_OsmiaFemaleBckMort("OSMIA_FEMALEBACKMORT", CFG_CUSTOM, 0.02);
/** \brief The mimimum number of eggs planned for a nest */
static CfgInt cfg_OsmiaMinNoEggsInNest("OSMIA_MINNOEGGSINNEST", CFG_CUSTOM, 3);
/** \brief The maximum number of eggs planned for a nest  */
static CfgInt cfg_OsmiaMaxNoEggsInNest("OSMIA_MAXNOEGGSINNEST", CFG_CUSTOM, 30);
/** \brief Array of parameters for the Osmia sex ratio vs mothers age logistic equation  */
static CfgArray_Double cfg_OsmiaSexRatioVsMotherAgeLogistic("OSMIA_SEXRATIOVSMOTHERSAGELOGISTIC", CFG_CUSTOM, 4, vector<double> { 14.90257909, 0.09141286, 0.6031729, -0.39213001 });
/** \brief Array of parameters for the Osmia female cocoon mass vs mothers age logistic equation  */
static CfgArray_Double Cfg_OsmiaFemaleCocoonMassVsMotherAgeLogistic("OSMIA_FEMALECOCOONMASSVSMOTHERSAGELOGISTIC", CFG_CUSTOM, 4, vector<double> { 18.04087868, 104.19820591, 133.74150303, -0.17686981});
/** \brief Array of parameters for the Osmia sex ratio vs mothers mass linear equation  */
static CfgArray_Double cfg_OsmiaSexRatioVsMotherMassLinear("OSMIA_SEXRATIOVSMOTHERSMASSLINEAR", CFG_CUSTOM, 2, vector<double> { 0.0055, -0.1025 });
/** \brief Array of parameters for the Osmia female first cocoon mass vs mothers mass linear equation  */
static CfgArray_Double Cfg_OsmiaFemaleCocoonMassVsMotherMassLinear("OSMIA_FEMALECOCOONMASSVSMOTHERSMASSLINEAR", CFG_CUSTOM, 2, vector<double> { 0.3, 65.1 });//{ 0.46, 63.85 });
/** \brief Assumed (fitted) value for the total difference in cocoon mass from first to last cocoon  */
static CfgFloat cfg_Osmia_LifetimeCocoonMassLoss("OSMIA_LIFETIMECOCOONMASSLOSS", CFG_CUSTOM, 30.0);
/** \brief The relationhip between cocoon massand provisioning mass is linear */
CfgFloat cfg_OsmiaCocoonMassFromProvMass("OSMIAS_COCOONTOPROVISIONING", CFG_CUSTOM, 1.0 / 3.247);
/** \brief The relationhip between cocoon mass and provisioning mass is linear */
CfgFloat cfg_OsmiaProvMassFromCocoonMass("OSMIAS_PROVISIONINGTOCOCOON", CFG_CUSTOM, 3.247);
/** \brief The minimum amount of pollen needed to provision a male cell */
static CfgFloat  cfg_MaleMinTargetProvisionMass("OSMIA_MALEMINTARGETPROVISIONMASS", CFG_CUSTOM, 10.0);
/** \brief The minimum time to construct a cell in days */
static CfgInt cfg_MinimumCellConstructionTime("OSMIA_MINCELLCONSTRUCTTIME", CFG_CUSTOM, 1);
/** \brief The maximum time allowed to construct a cell in days */
static CfgInt cfg_MaximumCellConstructionTime("OSMIA_MAXCELLCONSTRUCTTIME", CFG_CUSTOM, 4);
/** \brief The maximum number of nests possible for a bee */
static CfgInt cfg_TotalNestsPossible("OSMIA_TOTALNESTSPOSSIBLE",CFG_CUSTOM, 4); // Formal Model: 5
/** \brief The flag for using mechanistic (true) or statistical (false) parasitoid assumptions */
static CfgBool cfg_UsingMechanisticParasitoids("OSMIA_USEMECHANISTICPARASITOIDS", CFG_CUSTOM, false);
/** \brief The probability that a parasitoid is Bombylid - statistical parasitoids only */
static CfgFloat cfg_OsmiaBombylidProb("OSMIA_BOMBYLIDPROB", CFG_CUSTOM, 0.5);
/** \brief The conversion rate from timethe cell is open to open cell parasitism */
static CfgFloat cfg_OsmiaParasitismProbToTimeCellOpen("OSMIA_PARASITISMPROBTOTIMECELLOPEN", CFG_CUSTOM, 0.0075);
/** \brief Array of parameters for per capita parasitoid probability for each type of parasitoid in the order based on TTypeOfOsmiaParasitoids */
static CfgArray_Double cfg_OsmiaPerCapitaParasationChance("OSMIA_PERCAPITAPARASITATIONCHANCE", CFG_CUSTOM, static_cast<int>(TTypeOfOsmiaParasitoids::topara_foobar) - 1, vector<double> { 0.00001, 0.00002 });
/** \brief Parameter to set the numbe of nest finding attempts */
static CfgInt cfg_OsmiaFemaleFindNestAttemptNo("OSMIA_FEMALEFINDNESTATTEMPTNO", CFG_CUSTOM, 20);
/** \brief The change in proportion pollen before a new patch is selected */
static CfgFloat cfg_OsmiaPollenGiveUpThreshold("OSMIA_POLLENGIVEUPTHRESHOLD", CFG_CUSTOM, 0.75, 0, 1.0);
/** \brief The change in proportion pollen before a new patch is selected */
static CfgFloat cfg_OsmiaPollenGiveUpReturn("OSMIA_POLLENGIVEUPRETURN", CFG_CUSTOM, 0.75, 0, 50.0);
/** \brief Parameter to set the proportion of pollen removed by other bees */
static CfgFloat cfg_OsmiaDensityDependentPollenRemovalConst("OSMIADENSITYDENPENDENTPOLLENREMOVALCONST", CFG_CUSTOM, 0.5); // EZ: no competition assumed as default
/** \brief The conversion rate from pollen availability score to mg pollen provisioned per day per bee  - a fitting parameter*/
static CfgFloat  cfg_PollenScoreToMg("OSMIA_POLLENSCORETOMG", CFG_CUSTOM,  0.8);
/** \brief A distribution to create an exponential decay from zero to one */
probability_distribution Osmia_Population_Manager::m_exp_ZeroTo1 = probability_distribution("BETA", "0.75,2.5");
/** \brief The probability of death per day if the pesticide body burden threshold is exceeded */
static CfgFloat cfg_OsmiaPesticideProbability("OSMIA_PPP_PROB", CFG_CUSTOM, 0.0);
/** \brief The pesticide body burden threshold for ppp effects. */
static CfgFloat cfg_OsmiaPesticideThreshold("OSMIA_PPP_THRESHOLD", CFG_CUSTOM, 10000.0);
/** \brief The probability of death per time if the pesticide body burden threshold is exceeded for a egg*/
static CfgFloat cfg_OsmiaEggPesticideProbability("OSMIA_PPP_EGG_PROB", CFG_CUSTOM, 0.0);
/** \brief The pesticide threshold causing egg death. */
static CfgFloat cfg_OsmiaEggPesticideThreshold("OSMIA_PPP_EGG_THRESHOLD", CFG_CUSTOM, 10000.0);
/** \brief The pesticide killing rate.*/
static CfgFloat cfg_OsmiaPesticideKillRate("OSMIA_PPP_KILL_RATE", CFG_CUSTOM, 0.0);
/** \brief The pesticide recovery rate. */
static CfgFloat cfg_OsmiaPesticideRecoveryRate("OSMIA_PPP_RECOVERY_RATE", CFG_CUSTOM, 0.0);
/** \brief The decay rate of the pesticide within the osmia body. */
static CfgFloat cfg_OsmiaPesticideDecayRate("OSMIA_PPP_DECAY_RATE", CFG_CUSTOM, 0.0);
/** \brief The overspray chance for osmia when it is foraging in a field being sprayed. */
static CfgFloat cfg_OsmiaPesticideOversprayChance("OSMIA_PPP_OVERSPRAY_CHANCE", CFG_CUSTOM, 0.5);
/** \brief The absorption rate of pesticide on the body surface to the body. -- contact*/
static CfgFloat cfg_OsmiaPesticideAbsorptionRateContact("OSMIA_PPP_ABSORPTION_RATE_Contact", CFG_CUSTOM, 0.1);
/** \brief The absorption rate of pesticide on the body surface to the body. -- overspray*/
static CfgFloat cfg_OsmiaPesticideAbsorptionRateOverspray("OSMIA_PPP_ABSORPTION_RATE_Overspray", CFG_CUSTOM, 0.1);
/** \brief The osmia body surface used to overspray. */
static CfgFloat cfg_OsmiaPesticideOversprayBodySurface("OSMIA_PPP_OVERSPRAY_BODY_SURFACE", CFG_CUSTOM, 0.0);
/** \brief The osmia body surface used to contact exposure path. */
static CfgFloat cfg_OsmiaPesticideContactBodySurface("OSMIA_PPP_CONTACT_BODY_SURFACE", CFG_CUSTOM, 0.0);
/** \brief The minimum temperature in degree for osmia to fly out*/
static CfgFloat cfg_OsmiaMinTempForFlying("OSMIA_MIN_TEMP_FOR_FLYING", CFG_CUSTOM, 6);
/** \brief The maximum wind speed in meter per second for osmia to fly out*/
static CfgFloat cfg_OsmiaMaxWindSpeedForFlying("OSMIA_MAX_WIND_SPEED_FOR_FLYING", CFG_CUSTOM, 8);
/** \brief The maximum precipitation for osmia to fly out*/
static CfgFloat cfg_OsmiaMaxPrecipForFlying("OSMIA_MAX_PRECIP_FOR_FLYING", CFG_CUSTOM, 0.1);
/** \brief The accumulated overwinter degree days for cocoon when initialising the simulations.*/
static CfgFloat cfg_OsmiaOverwinterDegreeDaysInitialSimu("OSMIA_OVERWINTER_DEGREE_DAYS_INITIAL_SIMU", CFG_CUSTOM, 320);
/** \brief The flag to enable storing population dynamics with new-born and death. */
CfgBool cfg_OsmiaStorePopulationDynamics("OSMIA_STORE_POPULATION_DYNAMICS", CFG_CUSTOM, false);
/** \brief The file name for storing population dynamics with new-born and death. */
CfgStr cfg_OsmiaPopulationDynamicsFile("OSMIA_POPULATION_DYNAMICS_FILE", CFG_CUSTOM, "OsmiaPopulationDynamics.txt");

extern CfgFloat cfg_OsmiaInCocoonOverwinteringTempThreshold;
extern CfgFloat cfg_OsmiaInCocoonEmergenceTempThreshold;
extern CfgFloat cfg_OsmiaFemaleMassMin;
extern CfgFloat cfg_OsmiaFemaleMassMax;
extern CfgFloat cfg_OsmiaFemaleMassFromProvMassConst;
extern CfgFloat cfg_OsmiaFemaleMassFromProvMassSlope;
extern CfgInt	cfg_OsmiaTypicalHomingDistance;
extern CfgInt cfg_OsmiaMaxHomingDistance;
extern Landscape* g_landscape_ptr;

// Assign default static member values (these will be changed later).
double Osmia_Base::m_DailyDevelopmentMortEggs = 0;
double Osmia_Base::m_DailyDevelopmentMortLarvae = 0;
double Osmia_Base::m_DailyDevelopmentMortPrepupae = 0;
double Osmia_Base::m_DailyDevelopmentMortPupae = 0;
double Osmia_Base::m_OsmiaEggDevelTotalDD = 0;
double Osmia_Base::m_OsmiaEggDevelThreshold = 0;
double Osmia_Base::m_OsmiaLarvaDevelTotalDD = 0;
double Osmia_Base::m_OsmiaLarvaDevelThreshold = 0;
double Osmia_Base::m_OsmiaPupaDevelTotalDD = 0;
double Osmia_Base::m_OsmiaPupaDevelThreshold = 0;
double Osmia_Base::m_OsmiaPrepupalDevelTotalDays = 0;
double Osmia_Base::m_OsmiaPrepupalDevelTotalDays10pct = 0;
double Osmia_Base::m_OsmiaInCocoonOverwinteringTempThreshold = 0;
double Osmia_Base::m_OsmiaInCocoonEmergenceTempThreshold = 0;
double Osmia_Base::m_OsmiaInCocoonPrewinteringTempThreshold = 0;
double Osmia_Base::m_OsmiaInCocoonWinterMortConst = 0.0;
double Osmia_Base::m_OsmiaInCocoonWinterMortSlope = 0.0;
double Osmia_Base::m_OsmiaInCocoonEmergCountConst = 0.0;
double Osmia_Base::m_OsmiaInCocoonEmergCountSlope = 0.0;
double Osmia_Base::m_OsmiaFemaleMassFromProvMassConst = 0.0;
double Osmia_Base::m_OsmiaFemaleMassFromProvMassSlope = 0.0;
double Osmia_Base::m_TempToday = -9999;
int Osmia_Base::m_TempTodayInt = -9999;
OsmiaParasitoid_Population_Manager* Osmia_Base::m_OurParasitoidPopulationManager = NULL;
double Osmia_InCocoon::m_OverwinteringTempThreshold = 0.0;
double Osmia_Base::m_OsmiaFemaleBckMort = 0.0;
int Osmia_Base::m_OsmiaFindNestAttemptNo = 0;
int Osmia_Base::m_OsmiaFemaleMinEggsPerNest = 0;
int Osmia_Base::m_OsmiaFemaleMaxEggsPerNest = 0;
double  Osmia_Base::m_CocoonToProvisionMass = 0.0;
double  Osmia_Base::m_ProvisionToCocoonMass = 0.0;
double  Osmia_Base::m_TotalProvisioningMassLoss = 0.0;
double  Osmia_Base::m_TotalProvisioningMassLossRange = 0.0;
double  Osmia_Base::m_TotalProvisioningMassLossRangeX2 = 0.0;
bool 	Osmia_Base::m_UsingMechanisticParasitoids = false;
double Osmia_Base::m_PollenScoreToMg = 0.0;
double Osmia_Base::m_DensityDependentPollenRemovalConst = 0.0;
double Osmia_Base::m_MaleMinTargetProvisionMass = 0.0;
double Osmia_Base::m_MaleMaxTargetProvisionMass = 0.0;
double Osmia_Base::m_FemaleMinTargetProvisionMass = 0.0;
double Osmia_Base::m_FemaleMaxTargetProvisionMass = 0.0;
double Osmia_Base::m_MaleMaxMass = 0.0;
double Osmia_Base::m_FemaleMinMass = 0.0;
double Osmia_Base::m_FemaleMaxMass = 0.0;
double Osmia_Base::m_MinimumCellConstructionTime = 0.0;
double Osmia_Base::m_MaximumCellConstructionTime = 0.0;
int Osmia_Base::m_TotalNestsPossible = 0;
double Osmia_Base::m_BombylidProbability = 0.0;
double Osmia_Base::m_ParasitismProbToTimeCellOpen = 0.0;
double Osmia_Base::m_OsmiaFemaleR50distance = 0.0;
double Osmia_Base::m_OsmiaFemaleR90distance = 0.0;
int Osmia_Base::m_OsmiaFemaleLifespan = 0;
int Osmia_Base::m_OsmiaFemalePrenesting = 0;
vector<double>  Osmia_Base::m_ParasitoidAttackChance = {};
Osmia_Nest_Manager* Osmia_Nest::m_OurManager = NULL;
array<double,12> OsmiaParasitoidSubPopulation::m_MortalityPerMonth = { 0,0,0,0,0,0,0,0,0,0,0,0 };
int OsmiaParasitoidSubPopulation::m_ThisMonth = -1;
vector<double> Osmia_Female::m_FemaleForageEfficiency = {};
double Osmia_Female::m_pollengiveupthreshold = 0.0;
double Osmia_Female::m_pollengiveupreturn = 0.0;
double Osmia_Female::m_OsmiaMaxPollen = 0.0;
double Osmia_Female::m_OsmiaSugarPerDay = 0.0;
double Osmia_Female::m_PollenCompetitionsReductionScaler = cfg_OsmiaDensityDependentPollenRemovalConst.value();
extern int g_thread_count;
#ifdef __OSMIARECORDFORAGE
double Osmia_Female::m_foragesum = 0.0;
int Osmia_Female::m_foragecount = 0;
#endif

double Osmia_Female::m_OsmiaPPPEffectProb = 0.0;
double Osmia_Female::m_OsmiaPPPThreshold = 0.0;
double Osmia_Female::m_OsmiaPPPDecayRate = 0.0;
double Osmia_Female::m_OsmiaPPPAbsorptionRateContact = 0.0;
double Osmia_Female::m_OsmiaPPPAbsorptionRateOverspray = 0.0;
double Osmia_Female::m_OsmiaPPPOversprayBodySurface = 0.0;
double Osmia_Female::m_OsmiaPPPContactBodySurface = 0.0;
double Osmia_Female::m_OsmiaPPPOversprayChance = 0.0;
double Osmia_Female::m_OsmiaEggPPPEffectProb = 0.0;
double Osmia_Female::m_OsmiaEggPPPThreshold = 0.0;

#ifdef __OSMIA_PESTICIDE_STORE
#endif

//---------------------------------------------------------------------------

Osmia_Population_Manager::~Osmia_Population_Manager (void)
{
//	delete m_PollenNectarLists;
#ifdef __OSMIATESTING
	delete m_female_weight_record_lock;
	m_eggsfirstnest.close();
	ofstream ofile("EggsDistributions.txt", ios::out);
	for (int i = 0; i < 30; i++)
	{
		ofile << m_egghistogram[0][i] << '\t' << m_egghistogram[1][i] << '\t' << m_egghistogram[2][i] << '\t' << m_egghistogram[3][i] << endl;
	}
	ofile.close();
#endif // __OSMIATESTING
	if (cfg_OsmiaStorePopulationDynamics.value())
	{
		m_population_dynamics_file->close();
		delete m_population_dynamics_file;
	}
}
//---------------------------------------------------------------------------

Osmia_Population_Manager::Osmia_Population_Manager(Landscape* L) : Population_Manager(L, 6)
{
	/** Loads the list of Animal Classes. */
	m_ListNames[0] = "Egg";		// from egg laying to the beginning of feeding
	m_ListNames[1] = "Larva";	// larval development + cocoon spinning
	m_ListNames[2] = "Prepupa";	// the summer diapause period
	m_ListNames[3] = "Pupa";
	m_ListNames[4] = "In Cocoon";	// fully developed adults remaining within cocoons (includes overwintering period)
	m_ListNames[5] = "Female";
	m_ListNameLength = 6;
	m_SimulationName = "Osmia";
	// Init performs intialisation of the data to run the Osmia model
	Init();
	// Set up neccessary conditions for mid-lifecycle start
	m_PreWinteringEndFlag = true;
	m_OverWinterEndFlag = false;


	// Get all the polygon that is suitable for osmia nesting
	std::vector<int> suitable_polygons;
	m_OurOsmiaNestManager.UpdateOsmiaNesting();
	int num_poly = m_TheLandscape->SupplyNumberOfPolygons();
	for (int i = 0; i < num_poly; i++)
	{
		if (IsOsmiaNestPossible(i))
		{
			suitable_polygons.push_back(i);
		}
	}
	
	int num_poly_for_nesting = suitable_polygons.size();
	if(cfg_OsmiaStorePopulationDynamics.value())
	{
		//open the file for storing population dynamics
		m_population_dynamics_file = new std::ofstream(cfg_OsmiaPopulationDynamicsFile.value(), ios::out);
		*m_population_dynamics_file << "Year" << '\t' << "Day" << '\t' << "EggNewborn" << '\t' << "LarvaeNewborn " << '\t' << "PrepupaeNewborn" << '\t' << "PupaeNewborn" << '\t' << "InCocoonNewborn" << '\t' <<  "FemaleNewborn" << '\t' << "EggDeath" << '\t' << "LarvaeDeath " << '\t' << "PrepupaeDeath" << '\t' << "PupaeDeath" << '\t' << "InCocoonDeath" << '\t' <<  "FemaleDeath" << '\t' << "EggDeathPesticide" << '\t' << "LarvaeDeathPesticide " << '\t' << "PrepupaeDeathPesticide" << '\t' << "PupaeDeathPesticide" << '\t' << "InCocoonDeathPesticide" << '\t' <<  "FemaleDeathPesticide" << std::endl;
		//initia the population dynamics vectors
		m_NewBornPopulation.resize(m_ListNameLength);
		m_DeadPopulation.resize(m_ListNameLength);
		m_DeadPopulationPesticide.resize(m_ListNameLength);
		for(int i = 0; i < m_ListNameLength; i++)
		{
			m_NewBornPopulation[i].resize(g_thread_count);
			std::fill(m_NewBornPopulation[i].begin(), m_NewBornPopulation[i].end(), 0);
			m_DeadPopulation[i].resize(g_thread_count);
			std::fill(m_DeadPopulation[i].begin(), m_DeadPopulation[i].end(), 0);
			m_DeadPopulationPesticide[i].resize(g_thread_count);
			std::fill(m_DeadPopulationPesticide[i].begin(), m_DeadPopulationPesticide[i].end(), 0);
		}
	}

	// Create some animals
    int start_num_in_thread = (cfg_OsmiaStartNo.value() / g_thread_count);
	int* start_num_arary = new int[g_thread_count];
	int temp_sum = 0;
	for (int i = 0; i < g_thread_count; i++)
	{
		start_num_arary[i] = start_num_in_thread;
		temp_sum += start_num_in_thread;
	}

	start_num_arary[0] += (cfg_OsmiaStartNo.value() - temp_sum);

	#pragma omp parallel
	{
		int thread_num = omp_get_thread_num();
		for (int i = 0; i < start_num_arary[thread_num]; i++) // This will need to be an input variable (config)
		{
			struct_Osmia* sp;
			sp = new struct_Osmia;
			sp->OPM = this;
			sp->L = m_TheLandscape;
			double minmass = (cfg_OsmiaFemaleMassMin.value() - cfg_OsmiaFemaleMassFromProvMassConst.value())
                 / cfg_OsmiaFemaleMassFromProvMassSlope.value();
			double maxmass = (cfg_OsmiaFemaleMassMax.value() - cfg_OsmiaFemaleMassFromProvMassConst.value())
                 / cfg_OsmiaFemaleMassFromProvMassSlope.value();
			sp->mass = minmass + (maxmass - minmass) * g_rand_uni_fnc();
			sp->parasitised = TTypeOfOsmiaParasitoids::topara_Unparasitised;
			sp->sex = true;

			//first find a suitable polygon randomly
			int pindex = suitable_polygons[g_random_fnc(num_poly_for_nesting)];
			APoint temp_point = m_TheLandscape->SupplyARandomLocPoly(pindex);
			sp->x = temp_point.m_x;
			sp->y = temp_point.m_y;
			sp->nest = CreateNest(sp->x, sp->y, pindex);
			sp->overwintering_degree_days = cfg_OsmiaOverwinterDegreeDaysInitialSimu.value();
			//sp->nest->AddEgg(NULL);
			CreateObjects(TTypeOfOsmiaLifeStages::to_OsmiaInCocoon, NULL, sp, 1);
			delete sp;
		}
	}
	for (unsigned co = 0; co<unsigned(SupplyListSize(int(TTypeOfOsmiaLifeStages::to_OsmiaInCocoon))); co++)
	{
		dynamic_cast<Osmia_InCocoon*>(SupplyAnimalPtr(int(TTypeOfOsmiaLifeStages::to_OsmiaInCocoon),co))->SetAgeDegrees(2000);
	}
	// Store the competition scaler for faster access
	m_PollenCompetitionsReductionScaler = cfg_OsmiaDensityDependentPollenRemovalConst.value();
	/** Cache the prepupal rate quadratic and precompute its normalising constant q(Topt),
	* which is fixed for the whole run. The coefficients are validated here rather than at the
	* point of use, so that the daily evaluation in DoFirst() needs no guard. */
	m_PrePupalRateA = cfg_OsmiaPrepupalRateA.value();
	m_PrePupalRateB = cfg_OsmiaPrepupalRateB.value();
	m_PrePupalRateC = cfg_OsmiaPrepupalRateC.value();
	const double l_topt = cfg_OsmiaPrepupalRateTOpt.value();
	m_PrePupalRateQOpt = m_PrePupalRateA * l_topt * l_topt + m_PrePupalRateB * l_topt + m_PrePupalRateC;
	/** A downward-opening or root-bearing parabola would make the prepupal rate negative or
	* unbounded at some temperature, so both are rejected at startup. */
	if (m_PrePupalRateA <= 0.0 ||
		(m_PrePupalRateB * m_PrePupalRateB - 4.0 * m_PrePupalRateA * m_PrePupalRateC) >= 0.0)
	{
		m_TheLandscape->Warn("Osmia_Population_Manager::Init()",
			"OSMIA_PREPUPALRATE_* quadratic opens downward or has real roots; the prepupal rate would be negative or unbounded at some temperature");
		exit(TOP_Osmia);
	}
	if (m_PrePupalRateQOpt <= 0.0)
	{
		m_TheLandscape->Warn("Osmia_Population_Manager::Init()",
			"OSMIA_PREPUPALRATE_* give a non-positive q(Topt); check the coefficients and OSMIA_PREPUPALRATE_TOPT");
		exit(TOP_Osmia);
	}
	//PartitionLiveDead(int(TTypeOfOsmiaLifeStages::to_OsmiaInCocoon));
	m_is_paralleled = true; // Osmia is always parallel

	//reset the pesticide files
   #ifdef __OSMIA_PESTICIDE_STORE
   ofstream oversprayfile("osmia_overspray.txt", ios::trunc);
   oversprayfile << "Year" << '\t' << "Day" << '\t' << "Female ID" << endl;
   oversprayfile.close();

   ofstream contactfile("osmia_contact.txt", ios::trunc);
   contactfile << "Year" << '\t' << "Day" << '\t' << "Female ID" << '\t' << "Pesticide(g/m2)"  << endl;
   contactfile.close();

   ofstream intakefile("osmia_pest_intake.txt", ios::trunc);
   intakefile << "Year" << '\t' << "Day" << '\t' << "Female ID" << '\t' << "Pesticide(g)" << '\t' << "Sugar(g)" << endl;
   intakefile.close();

   #endif
}

void Osmia_Population_Manager::Init()
{
	/**
	* Initiates the Osmia nests on each landscape element
	* The temperature related mortality needs to be calculated for in nest development and stored in the static variable in the Osmia_Base

	*/
	// struct_Osmia o_data;
	// o_data.age = 0;
	// o_data.L = m_TheLandscape;
	// o_data.OPM = this;
	// o_data.x = 0;
	// o_data.y = 0;
	// o_data.nest = NULL;
	// o_data.parasitised = TTypeOfOsmiaParasitoids::topara_Unparasitised;
	// o_data.mass = 100;
	// o_data.sex = true;
	m_PreWinteringEndFlag = true;
	#ifdef __OSMIATESTING
		m_female_weight_record_lock = new omp_nest_lock_t;
		omp_init_nest_lock(m_female_weight_record_lock);

		for (int i = 0; i < 30; i++)
		{
			m_egghistogram[0][i] = 0;
			m_egghistogram[1][i] = 0;
			m_egghistogram[2][i] = 0;
			m_egghistogram[3][i] = 0;
		}
		m_eggsfirstnest.open("eggsfirstnest.txt", ios_base::out);
		ofstream ofile("OsmiaFemaleWeights.txt", ios::out);
	#endif
	m_OurOsmiaNestManager.InitOsmiaBeeNesting();
	// Set the values of egg/base static variables

	Osmia_Egg::SetParameterValues();
	/*temporarily uses .get()'ed version of GetPopulation to get the raw pointer, will move to smart pointers: issue opened*/
	Osmia_Egg::SetParasitoidManager(static_cast<OsmiaParasitoid_Population_Manager*>(this->m_TheLandscape->SupplyThePopManagerList()->GetPopulation(TOP_OsmiaParasitoids)));
	// Set the values of InCocoon static variables
	Osmia_InCocoon::SetOverwinteringTempThreshold(cfg_OsmiaInCocoonOverwinteringTempThreshold.value());

	// Set the values of female static variables
	Osmia_Female::SetDailyMort(cfg_OsmiaFemaleBckMort.value());
	Osmia_Female::SetMinEggsPerNest(cfg_OsmiaMinNoEggsInNest.value());
	Osmia_Female::SetMaxEggsPerNest(cfg_OsmiaMaxNoEggsInNest.value());
	Osmia_Female::SetCocoonToProvisionMass(cfg_OsmiaProvMassFromCocoonMass.value());// Also sets some other derived static members
	Osmia_Female::SetProvisionToCocoonMass(cfg_OsmiaCocoonMassFromProvMass.value());
	Osmia_Female::SetPollenScoreToMg(cfg_PollenScoreToMg.value());
	Osmia_Female::SetMinimumCellConstructionTime(cfg_MinimumCellConstructionTime.value());
	Osmia_Female::SetMaximumCellConstructionTime(cfg_MaximumCellConstructionTime.value());
	Osmia_Female::SetTotalNestsPossible(cfg_TotalNestsPossible.value());
	Osmia_Female::SetBombylidProbability(cfg_OsmiaBombylidProb.value());
	Osmia_Female::SetParasitismProbToTimeCellOpen(cfg_OsmiaParasitismProbToTimeCellOpen.value());
	Osmia_Female::SetUsingMechanisticParasitoids(cfg_UsingMechanisticParasitoids.value());
	Osmia_Female::SetNestFindAttempts(cfg_OsmiaFemaleFindNestAttemptNo.value());
	Osmia_Female::SetPollenGiveUpThreshold(cfg_OsmiaPollenGiveUpThreshold.value());
	Osmia_Female::SetPollenGiveUpReturn(cfg_OsmiaPollenGiveUpReturn.value());
#ifdef __OSMIARECORDFORAGE
	Osmia_Female::m_foragesum = 0.0;
	Osmia_Female::m_foragecount = 0;
#endif

	Osmia_Female::m_OsmiaEggPPPEffectProb = cfg_OsmiaEggPesticideProbability.value();
	Osmia_Female::m_OsmiaEggPPPThreshold = cfg_OsmiaEggPesticideThreshold.value();
	Osmia_Female::m_OsmiaPPPEffectProb = cfg_OsmiaPesticideProbability.value();
	Osmia_Female::m_OsmiaPPPThreshold = cfg_OsmiaPesticideThreshold.value();
	Osmia_Female::m_OsmiaPPPDecayRate = cfg_OsmiaPesticideDecayRate.value();
	Osmia_Female::m_OsmiaPPPAbsorptionRateOverspray = cfg_OsmiaPesticideAbsorptionRateOverspray.value();
	Osmia_Female::m_OsmiaPPPOversprayBodySurface = cfg_OsmiaPesticideOversprayBodySurface.value();
	Osmia_Female::m_OsmiaPPPAbsorptionRateContact = cfg_OsmiaPesticideAbsorptionRateContact.value();
	Osmia_Female::m_OsmiaPPPContactBodySurface = cfg_OsmiaPesticideContactBodySurface.value();
	Osmia_Female::m_OsmiaPPPOversprayChance = cfg_OsmiaPesticideOversprayChance.value();


	// Read the pollen and nectar thresholds
	OsmiaPollenNectarThresholds pnt;
	for (int m = 0; m < 12; m++)
	{
		pnt.m_pollenTquan = cfg_OsmiaPollenThresholds.value(m);
		pnt.m_pollenTqual = cfg_OsmiaPollenThresholds.value(m + 12);
		pnt.m_nectarTquan = cfg_OsmiaNectarThresholds.value(m);
		pnt.m_nectarTqual = cfg_OsmiaNectarThresholds.value(m + 12);
		m_PN_thresholds.push_back(pnt);
	}

	// Calculates sexratio for egg lookups

	// Create age-dependent lookup curves through at least day 60 and extend them
	// when a longer adult lifespan is configured.
	// Logistic(x | x0, min, max, k) = min + (max - min) / (1 + exp(-k * (x - x0)))
	vector<double> params_logistic, params_lin, params_logistic2, params_lin2; // these are X0,min,max,k, and linear a,b respectively
	params_logistic = cfg_OsmiaSexRatioVsMotherAgeLogistic.value();
	params_lin = cfg_OsmiaSexRatioVsMotherMassLinear.value();
	eggsexratiovsagelogisticcurvedata curve1;
	params_lin2 = Cfg_OsmiaFemaleCocoonMassVsMotherMassLinear.value();
	params_logistic2 = Cfg_OsmiaFemaleCocoonMassVsMotherAgeLogistic.value();
	femalecocoonmassvsagelogisticcurvedata curve2;
	const unsigned adult_lookup_max_age = (Osmia_Base::GetFemaleLifespan() > 60)
		? static_cast<unsigned>(Osmia_Base::GetFemaleLifespan()) : 60U;
	for (double mass = cfg_OsmiaFemaleMassMin.value(); mass <= cfg_OsmiaFemaleMassMax.value(); mass += cfg_OsmiaAdultMassCategoryStep.value())
	{
		for (unsigned age = 0; age <= adult_lookup_max_age; age++)
		{
			// Calculate the data for the egg sex ration curve
			double adjustedmax = params_lin[0] * mass + params_lin[1];
			curve1.push_back(params_logistic[1] + (adjustedmax - params_logistic[1]) / (1 + exp(-params_logistic[3] * (age - params_logistic[0]))));
			// Calculate the data for the cocoon mass vs female mass curve
			double avg_female_cocoon_mass = params_lin2[0] * mass + params_lin2[1];
			double first_female_cocoon_mass = avg_female_cocoon_mass + cfg_Osmia_LifetimeCocoonMassLoss.value() / 2.0; // this lifetime cocoon mass loss nedds to have +/- 5 mg
			// NB this is calculated as provisioning mass
			curve2.push_back( 40.0+ (cfg_OsmiaProvMassFromCocoonMass.value() * (params_logistic2[1] + (first_female_cocoon_mass - params_logistic2[1]) / (1 + exp(-params_logistic2[3] * (age - params_logistic2[0]))))));
		}
		m_EggSexRatioEqns.push_back(curve1);
		m_FemaleCocoonMassEqns.push_back(curve2);
		curve1.clear();
		curve2.clear();
	}
	// Calculate nest provisioning time parameters
	for (int d = 0; d < 365; d++) //here max 60 is needed because then bee will die anyway
	{
		/** Calculate number of hours needed for provisioning of one cell depending on Osmia age [based on Seidelmann 2006]  */
		/***CJT** Changed from daily to hours */
		double eff = 21.643 / (1 + pow(exp(1.0), (log(d) - log(18.888)) * 3.571)); // provisioning efficiency [mg/h]
		double constructime = (2.576 * eff + 56.17) / eff;   // cell construction time [h]
		m_NestProvisioningParameters[d] = int(constructime );	// days needed for 1 cell construction
	}
	/**
	* Sets the parasitoid parameters for per capitata likelihood of parasitism
	*/
	Osmia_Female::SetParasitoidParameters(cfg_OsmiaPerCapitaParasationChance.value());
	/**
	* Creates the data structures to hold Osmia density measures
	*/
	m_GridExtent = SimW / 1000;
	int GEy = SimH / 1000;
	m_FemaleDensityGrid.resize(m_GridExtent * GEy);
	ClearDensityGrid();

	/**
	* Calculate the foraging efficiency with age mg/hr.
	* Sized to cover the full configurable lifespan (m_EmergeAge can reach OSMIA_LIFESPAN);
	* at least 100 so the default curve is unchanged. Prevents an out-of-bounds read at
	* m_FemaleForageEfficiency[m_EmergeAge] when OSMIA_LIFESPAN is calibrated above 100.
	*/
	int forage_eff_max_age = (Osmia_Base::GetFemaleLifespan() > 100) ? Osmia_Base::GetFemaleLifespan() : 100;
	Osmia_Female::AddForageEfficiency(0);
	for (int i = 1; i <= forage_eff_max_age; i++) Osmia_Female::AddForageEfficiency(21.643 / (1 + exp((log(i) - log(18.888)) * 3.571)));
	/**
	* Resets the OsmiaStageLengths output file.
	*/
	#ifdef __OSMIATESTING
	ofstream file1("OsmiaStageLengths.txt", ios::out);
	file1.close();
	#endif

}
//---------------------------------------------------------------------------

void Osmia_Population_Manager::WritePopulationDynamics(){
	if(cfg_OsmiaStorePopulationDynamics.value())
	{
		*m_population_dynamics_file << m_TheLandscape->SupplyYear() << '\t' << m_TheLandscape->SupplyDayInYear();
		//write the new born population
		for(int i = 0; i < m_ListNameLength; i++)
		{
			*m_population_dynamics_file << '\t'<< std::accumulate(m_NewBornPopulation[i].begin(), m_NewBornPopulation[i].end(), 0);
			std::fill(m_NewBornPopulation[i].begin(), m_NewBornPopulation[i].end(), 0); //reset the new born population
		}
		//write the dead population
		for(int i = 0; i < m_ListNameLength; i++)
		{
			*m_population_dynamics_file << '\t'<< std::accumulate(m_DeadPopulation[i].begin(), m_DeadPopulation[i].end(), 0);
			std::fill(m_DeadPopulation[i].begin(), m_DeadPopulation[i].end(), 0); //reset the dead population
		}
		//write the dead population due to pesticide
		for(int i = 0; i < m_ListNameLength; i++)
		{
			*m_population_dynamics_file << '\t'<< std::accumulate(m_DeadPopulationPesticide[i].begin(), m_DeadPopulationPesticide[i].end(), 0);
			std::fill(m_DeadPopulationPesticide[i].begin(), m_DeadPopulationPesticide[i].end(), 0); //reset the dead population
		}
		*m_population_dynamics_file << std::endl;
	}
}

void Osmia_Population_Manager::TheAOROutputProbe() { m_AOR_Probe->DoProbe(int(TTypeOfOsmiaLifeStages::to_OsmiaFemale)); }
//-----------------------------------------------------------------------------

void Osmia_Population_Manager::CreateObjects(TTypeOfOsmiaLifeStages os_type, TAnimal * a_caller, struct_Osmia * data, int number) {
	Osmia_Egg*  new_Osmia_Egg;
	Osmia_Larva*  new_Osmia_Larva;
	Osmia_Prepupa* new_Osmia_Prepupa;
	Osmia_Pupa*  new_Osmia_Pupa;
	Osmia_InCocoon*  new_Osmia_InCocoon;
	Osmia_Female*  new_Osmia_Female;
#ifdef __RECORDOSMIAEGGPRODUCTION
	if (os_type == TTypeOfOsmiaLifeStages::to_OsmiaEgg) RecordEggProduction(number);
#endif
	for (int i = 0; i < number; i++) {
		switch (os_type) {
		case TTypeOfOsmiaLifeStages::to_OsmiaEgg:		
			new_Osmia_Egg = new Osmia_Egg(data);
			PushIndividual(int(os_type),new_Osmia_Egg);		
			IncLiveArraySize(int(os_type));
			data->nest->SetCellLock();
			data->nest->AddEgg(new_Osmia_Egg);
			data->nest->ReleaseCellLock();
			break;
		case TTypeOfOsmiaLifeStages::to_OsmiaLarva:
			new_Osmia_Larva = new Osmia_Larva(data);
			PushIndividual(int(os_type),new_Osmia_Larva);
			IncLiveArraySize(int(os_type));
			// Need to replace the pointer held by the nest (egg->larvae)
			data->nest->SetCellLock();
			data->nest->ReplaceNestPointer(a_caller,new_Osmia_Larva);
			data->nest->ReleaseCellLock();
			break;
		case TTypeOfOsmiaLifeStages::to_OsmiaPrepupa:
			new_Osmia_Prepupa = new Osmia_Prepupa(data);
			PushIndividual(int(os_type),new_Osmia_Prepupa);
			IncLiveArraySize(int(os_type));
			data->nest->SetCellLock();
			data->nest->ReplaceNestPointer(a_caller, new_Osmia_Prepupa);
			data->nest->ReleaseCellLock();
			break;
		case TTypeOfOsmiaLifeStages::to_OsmiaPupa:
			new_Osmia_Pupa = new Osmia_Pupa(data);
			PushIndividual(int(os_type),new_Osmia_Pupa);
			IncLiveArraySize(int(os_type));
			data->nest->SetCellLock();
			data->nest->ReplaceNestPointer(a_caller, new_Osmia_Pupa);
			data->nest->ReleaseCellLock();
			break;
		case TTypeOfOsmiaLifeStages::to_OsmiaInCocoon:
			new_Osmia_InCocoon = new Osmia_InCocoon(data);
			PushIndividual(int(os_type),new_Osmia_InCocoon);		
			IncLiveArraySize(int(os_type));
			data->nest->SetCellLock();
			//cout<<"Osmia cocoon created"<<endl;
			if(a_caller == NULL){
				data->nest->AddCocoon(new_Osmia_InCocoon);
			}
			else{
				data->nest->ReplaceNestPointer(a_caller, new_Osmia_InCocoon);
			}
			//cout << "Nest set created" << endl;
			data->nest->ReleaseCellLock();
			break;
		case TTypeOfOsmiaLifeStages::to_OsmiaFemale:
			new_Osmia_Female = new Osmia_Female(data);
			//code used for pestiside store
			#ifdef __OSMIA_PESTICIDE_STORE
			#pragma omp critical
			{
				m_female_count++;
				new_Osmia_Female->m_animal_id = m_female_count;
			}
			#endif
			PushIndividual(int(os_type),new_Osmia_Female);
			IncLiveArraySize(int(os_type));
			//data->nest->RemoveCell(a_caller);
			break;
		}
		if(cfg_OsmiaStorePopulationDynamics.value())
		{
			m_NewBornPopulation[int(os_type)][omp_get_thread_num()]++;
		}
	}
}
//---------------------------------------------------------------------------
void Osmia_Population_Manager::DoFirst() {
	
	double temp = m_TheLandscape->SupplyTemp();
	Osmia_Base::SetTemp(temp); // Sets the static variable for temperature for all Osmia (speed optimisation)
	//if ((!g_weather->Raining()) && (temp > 10.0) && (g_weather->GetWind() < 8.0))  m_FlyingWeather = true; else m_FlyingWeather = false;
	//if (temp > 10.0 && g_weather->GetWind() < 8.0)  m_FlyingWeather = true; else m_FlyingWeather = false;
	//m_FlyingWeather = g_weather->GetFlyingHours();
	CalForageHours();
	m_OurOsmiaNestManager.UpdateOsmiaNesting(); // Updates nest status for all nests
	ClearDensityGrid(); // Clears this before all the bees get going
	/** Prepupal development rate for today, evaluated directly from the Formal Model quadratic
	* rather than looked up on temperature rounded to the nearest degree. This is done once per
	* day for the whole population; Osmia_Prepupa::st_Develop reads the cached value via
	* Osmia_Population_Manager::GetPrePupalDevelDays.
	*
	* q(T) is guaranteed strictly positive by the coefficient check in Init(), so no guard is
	* needed here and the temperature does not have to be clamped. The former lookup clamped the
	* index below at 0 but not above at 41, so temperatures over 41.5 C indexed past the end of a
	* 42-element vector. */
	m_PrePupalDevelDaysToday = m_PrePupalRateQOpt /
		(m_PrePupalRateA * temp * temp + m_PrePupalRateB * temp + m_PrePupalRateC);

}

#ifdef __OSMIATESTING
void Osmia_Population_Manager::RecordEggProduction(int a_eggs) {
	// A Osmia has become larva, we need to record this for the statistics
	m_OsmiaEggProdStats.add_variable(a_eggs);
}

void Osmia_Population_Manager::RecordEggLength(int a_length) {
	m_EggStageLength.add_variable(a_length);
}

void Osmia_Population_Manager::RecordLarvalLength(int a_length) {	
	m_LarvalStageLength.add_variable(a_length);
}

void Osmia_Population_Manager::RecordPrePupaLength(int a_length) {
	m_PrePupaStageLength.add_variable(a_length);
}

void Osmia_Population_Manager::RecordPupaLength(int a_length) {
	m_PupaStageLength.add_variable(a_length);
}

void Osmia_Population_Manager::RecordInCocoonLength(int a_length) {
	m_InCocoonStageLength.add_variable(a_length);
}
#endif

void Osmia_Population_Manager::CalForageHours(void) {
	//set it to zero first
	m_FlyingWeather = 0;
	int sunrise_time = g_date->SunRiseTime()/60;
	int sunset_time = g_date->SunSetTime()/60;
	for (int i = sunrise_time; i<=sunset_time; i++ ){
		if(g_weather->GetTempHour(i) >= cfg_OsmiaMinTempForFlying.value()){
			if(g_weather->GetWindHour(i) <= cfg_OsmiaMaxWindSpeedForFlying.value()){
				if(g_weather->GetRainHour(i) <= cfg_OsmiaMaxPrecipForFlying.value()){
					m_FlyingWeather += 1.0;
				}
			}
		}
	}
}


#ifdef __OSMIATESTING 
void Osmia_Population_Manager::WriteNestTestData(OsmiaNestData a_target, OsmiaNestData a_achieved)
{
	m_eggsfirstnest << a_target.m_no_eggs << '\t' << a_target.m_no_females << '\t';
	for (int i=0; i < a_target.m_cell_provision.size(); i++) {
		m_eggsfirstnest << a_target.m_cell_provision[i] << '\t';
	}
	m_eggsfirstnest << '\n';

	m_eggsfirstnest << a_achieved.m_no_eggs << '\t' << a_achieved.m_no_females << '\t';
	for (int i = 0; i < a_achieved.m_cell_provision.size(); i++) {
		m_eggsfirstnest << a_achieved.m_cell_provision[i] << '\t';
	}
	m_eggsfirstnest << '\n';
}
#endif

void Osmia_Population_Manager::DoBefore()
{
	/** \brief Things to do before the Step */
	/** To save time we only refil the pollen map if any adults are around to care about this. */
	/** If some stages survive to winter due to slow development we kill these here */
	int today = m_TheLandscape->SupplyDayInYear();
	if (today == 0)
	{
		unsigned size2 = (unsigned)GetLiveArraySize(int(TTypeOfOsmiaLifeStages::to_OsmiaLarva));
		for (unsigned j = 0; j < size2; j++) {
			Osmia_Larva* larva = dynamic_cast<Osmia_Larva*>(SupplyAnimalPtr(int(TTypeOfOsmiaLifeStages::to_OsmiaLarva), j));
			larva->st_Dying();
		}
		size2 = (unsigned)GetLiveArraySize(int(TTypeOfOsmiaLifeStages::to_OsmiaPrepupa));
		for (unsigned j = 0; j < size2; j++) {
			Osmia_Prepupa* prepupa = dynamic_cast<Osmia_Prepupa*>(SupplyAnimalPtr(int(TTypeOfOsmiaLifeStages::to_OsmiaPrepupa), j));
			prepupa->st_Dying();
		}
		size2 = (unsigned)GetLiveArraySize(int(TTypeOfOsmiaLifeStages::to_OsmiaPupa));
		for (unsigned j = 0; j < size2; j++) {
			Osmia_Pupa* pupa = dynamic_cast<Osmia_Pupa*>(SupplyAnimalPtr(int(TTypeOfOsmiaLifeStages::to_OsmiaPupa), j));
			pupa->st_Dying();
		}
#ifdef __OSMIATESTING
		// Record the Osmia weights if needed
		ofstream ofile("OsmiaFemaleWeights.txt", ios::app);
		int sz = m_FemaleWeights.size();
		vector<int> histogram;
		for (int h = 0; h <= 20; h++) histogram.push_back(0);
		for (int i = 0; i < sz; i++) {
			// Create histogram values
			int col = int(m_FemaleWeights[i] / 10);
			histogram[col]++;
		}
		for (int h = 0; h <= 20; h++) {
			ofile << histogram[h] << '\t';
		}
		ofile << endl;
		ofile.close();
		m_FemaleWeights.clear();
#endif

	}
	// Debug code to ensure all nests have some occupied cells.
	if (GetLiveArraySize(int(TTypeOfOsmiaLifeStages::to_OsmiaFemale)) == 0)
	{
		// Only if no females around to mess this up.
		m_OurOsmiaNestManager.CheckZeroNests();
	}
}
//---------------------------------------------------------------------------

OsmiaParasitoid_Population_Manager::OsmiaParasitoid_Population_Manager(Landscape * L, int a_cellsize) : Population_Manager(L)
{
	/**
	* Intialises each subpopulation of Osmia parasitoids. Creates subpopulation maps and intialises each population
	* Parasitoid populations are described by the enum TTypeOfOsmiaParasitoids
	*/
	// Calculate the number of cells
	m_CellSize = a_cellsize;
	m_Wide = (1 + (m_TheLandscape->SupplySimAreaWidth() / m_CellSize));
	m_High = (1 + (m_TheLandscape->SupplySimAreaHeight() / m_CellSize));
	m_Size = m_Wide*m_High;
	unsigned notypes = static_cast<int>(TTypeOfOsmiaParasitoids::topara_foobar) - 1;
	std::vector<OsmiaParasitoidSubPopulation*>::size_type index = notypes * m_Size;;
	m_SubPopulations.resize(index);
	for (unsigned ps = 0; ps < notypes; ps++)
	{
		double range = cfg_OsmiaParasStartHighLow.value(2 * ps) - cfg_OsmiaParasStartHighLow.value(2 * ps + 1);
		for (unsigned int y = 0; y < m_High; y++)
			for (unsigned int x = 0; x < m_Wide; x++)
			{
				OsmiaParasitoidSubPopulation* OPsP = new OsmiaParasitoidSubPopulation(cfg_OsmiaParasDispersal.value(ps), (g_rand_uni_fnc() * range) + cfg_OsmiaParasStartHighLow.value(2 * ps + 1), x, y, m_Wide, m_High, this);
				index = ps* (x + y * m_Wide);
				m_SubPopulations[index] = OPsP;
			}
		array<double, 12> morts;
		for (int m=0; m<12; m++) morts[m]= cfg_OsmiaParasDailyMort.value(ps*12+m);
		m_SubPopulations[0]->SetMortalities(morts); // This is a static member so only the one intialisation is needed, therefore outside the for y loop
	}
}
//---------------------------------------------------------------------------------------------------------------------------------

OsmiaParasitoid_Population_Manager::~OsmiaParasitoid_Population_Manager()
{
	for (int p=0; p< int(m_SubPopulations.size()); p++)
	{
		delete m_SubPopulations[p];
	}
}
//---------------------------------------------------------------------------------------------------------------------------------

array<double, static_cast<unsigned>(TTypeOfOsmiaParasitoids::topara_foobar)> OsmiaParasitoid_Population_Manager::GetParasitoidNumbers(int a_x, int a_y)
{
	array<double, static_cast<unsigned>(TTypeOfOsmiaParasitoids::topara_foobar)> parasitoiddensitylist;
	// First find the right sub-population
	int subpop = (a_x / m_CellSize) + (a_y / m_CellSize) * m_Wide;
	for (unsigned ps = 1; ps < static_cast<int>(TTypeOfOsmiaParasitoids::topara_foobar); ps++)
	{
		parasitoiddensitylist[ps] = m_SubPopulations[subpop]->GetSubPopnSize();
		subpop += m_Size;
	}
	return parasitoiddensitylist;
}
//---------------------------------------------------------------------------------------------------------------------------------

OsmiaParasitoidSubPopulation::OsmiaParasitoidSubPopulation(double a_dispersalfraction, double a_startno, int a_x, int a_y, int a_wide, int a_high, OsmiaParasitoid_Population_Manager* a_popman)
{
	m_NoParasitoids = a_startno;
	m_x = a_x;
	m_y = a_y;
	m_OurPopulationManager = a_popman;
	m_DiffusionRate = a_dispersalfraction;
	
	/**
	* Calculate neighbour cells for dispersal.
	* There will be 8 cells for dispersal
	* We need also to consider the bounds for wrap around
	* m_CellIndexArray is an array containing numbers which represent the cell position in a spatial array (1 dimension representing the matrix). 
	* The actual cells are stored in the population manager for the parasitoids.
	*/
	int count = 0;
	int dx = -1;
	int dy = 0;
	int dxx = 1;
	int dyy = 0;
	for (int x = dx; x <= dxx; x++)
	{
		int actualx = m_x + dx;
		if (actualx < 0) actualx += a_wide;
		if (actualx >= a_wide) actualx -= a_wide;
		int actualy1 = m_y - dx;
		if (actualy1 < 0) actualy1 += a_high;
		int actualy2 = m_y + dx;
		if (actualy2 > a_high) actualy2 -= a_high;
		m_CellIndexArray[count] = actualx + actualy1 * a_wide;
		count++;
		m_CellIndexArray[count] = actualx + actualy2 * a_wide;
		count++;
		// Now do the y
	}
	int actualx1 = m_x + dx;
	int actualx2 = m_x - dx;
	if (actualx1 < 0) actualx1 += a_wide;
	if (actualx2 >= a_wide) actualx2 -= a_wide;
	for (int y = dy; y <= dyy; y++)
	{
		int actualy = m_y + dy;
		if (actualy < 0) actualy += a_high;
		if (actualy > a_high) actualy -= a_high;
		m_CellIndexArray[count] = actualx1 + actualy * a_wide;
		count++;
		m_CellIndexArray[count] = actualx2 + actualy * a_wide;
		count++;
	}

	/**
	* Calculate the diffusion rates.
	* This is done per row by dividing the total diffusion rate by the number of rows,
	* then dividing out result by the number of squares in the row.  Currently we assume dispersal only to the next square (8 round this current population)
	*/
	m_DiffusionConstant = m_DiffusionRate;
}

void OsmiaParasitoidSubPopulation::DailyMortality()
{
	/**
	* Applies a daily probability of mortality for adults. This is provided as a monthly figure, which allows some seaonality to be included.
	* It is a static member so only one copy exists per subpopulation type.
	*/
	m_NoParasitoids *= m_MortalityPerMonth[m_ThisMonth];
}

void OsmiaParasitoidSubPopulation::Dispersal()
{
	/**
	Calculates the number of dispersers per cell to the 8 surrounding cells and removes them from the present cell
	*/
	int count = 0;
	double dispersers = m_NoParasitoids * m_DiffusionConstant;
	m_NoParasitoids-=dispersers;
	double disperserspercell = dispersers / 8.0;
	m_NoParasitoids -= dispersers;
	for (int c = 0; c < 8; c++)
	{
		m_OurPopulationManager->AddDispersers(m_CellIndexArray[count++], dispersers);
	}
}

void OsmiaParasitoidSubPopulation::Reproduce()
{
	/**
	* This links the chance of a cell being parasitised to the local population size.
	* The probability of attack depends on factors under control of the bee therefore currently there is nothing needed here.
	* When an egg is produced the local conditions will determine its fate wrt parasitoids. 
	* Therefore parasitoid attack density related probability curves are a property of the bee.
	*/
}

OsmiaParasitoidSubPopulation::~OsmiaParasitoidSubPopulation()
{
}

void Osmia_Nest_Manager::InitOsmiaBeeNesting()
{
	/** Reads in an input file **Ela** and provides a max nest number to each instance of LE* in the m_elems vector */
	array<TTypesOfLandscapeElement, tole_Foobar> tole_ref;
	array<double, tole_Foobar> maxOsmiaNests;
	std::fill(maxOsmiaNests.begin(), maxOsmiaNests.end(), 0);
	array<double, tole_Foobar> minOsmiaNests;
	std::fill(minOsmiaNests.begin(), minOsmiaNests.end(), 0);
	fstream ifile(cfg_OsmiaNestByLE_Datafile.value(), ios::in);
	if (!ifile.is_open()) {
		g_msg->Warn("Cannot open file: ", cfg_OsmiaNestByLE_Datafile.value());
		exit(1);
	}
	// Read the file tole type by tole type - here we can't rely on the order but need the tole number
	int length;
	ifile >> length;
	if (length != tole_Foobar) {
		g_msg->Warn("Inconsistent file length with tole_Foobar: ", int(tole_Foobar));
		exit(1);
	}
	// read the file
	for (int i = 0; i < length; i++)
	{
		int toleref;
		ifile >> toleref >> minOsmiaNests[i] >> maxOsmiaNests[i];
		tole_ref[i] = g_landscape_ptr->TranslateEleTypes(toleref);
		if (minOsmiaNests[i] > 0) m_PossibleNestType[tole_ref[i]] = true; else m_PossibleNestType[tole_ref[i]] = false;
	}
	ifile.close();
	unsigned nopolys = g_landscape_ptr->SupplyNumberOfPolygons();
	OsmiaPolygonEntry ope(0, 0);
	m_PolyList.resize(nopolys);
	m_PolyListLocks.resize(nopolys);
	double totalnests = 0;
	for (unsigned int e = 0; e < nopolys; e++) {
		m_PolyListLocks[e] = new omp_nest_lock_t;
		omp_init_nest_lock(m_PolyListLocks[e]);
		ope.SetAreaAttribute( int(floor(g_landscape_ptr->SupplyPolygonAreaVector(e)+0.5)));
		ope.SetIndexAttribute(e);
		m_PolyList[e] = ope;
		TTypesOfLandscapeElement eletype = g_landscape_ptr->SupplyElementTypeFromVector(e);
		// first find the eletype
		int found = -1;
		for (int j = 0; j < length; j++)
		{
			if (tole_ref[j] == eletype) {
				found = j;
				break;
			}
		}
		if (found == -1) {
			g_msg->Warn("Inconsistent file data, missing tole type ref: ", eletype);
			exit(1);
		}
		// We have the ref type, so now calculate the number of nests and set it
		// Intentional operational scaling: the input density is converted to 0.001 of its supplied value
		// before multiplication by polygon area. Retained by developer decision during MIDox review.
		double n = (minOsmiaNests[found] + double(g_rand_uni_fnc()  * (maxOsmiaNests[found] - minOsmiaNests[found])))*0.001;
		m_PolyList[e].SetMaxOsmiaNests(n);
		totalnests += m_PolyList[e].GetMaxNoNests(); // Just to have a record of the max possible nests
	}
}
