/*
*******************************************************************************************************
Copyright (c) 2019, Christopher John Topping, Aarhus University
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
/** \file Osmia.cpp
Version of  August 2019 \n
By Chris J. Topping \n \n
*/


#include "../BatchALMaSS/phase_timing.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <random>


#pragma warning( push )
#pragma warning( disable : 4100)
#pragma warning( disable : 4127)
#pragma warning( disable : 4244)
#pragma warning( disable : 4267)
#pragma warning( pop ) 
#include "../BatchALMaSS/ALMaSS_Setup.h"
#include "../ALMaSSDefines.h"
#include "../Landscape/ls.h"
#include "../BatchALMaSS/ALMaSS_Random.h"
#include "../BatchALMaSS/PopulationManager.h"
#include "../Osmia/Osmia.h"
#include "../Osmia/Osmia_Population_Manager.h"


//---------------------------------------------------------------------------

using namespace std;

//---------------------------------------------------------------------------

extern MapErrorMsg *g_msg;
extern CfgFloat cfg_OsmiaAdultMassCategoryStep;
extern CfgFloat cfg_OsmiaCocoonMassFromProvMass;
extern CfgFloat cfg_OsmiaProvMassFromCocoonMass;
extern int g_pest_NoPPPs;

// Day degree development curves for Osmia
/** \brief Is the number of day degrees needed for egg hatch above the developmental threshold for eggs */
static CfgFloat cfg_OsmiaEggDevelTotalDD("OSMIA_EGGDEVELDD", CFG_CUSTOM, 104.435); // Formal Model: 37.0
/** \brief Is temperature developmental threshold for egg development */
static CfgFloat cfg_OsmiaEggDevelThreshold("OSMIA_EGGDEVELTHRESHOLD", CFG_CUSTOM, 0.518); // Formal Model: 13.8
/** \brief Is the number of day degrees needed for larval hatch above the developmental threshold for larvae */
static CfgFloat cfg_OsmiaLarvaDevelTotalDD("OSMIA_LARVADEVELDD", CFG_CUSTOM, 305.235); // Formal Model: 422.4
/** \brief Is temperature developmental threshold for larval development */
static CfgFloat cfg_OsmiaLarvaDevelThreshold("OSMIA_LARVADEVELTHRESHOLD", CFG_CUSTOM, 8.834); // Formal Model: 8.5
/** \brief Is the number of day degrees needed for pupal hatch above the developmental threshold for pupae */
static CfgFloat cfg_OsmiaPupaDevelTotalDD("OSMIA_PUPADEVELDD", CFG_CUSTOM, 555.907); // Formal Model: 272.3
/** \brief Is temperature developmental threshold for pupal development */
static CfgFloat cfg_OsmiaPupaDevelThreshold("OSMIA_PUPADEVELTHRESHOLD", CFG_CUSTOM, 2.463); // Formal Model: 13.2
/** \brief Is the temperature developmental threshold for overwintering development (a temperature below which day degrees are not summed) */
CfgFloat cfg_OsmiaInCocoonOverwinteringTempThreshold("OSMIA_INCOCOONOVERWINTERINGTEMPTHRESHOLD", CFG_CUSTOM, 7.758); // Formal Model: 0.0
/** \brief Is the temperature threshold for calculating days left to emergence (a temperature below which days are not counted) */
CfgFloat cfg_OsmiaInCocoonEmergenceTempThreshold("OSMIA_INCOCOONEMERGENCETEMPTHRESHOLD", CFG_CUSTOM, 10.085); // Formal Model: 12.0
/** \brief Is the temperature developmental threshold for prewintering development (a temperature below which day degrees are not summed) */
CfgFloat cfg_OsmiaInCocoonPrewinteringTempThreshold("OSMIA_INCOCOONPREWINTERINGTEMPTHRESHOLD", CFG_CUSTOM, 15.0);
/** \brief Prepupal stage duration in days at the thermal optimum, scaling the rate function
* declared at @c cfg_OsmiaPrepupalRateA.
*
* Set to the calibrated value, matching OSMIA_PREPUPADEVELDAYS in Osmia_Calibrated.cfg. The
* Formal Model value is 24.3 d (Ziolkowska et al. 2023, after Giejdasz et al. 2016).
*
* \warning The former default of 45 was fitted with the post-increment then present in
* Osmia_Prepupa::st_Develop, where the effective peak rate was 2.0 rather than 1.0, and so
* encoded roughly twice the intended target. It is not transferable now that the increment has
* been removed and must not be reinstated without it.
*
* \note Earlier calibration against the Poznan and Regensburg field series gave 18.0 d at an
* 8 C threshold floor and 15.9 d at a 10 C floor. Neither is the value used here; the current
* calibrated set supersedes them. */
static CfgFloat cfg_OsmiaPrepupaDevelTotalDays("OSMIA_PREPUPADEVELDAYS", CFG_CUSTOM, 21.27); // Formal Model: 24.3
/** \brief Constant term in emergence counter equation for Osmia in cocoon */
CfgFloat cfg_OsmiaInCocoonEmergCountConst("OSMIA_INCOCOONEMERGENCECOUNTERCONST", CFG_CUSTOM, 13.3685); // Formal Model: 39.4819
/** \brief Coefficient in emergence counter equation for Osmia in cocoon */
CfgFloat cfg_OsmiaInCocoonEmergCountSlope("OSMIA_INCOCOONEMERGENCECOUNTERSLOPE", CFG_CUSTOM, -0.01613); // Formal Model: -0.0147

// Mortality parameters for Osmia
// Mortalities for stages egg, larva, pre-pupa and pupa are constant (not depending on temp)
/** \brief Daily unspecified mortality for Osmia eggs */
static CfgFloat cfg_OsmiaEggDailyMORT("OSMIA_EGGDAILYMORT", CFG_CUSTOM, 0.0014);
/** \brief Daily unspecified mortality for Osmia larvae */
static CfgFloat cfg_OsmiaLarvaDailyMORT("OSMIA_LARVADAILYMORT", CFG_CUSTOM, 0.0014);
/** \brief Daily unspecified mortality for Osmia prepupae */
static CfgFloat cfg_OsmiaPrepupaDailyMORT("OSMIA_PREPUPADAILYMORT", CFG_CUSTOM, 0.003);
/** \brief Daily unspecified mortality for Osmia pupae */
static CfgFloat cfg_OsmiaPupaDailyMORT("OSMIA_PUPADAILYMORT", CFG_CUSTOM, 0.003);
/** \brief Constant term in winter mortality equation for Osmia in cocoon */
CfgFloat cfg_OsmiaInCocoonWinterMortConst("OSMIA_INCOCOONWINTERMORTCONST", CFG_CUSTOM, -4.63);
/** \brief Coefficient in winter mortality equation for Osmia in cocoon */
CfgFloat cfg_OsmiaInCocoonWinterMortSlope("OSMIA_INCOCOONWINTERMORTSLOPE", CFG_CUSTOM, 0.05);

// Miscellaneous parameters
/** \brief min possible male mass in mg */
CfgFloat cfg_OsmiaMaleMassMin("OSMIA_MINMALEMASS", CFG_CUSTOM, 88);
/** \brief Maximum possible male mass in mg. */
CfgFloat cfg_OsmiaMaleMassMax("OSMIA_MAXMALEMASS", CFG_CUSTOM, 105.0);
/** \brief min possible female mass in mg */
CfgFloat cfg_OsmiaFemaleMassMin("OSMIA_MINFEMALEMASS", CFG_CUSTOM, 25.0);
/** \brief max possible female mass in mg */
CfgFloat cfg_OsmiaFemaleMassMax("OSMIA_MAXFEMALEMASS", CFG_CUSTOM, 200.0);
/** \brief duration of prenesting in days */
CfgInt cfg_OsmiaFemalePrenestingDuration("OSMIA_PRENESTINGDURATION", CFG_CUSTOM, 2);
/** \brief max lifespan in days */
CfgInt cfg_OsmiaFemaleLifespan("OSMIA_LIFESPAN", CFG_CUSTOM, 60);
/** \brief Constant term in osmia female mass from provision mass calculation */
CfgFloat cfg_OsmiaFemaleMassFromProvMassConst("OSMIA_FEMALEMASSFROMPROVMASSCONST", CFG_CUSTOM, 0.0); // Formal Model: 4.00
/** \brief Coefficient in osmia female mass from provision mass calculation */
CfgFloat cfg_OsmiaFemaleMassFromProvMassSlope("OSMIA_FEMALEMASSFROMPROVMASSSLOPE", CFG_CUSTOM, 0.2324); // Formal Model: 0.25
/** \brief  The Osmial simple insecticide mortality parameter */
CfgFloat cfg_OsmiaInsecticideApplicationMortality("OSMIA_INSECTICIDE_APPLICATION_MORTALITY", CFG_CUSTOM, 0.8);
extern CfgFloat cfg_biocide_reduction_val;
extern CfgBool cfg_OsmiaStorePopulationDynamics;
extern CfgFloat l_pest_zero_threshold_animal;

// Movement distributions
/** @brief Median homing-distance scale in metres used to parameterise adult movement. */
CfgInt cfg_OsmiaTypicalHomingDistance("OSMIA_TYPICALHOMINGDISTANCE", CFG_CUSTOM, 660); // 660 50% of bees cannot find their way home at this distance //this is static for now but can be related to female bee massclass
/** @brief Upper homing-distance scale in metres used to scale dispersal draws. */
CfgInt cfg_OsmiaMaxHomingDistance("OSMIA_MAXHOMINGDISTANCE", CFG_CUSTOM, 1430);  //  90% of bees cannot find their way home at this distance // EZ: change from 1430 to 715 generated negative numbers in pollen mask
// EZ: for now the distributions are the same but I'm leaving them separately if we would like to change that later
/** @brief Distribution family used for adult dispersal distance. */
static CfgStr cfg_OsmiaDispersalMovementProbType("OSMIA_DISPMOVPROBTYPE", CFG_CUSTOM, "BETA");
/** @brief Arguments passed to the configured adult dispersal-distance distribution. */
static CfgStr cfg_OsmiaDispersalMovementProbArgs("OSMIA_DISPMOVPROBARGS", CFG_CUSTOM, "10 5");  // Was 1 2.5 
/** @brief Distribution family retained for general movement-distance draws. */
static CfgStr cfg_OsmiaGeneralMovementProbType("OSMIA_GENMOVPROBTYPE", CFG_CUSTOM, "BETA");
/** @brief Arguments passed to the retained general movement-distance distribution. */
static CfgStr cfg_OsmiaGenerallMovementProbArgs("OSMIA_GENMOVPROBARGS", CFG_CUSTOM, "10 5");  // Was 1 2.5 
/** \brief Distribution type for the planned eggs per nest probability distribution */
static CfgStr cfg_OsmiaEggsPerNestProbType("OSMIA_EGGSPERNESTPROBYPE", CFG_CUSTOM, "BETA");
/** \brief Arguments for the planned eggs per nest probability distribution */
static CfgStr cfg_OsmiaEggsPerNestProbArgs("OSMIA_EGGSPERNESTPROBARGS", CFG_CUSTOM, "2.277 5.940"); // Formal Model: "1.8 5.0"

// Emergence distributions
/** @brief Distribution family used for individual emergence-delay draws. */
static CfgStr cfg_OsmiaEmergenceProbType("OSMIA_EMERGENCEPROBTYPE", CFG_CUSTOM, "DISCRETE");
/** @brief Weights passed to the discrete emergence-delay distribution. */
static CfgStr cfg_OsmiaEmergenceProbArgs("OSMIA_EMERGENCEPROBARGS", CFG_CUSTOM, "8 7 9 24 20 8 6 5 5 4 4"); // data from A.Bednarska

// Foraging
/** \brief A cap (mg) on the amount of pollen possible to bring back - this is because pollen densities in the landscape can be very high */
static CfgFloat cfg_OsmiaMaxPollen("OSMIA_MAXPOLLEN", CFG_CUSTOM, 2.5);
/** \brief The amount of sugar in mg that a female osmia needs per day.*/
static CfgFloat cfg_OsmiaSugarPerDay("OSMIA_NECTAR_PER_DAY", CFG_CUSTOM, 20); 
// Static initialisation
probability_distribution Osmia_Base::m_emergenceday = probability_distribution(cfg_OsmiaEmergenceProbType.value(), cfg_OsmiaEmergenceProbArgs.value());
probability_distribution Osmia_Base::m_dispersalmovementdistances = probability_distribution(cfg_OsmiaDispersalMovementProbType.value(), cfg_OsmiaDispersalMovementProbArgs.value());
probability_distribution Osmia_Base::m_generalmovementdistances = probability_distribution(cfg_OsmiaGeneralMovementProbType.value(), cfg_OsmiaGenerallMovementProbArgs.value());
probability_distribution Osmia_Base::m_eggspernestdistribution = probability_distribution(cfg_OsmiaEggsPerNestProbType.value(), cfg_OsmiaEggsPerNestProbArgs.value());
probability_distribution Osmia_Base::m_exp_ZeroToOne = probability_distribution("BETA", "1.0, 5.0");
/** \brief Half width of the maximum square that a female can search for pollen. */
CfgInt cfg_OsmiaMaxHalfWidthForageMask("OSMIA_MAX_HALF_WIDTH_FORAGE_MASK", CFG_CUSTOM, 600);
/** \brief The incremental for searching resource mask.*/
CfgInt cfg_OsmiaForageMaskStep("OSMIA_FORAGE_MASK_STEP", CFG_CUSTOM, 50);
/** \brief The flag to use threshold based pesticide response for females.*/
CfgBool cfg_OsmiaFemaleThresholdBasedPesticideResponse("OSMIA_FEMALE_THRESHOLD_BASED_PESTICIDE_RESPONSE", CFG_CUSTOM, false);
/** \brief The flag to use damage based pesticide response for females. */
CfgBool cfg_OsmiaFemaleDamageBasedPesticideResponse("OSMIA_FEMALE_DAMAGE_BASED_PESTICIDE_RESPONSE", CFG_CUSTOM, false);
/** \brief The flag to use threshold based pesticide response for eggs. */
CfgBool cfg_OsmiaEggThresholdBasedPesticideResponse("OSMIA_EGG_THRESHOLD_BASED_PESTICIDE_RESPONSE", CFG_CUSTOM, false);
/** \brief The flag to use backgroud pesticide response for females. */
CfgBool cfg_OsmiaFemaleBcackgroundPeticideResponse("OSMIA_FEMALE_BACKGROUND_PESTICIDE_RESPONSE", CFG_CUSTOM, true);

static std::uniform_int_distribution<int> g_uni_0to35(0, 35);
extern thread_local std::mt19937 g_generator;
extern CfgBool l_pest_enable_pesticide_engine;

//********************************************************************************************************************************
//**************************************** Osmia_Base Definition ******************************************************************
//*******************************************************************************************************************************/

Osmia_Base::Osmia_Base(struct_Osmia* data) : TAnimalToxicity(data->x,data->y)
{
	ReInit(data);
	// Assign the pointer to the population manager
	m_OurPopulationManager = data->OPM;
	m_CurrentOState = toOsmias_InitialState;
	m_OurNest = data->nest;
	SetAge(data->age); // Set the age
	SetMass(data->mass);
	SetParasitised(data->parasitised);
}
//--------------------------------------------------------------------------------------------------------------------------------

void Osmia_Base::ReInit(struct_Osmia* data) {
	TAnimal::ReinitialiseObject(data->x, data->y);
	// Assign the pointer to the population manager
	m_OurPopulationManager = data->OPM;
	m_CurrentOState = toOsmias_InitialState;
	SetAge(data->age); // Set the age
	SetMass(data->mass);
	SetParasitised(data->parasitised);
}
//--------------------------------------------------------------------------------------------------------------------------------

Osmia_Base::~Osmia_Base(void)
{
	;
}
//--------------------------------------------------------------------------------------------------------------------------------

void Osmia_Base::SetParameterValues() {
	// Mortality
	m_DailyDevelopmentMortEggs = cfg_OsmiaEggDailyMORT.value();
	m_DailyDevelopmentMortLarvae = cfg_OsmiaLarvaDailyMORT.value();
	m_DailyDevelopmentMortPrepupae = cfg_OsmiaPrepupaDailyMORT.value();
	m_DailyDevelopmentMortPupae = cfg_OsmiaPupaDailyMORT.value();
	m_OsmiaInCocoonWinterMortConst = cfg_OsmiaInCocoonWinterMortConst.value();
	m_OsmiaInCocoonWinterMortSlope = cfg_OsmiaInCocoonWinterMortSlope.value();
	// Development
	m_OsmiaEggDevelTotalDD = cfg_OsmiaEggDevelTotalDD.value();
	m_OsmiaEggDevelThreshold = cfg_OsmiaEggDevelThreshold.value();
	m_OsmiaLarvaDevelThreshold = cfg_OsmiaLarvaDevelThreshold.value();
	m_OsmiaLarvaDevelTotalDD = cfg_OsmiaLarvaDevelTotalDD.value();
	m_OsmiaPupaDevelTotalDD = cfg_OsmiaPupaDevelTotalDD.value();
	m_OsmiaPupaDevelThreshold = cfg_OsmiaPupaDevelThreshold.value();
	m_OsmiaPrepupalDevelTotalDays = cfg_OsmiaPrepupaDevelTotalDays.value();
	m_OsmiaPrepupalDevelTotalDays10pct = cfg_OsmiaPrepupaDevelTotalDays.value()*0.1;
	m_OsmiaInCocoonOverwinteringTempThreshold  = cfg_OsmiaInCocoonOverwinteringTempThreshold.value();
	m_OsmiaInCocoonEmergenceTempThreshold = cfg_OsmiaInCocoonEmergenceTempThreshold.value();
	m_OsmiaInCocoonPrewinteringTempThreshold = cfg_OsmiaInCocoonPrewinteringTempThreshold.value();
	m_OsmiaInCocoonEmergCountConst = cfg_OsmiaInCocoonEmergCountConst.value();
	m_OsmiaInCocoonEmergCountSlope = cfg_OsmiaInCocoonEmergCountSlope.value();
	// Mass
	m_OsmiaFemaleMassFromProvMassConst = cfg_OsmiaFemaleMassFromProvMassConst.value();
	m_OsmiaFemaleMassFromProvMassSlope = cfg_OsmiaFemaleMassFromProvMassSlope.value();
	m_MaleMaxMass = cfg_OsmiaMaleMassMax.value();
	m_FemaleMinMass = cfg_OsmiaFemaleMassMin.value();
	m_FemaleMaxMass = cfg_OsmiaFemaleMassMax.value();
	m_FemaleMinTargetProvisionMass = ((m_FemaleMinMass - m_OsmiaFemaleMassFromProvMassConst) / m_OsmiaFemaleMassFromProvMassSlope);
	m_FemaleMaxTargetProvisionMass = ((m_FemaleMaxMass - m_OsmiaFemaleMassFromProvMassConst) / m_OsmiaFemaleMassFromProvMassSlope);
	m_MaleMinTargetProvisionMass = m_FemaleMinTargetProvisionMass * 0.95; // This must be smaller than the female min mass
	//m_MaleMaxTargetProvisionMass = m_FemaleMaxTargetProvisionMass * 0.61; // Note here we use the female values and scale it by the literature value ratio
	m_MaleMaxTargetProvisionMass = ((m_MaleMaxMass - m_OsmiaFemaleMassFromProvMassConst) / m_OsmiaFemaleMassFromProvMassSlope);
	// Movement/dispersal
	m_OsmiaFemaleR50distance = cfg_OsmiaTypicalHomingDistance.value();
	m_OsmiaFemaleR90distance = cfg_OsmiaMaxHomingDistance.value();

	m_OsmiaFemalePrenesting = cfg_OsmiaFemalePrenestingDuration.value();
	m_OsmiaFemaleLifespan = cfg_OsmiaFemaleLifespan.value();

	// Cache foraging constants once (these cfgs are static/file-scoped here; avoids a per-forage .value()).
	Osmia_Female::SetMaxPollen(cfg_OsmiaMaxPollen.value());
	Osmia_Female::SetSugarPerDay(cfg_OsmiaSugarPerDay.value());
}
//--------------------------------------------------------------------------------------------------------------------------------

void Osmia_Base::st_Dying( void )
{
	KillThis(); // this will kill the animal object and free up space
	m_OurNest->RemoveCell(this);
}
//--------------------------------------------------------------------------------------------------------------------------------

//********************************************************************************************************************************
//**************************************** Osmia_Egg Definition ******************************************************************
//*******************************************************************************************************************************/

Osmia_Egg::~Osmia_Egg(void)
{
	;
}
//--------------------------------------------------------------------------------------------------------------------------------

Osmia_Egg::Osmia_Egg(struct_Osmia* data) : Osmia_Base(data)
{
	ReInit(data);
	m_AgeDegrees = 0;
	m_Sex = data->sex;
	m_OurNest = data->nest;
	m_StageAge = data->age;;
	if(l_pest_enable_pesticide_engine.value()){
		if(data->pest_mortality > 0) m_egg_pest_mortality = data->pest_mortality;
	}
}
//--------------------------------------------------------------------------------------------------------------------------------

void Osmia_Egg::ReInit(struct_Osmia* data) {
	Osmia_Base::ReInit(data);
	m_AgeDegrees = 0;
	m_Sex = data->sex;
	m_OurNest = data->nest;
	m_StageAge = data->age;
	if(l_pest_enable_pesticide_engine.value()){
		//mark an egg as death because of pesticide
		if(data->pest_mortality > 0) m_egg_pest_mortality = data->pest_mortality;
	}
}
//--------------------------------------------------------------------------------------------------------------------------------

void Osmia_Egg::Step(void)
{
	PHASE_TIMER(PH_OsmiaImmStep);
	/**
	* Osmia egg behaviour is simple. It calls develop until the egg hatches or dies.
	*/
	if (m_StepDone || m_CurrentStateNo == -1) return;
	switch (m_CurrentOState)
	{
	case toOsmias_InitialState: // Initial state always starts with develop
		m_CurrentOState = toOsmias_Develop;
		break;
	case toOsmias_Develop:
		m_CurrentOState = st_Develop();
		m_StepDone = true;
		break;
	case toOsmias_NextStage:
		m_CurrentOState = st_Hatch();
		break;
	case toOsmias_Die:
		st_Dying(); // No return value - no behaviour after this
		m_StepDone = true;
		if(cfg_OsmiaStorePopulationDynamics.value()) m_OurPopulationManager->AddOneDeath(int(TTypeOfOsmiaLifeStages::to_OsmiaEgg), omp_get_thread_num());
		break;
	default:
		m_OurLandscape->Warn("Osmia_Egg::Step()", "unknown state - default");
		std::exit(TOP_Osmia);
	}
}
//--------------------------------------------------------------------------------------------------------------------------------

void Osmia_Egg::BeginStep(void)
{
	//killed by pesticide
	if(!m_OurNest->GetIsOpen() && l_pest_enable_pesticide_engine.value() && m_egg_pest_mortality > 0){
		if(cfg_OsmiaEggThresholdBasedPesticideResponse.value()){
			if (g_rand_uni_fnc()<m_egg_pest_mortality){
				if(cfg_OsmiaStorePopulationDynamics.value()) m_OurPopulationManager->AddOneDeathPesticide(int(TTypeOfOsmiaLifeStages::to_OsmiaEgg), omp_get_thread_num());
				m_CurrentOState = toOsmias_Die;
			}
			else{
				m_egg_pest_mortality = 0; //only die ones, otherwise set it to 0
			}
		}
	}
}

TTypeOfOsmiaState Osmia_Egg::st_Develop(void)
{
	/*
	* Development is preceded by a mortality test, then a day degree calculation is made to determine the development that occured in the last 24 hours.
	* When enough day degrees are achieved the egg hatches.If it does not hatch then the development behaviour is queued up for the next day.
	*/
	if (!m_OurNest->GetIsOpen()){ 
		if (DailyMortality()) return toOsmias_Die;
	}
	m_Age++;
	double DD = m_TempToday- m_OsmiaEggDevelThreshold;
	if (DD > 0) m_AgeDegrees += DD;
	if (m_AgeDegrees > m_OsmiaEggDevelTotalDD) return toOsmias_NextStage;
	return toOsmias_Develop;
}
//--------------------------------------------------------------------------------------------------------------------------------

TTypeOfOsmiaState Osmia_Egg::st_Hatch(void)
{
	/**
	* Creates a new larva object and passes the data from the egg to it, then signals egg object removal.
	*/
	struct_Osmia sO;
	sO.OPM = m_OurPopulationManager;
	sO.L = m_OurLandscape;
	sO.age = m_Age;
	sO.x = m_Location_x;
	sO.y = m_Location_y;
	sO.nest = m_OurNest;
	sO.parasitised = m_ParasitoidStatus;
	sO.mass = m_Mass;
	sO.sex = m_Sex;
	m_OurPopulationManager->CreateObjects(TTypeOfOsmiaLifeStages::to_OsmiaLarva, this, &sO, 1); // 
	#ifdef __OSMIATESTING
	m_OurPopulationManager->RecordEggLength(m_Age - m_StageAge);
	#endif
	KillThis(); // sets current state to -1 and StepDone to true;
	return toOsmias_Emerged; // This is just to have a return value, it is not used
}
//--------------------------------------------------------------------------------------------------------------------------------


//********************************************************************************************************************************
//**************************************** Osmia_Larva Definition ******************************************************************
//*******************************************************************************************************************************/

void Osmia_Larva::ReInit(struct_Osmia* data)
{
	Osmia_Egg::ReInit(data);
}
//--------------------------------------------------------------------------------------------------------------------------------

Osmia_Larva::~Osmia_Larva(void)
{
	;
}
//--------------------------------------------------------------------------------------------------------------------------------

Osmia_Larva::Osmia_Larva(struct_Osmia* data) : Osmia_Egg(data)
{
	ReInit(data);
}
//--------------------------------------------------------------------------------------------------------------------------------

void Osmia_Larva::Step(void)
{
	PHASE_TIMER(PH_OsmiaImmStep);
	/**
	* Osmia larva behaviour is simple. It calls develop until the larva prepupates or dies.
	*/
	if (m_StepDone || m_CurrentStateNo == -1) return;
	switch (m_CurrentOState)
	{
	case toOsmias_InitialState: // Initial state always starts with develop
		m_CurrentOState = toOsmias_Develop;
		break;
	case toOsmias_Develop:
		m_CurrentOState = st_Develop(); 
		m_StepDone = true;
		break;
	case toOsmias_NextStage:
		m_CurrentOState = st_Prepupate(); 
		break;
	case toOsmias_Die:
		st_Dying(); // No return value - no behaviour after this
		m_StepDone = true;
		if(cfg_OsmiaStorePopulationDynamics.value()) m_OurPopulationManager->AddOneDeath(int(TTypeOfOsmiaLifeStages::to_OsmiaLarva), omp_get_thread_num());
		break;
	default:
		m_OurLandscape->Warn("Osmia_Larva::Step()", "unknown state - default");
		std::exit(TOP_Osmia);
	}
}
//--------------------------------------------------------------------------------------------------------------------------------

TTypeOfOsmiaState Osmia_Larva::st_Develop(void)
{
	if (!m_OurNest->GetIsOpen())
		if (DailyMortality()) return toOsmias_Die;
	m_Age++;
	double DD = m_OurLandscape->SupplyTemp() - m_OsmiaLarvaDevelThreshold;
	if (DD > 0) m_AgeDegrees += DD;
	if (m_AgeDegrees > m_OsmiaLarvaDevelTotalDD) return toOsmias_NextStage;
	return toOsmias_Develop;
}
//--------------------------------------------------------------------------------------------------------------------------------

TTypeOfOsmiaState Osmia_Larva::st_Prepupate(void)
{
	/**
	* Creates a new prepupa object and passes the data from the larva to it, then signals young object removal.
	*/
	struct_Osmia sO;
	sO.OPM = m_OurPopulationManager;
	sO.L = m_OurLandscape;
	sO.age = m_Age;
	sO.x = m_Location_x;
	sO.y = m_Location_y;
	sO.nest = m_OurNest;
	sO.mass = m_Mass;
	sO.parasitised = m_ParasitoidStatus;
	sO.sex = m_Sex;
	m_OurPopulationManager->CreateObjects(TTypeOfOsmiaLifeStages::to_OsmiaPrepupa, this, &sO, 1); // 
	#ifdef __OSMIATESTING
	m_OurPopulationManager->RecordLarvalLength(m_Age-m_StageAge);
	#endif
	KillThis(); // sets current state to -1 and StepDone to true;
	return toOsmias_Emerged; // This is just to have a return value, it is not used
}
//--------------------------------------------------------------------------------------------------------------------------------


//********************************************************************************************************************************
//**************************************** Osmia_Prepupa Definition ******************************************************************
//*******************************************************************************************************************************/

void Osmia_Prepupa::ReInit(struct_Osmia* data)
{
	Osmia_Larva::ReInit(data);
}
//--------------------------------------------------------------------------------------------------------------------------------

Osmia_Prepupa::~Osmia_Prepupa(void)
{
	;
}
//--------------------------------------------------------------------------------------------------------------------------------

Osmia_Prepupa::Osmia_Prepupa(struct_Osmia* data) : Osmia_Larva(data)
{
	ReInit(data);
	m_AgeDegrees = 0;
	double max20pct = (m_OsmiaPrepupalDevelTotalDays * 0.2 * g_rand_uni_fnc());
	m_myOsmiaPrepupaDevelTotalDays = m_OsmiaPrepupalDevelTotalDays + max20pct - m_OsmiaPrepupalDevelTotalDays10pct;
}
//--------------------------------------------------------------------------------------------------------------------------------

void Osmia_Prepupa::Step(void)
{
	PHASE_TIMER(PH_OsmiaImmStep);
	/**
	* Osmia prepupa behaviour is simple. It calls develop until the prepupa pupates or dies.
	*/
	if (m_StepDone || m_CurrentStateNo == -1) return;
	switch (m_CurrentOState)
	{
	case toOsmias_InitialState: // Initial state always starts with develop
		m_CurrentOState = toOsmias_Develop;
		break;
	case toOsmias_Develop:
		m_CurrentOState = st_Develop();
		m_StepDone = true;
		break;
	case toOsmias_NextStage:
		m_CurrentOState = st_Pupate(); // Will cause the pupa object to be replaced with an adult in cocoon
		break;
	case toOsmias_Die:
		st_Dying(); // No return value - no behaviour after this
		m_StepDone = true;
		if(cfg_OsmiaStorePopulationDynamics.value()) m_OurPopulationManager->AddOneDeath(int(TTypeOfOsmiaLifeStages::to_OsmiaPrepupa), omp_get_thread_num());
		break;
	default:
		m_OurLandscape->Warn("Osmia_Prepupa::Step()", "unknown state - default");
		std::exit(TOP_Osmia);
	}
}
//--------------------------------------------------------------------------------------------------------------------------------

TTypeOfOsmiaState Osmia_Prepupa::st_Develop(void)
{
	/** 
	* Development occurs if the prepupa does not die of non-specified causes. Temperature drives the basic development
	* towards a target m_myOsmiaPrepupaDevelTotalDays. This has individual variation built in around a mean value.
	*/
	if (DailyMortality()) return toOsmias_Die;
	// Get the temperature dependent development
	m_Age++;
	m_AgeDegrees += m_OurPopulationManager->GetPrePupalDevelDays();
	/** The comparison below previously used a post-increment (m_AgeDegrees++), which added a flat
	* 1.0 per day on top of the temperature-dependent rate. Because the rate function is normalised
	* to a maximum of 1.0, that flat term was of the same order as the entire temperature signal:
	* it roughly halved the stage and cut the 10 C-to-22 C duration ratio from 3.28 to 1.53. It is
	* removed so that @c cfg_OsmiaPrepupaDevelTotalDays means what its name and the Formal Model say
	* it means - the stage duration in days at the thermal optimum.
	*
	* The test remains a strict >, matching Osmia_Egg, Osmia_Larva and Osmia_Pupa, so the day on
	* which the target is crossed still counts towards the stage duration. */
	if (m_AgeDegrees > m_myOsmiaPrepupaDevelTotalDays) return toOsmias_NextStage;
	return toOsmias_Develop;
}
//--------------------------------------------------------------------------------------------------------------------------------

TTypeOfOsmiaState Osmia_Prepupa::st_Pupate(void)
{
	/**
	* Determines sex, and creates a new Osmia pupa object and passes the data from the prepupa to it, then signals young object removal.
	*/
	struct_Osmia sO;
	sO.OPM = m_OurPopulationManager;
	sO.L = m_OurLandscape;
	sO.age = m_Age;
	sO.x = m_Location_x;
	sO.y = m_Location_y;
	sO.nest = m_OurNest;
	sO.mass = m_Mass;
	sO.parasitised = m_ParasitoidStatus;
	sO.sex = m_Sex;
	m_OurPopulationManager->CreateObjects(TTypeOfOsmiaLifeStages::to_OsmiaPupa, this, &sO, 1);
	#ifdef __OSMIATESTING
	m_OurPopulationManager->RecordPrePupaLength(m_Age - m_StageAge);
	#endif
	KillThis(); // sets current state to -1 and StepDone to true;
	return toOsmias_Emerged; // This is just to have a return value, it is not used
}
//--------------------------------------------------------------------------------------------------------------------------------


//********************************************************************************************************************************
//**************************************** Osmia_Pupa Definition ******************************************************************
//*******************************************************************************************************************************/

void Osmia_Pupa::ReInit(struct_Osmia* data)
{
	Osmia_Prepupa::ReInit(data);
}
//--------------------------------------------------------------------------------------------------------------------------------

Osmia_Pupa::~Osmia_Pupa(void)
{
	;
}
//--------------------------------------------------------------------------------------------------------------------------------

Osmia_Pupa::Osmia_Pupa(struct_Osmia* data) : Osmia_Prepupa(data)
{
	ReInit(data);
}
//--------------------------------------------------------------------------------------------------------------------------------

void Osmia_Pupa::Step(void)
{
	PHASE_TIMER(PH_OsmiaImmStep);
	/**
	* Osmia pupa behaviour is simple. It calls develop until the pupa emerges or dies.
	*/
	if (m_StepDone || m_CurrentStateNo == -1) return;
	switch (m_CurrentOState)
	{
	case toOsmias_InitialState: // Initial state always starts with develop
		m_CurrentOState = toOsmias_Develop;
		break;
	case toOsmias_Develop:
		m_CurrentOState = st_Develop(); 
		m_StepDone = true;
		break;
	case toOsmias_NextStage:
		m_CurrentOState = st_Emerge(); // Will cause the pupa object to be replaced with an adult in cocoon
		break;
	case toOsmias_Die:
		st_Dying(); // No return value - no behaviour after this
		m_StepDone = true;
		if(cfg_OsmiaStorePopulationDynamics.value()) m_OurPopulationManager->AddOneDeath(int(TTypeOfOsmiaLifeStages::to_OsmiaPupa), omp_get_thread_num());
		break;
	default:
		m_OurLandscape->Warn("Osmia_Pupa::Step()", "unknown state - default");
		std::exit(TOP_Osmia);
	}
}
//--------------------------------------------------------------------------------------------------------------------------------

TTypeOfOsmiaState Osmia_Pupa::st_Develop(void)
{
	if (DailyMortality()) return toOsmias_Die;
	m_Age++;
	double DD = m_OurLandscape->SupplyTemp() - m_OsmiaPupaDevelThreshold;
	if (DD > 0) m_AgeDegrees += DD;
	if (m_AgeDegrees > m_OsmiaPupaDevelTotalDD)
	{
		return toOsmias_NextStage;
	}
	return toOsmias_Develop;
}
//--------------------------------------------------------------------------------------------------------------------------------

TTypeOfOsmiaState Osmia_Pupa::st_Emerge(void)
{
	/**
	* Determines sex, and creates a new Osmia adult in cocoon object and passes the data from the pupa to it, then signals young object removal.
	*/
	struct_Osmia sO;
	sO.OPM = m_OurPopulationManager;
	sO.L = m_OurLandscape;
	sO.age = m_Age;
	sO.x = m_Location_x;
	sO.y = m_Location_y;
	sO.nest = m_OurNest;
	sO.parasitised = m_ParasitoidStatus;
	sO.mass = m_Mass;
	sO.sex = m_Sex;
	m_OurPopulationManager->CreateObjects(TTypeOfOsmiaLifeStages::to_OsmiaInCocoon, this, &sO, 1);
	#ifdef __OSMIATESTING
	m_OurPopulationManager->RecordPupaLength(m_Age - m_StageAge);
	#endif
	KillThis(); // sets current state to -1 and StepDone to true;
	return toOsmias_Emerged; // This is just to have a return value, it is not used
}
//--------------------------------------------------------------------------------------------------------------------------------

//********************************************************************************************************************************
//**************************************** Osmia_InCocoon Definition ******************************************************************
//*******************************************************************************************************************************/

void Osmia_InCocoon::ReInit(struct_Osmia* data)
{
	Osmia_Pupa::ReInit(data);
	m_emergencecounter = 99999;
	m_DDPrewinter = 0.0;
	m_AgeDegrees = data->overwintering_degree_days;
}
//--------------------------------------------------------------------------------------------------------------------------------

Osmia_InCocoon::~Osmia_InCocoon(void)
{
	;
}
//--------------------------------------------------------------------------------------------------------------------------------

Osmia_InCocoon::Osmia_InCocoon(struct_Osmia* data) : Osmia_Pupa(data)
{
	ReInit(data);
	m_emergencecounter = 99999;
	m_DDPrewinter = 0.0;
}
//--------------------------------------------------------------------------------------------------------------------------------

void Osmia_InCocoon::Step(void)
{
	PHASE_TIMER(PH_OsmiaImmStep);
	/**
	* Osmia adult in cocoon behaviour is simple. It calls develop until the adult in cocoon emerges or dies.
	*/
	if (m_StepDone || m_CurrentStateNo == -1) return;
	switch (m_CurrentOState)
	{
	case toOsmias_InitialState: // Initial state always starts with develop
		m_CurrentOState = toOsmias_Develop;
		break;
	case toOsmias_Develop:
		m_CurrentOState = st_Develop();
		m_StepDone = true;
		break;
	case toOsmias_NextStage:
		m_CurrentOState = st_Emerge(); // Will cause the Osmia in cocoon object to be replaced with an adult
		break;
	case toOsmias_Die:
		st_Dying(); // No return value - no behaviour after this
		m_StepDone = true;
		if(cfg_OsmiaStorePopulationDynamics.value()) m_OurPopulationManager->AddOneDeath(int(TTypeOfOsmiaLifeStages::to_OsmiaInCocoon), omp_get_thread_num());
		break;
	default:
		m_OurLandscape->Warn("Osmia_InCocoon::Step()", "unknown state - default");
		std::exit(TOP_Osmia);
	}
}
//--------------------------------------------------------------------------------------------------------------------------------

TTypeOfOsmiaState Osmia_InCocoon::st_Develop(void)
{
	/**
	* This is/must be called each day.
		* The population manager detects the end of pre-wintering from a sustained autumn temperature
		* decline and records the result in a shared seasonal flag. This method then switches from
		* pre-winter degree-day accumulation to the overwintering and spring-emergence calculations.
	*/
	m_Age++;
	if (m_OurPopulationManager->IsEndPreWinter())
	{
		// Must be after pre-wintering
		if (!m_OurPopulationManager->IsOverWinterEnd())
		{
			// The pre-wintering is over, but its not 1st of March yet 
			double DD = m_TempToday - m_OsmiaInCocoonOverwinteringTempThreshold;
			if (DD > 0) m_AgeDegrees += DD;
		}
		else // It is >= March 1st
		{
			if (m_DayInYear == March+1) { // if first day of March
				m_emergencecounter = int(m_OsmiaInCocoonEmergCountConst + m_OsmiaInCocoonEmergCountSlope * m_AgeDegrees) + m_emergenceday.Geti() + m_OurNest->GetAspectDelay();
			}
			else if (m_TempToday >= m_OsmiaInCocoonEmergenceTempThreshold)
			{
				if (--m_emergencecounter < 1)
				{
					if (WinterMortality()) return toOsmias_Die; // a once only test for overwintering mortality
					else return toOsmias_NextStage;
				}
			}

			// Too late to emerge -- applied regardless of today's temperature so a cocoon that
			// never reached the emergence threshold (e.g. a persistently cold spring) still dies
			// at the cutoff instead of being stuck in Develop indefinitely.
			if(m_DayInYear == June-1){
				return toOsmias_Die;
			}
		}
	}
	else
	{
		// Must be pre-wintering so count up prewintering day degrees
		if (m_TempToday > m_OsmiaInCocoonPrewinteringTempThreshold) m_DDPrewinter += (m_TempToday - m_OsmiaInCocoonPrewinteringTempThreshold);
	}
	return toOsmias_Develop;
}
//--------------------------------------------------------------------------------------------------------------------------------

TTypeOfOsmiaState Osmia_InCocoon::st_Emerge(void)
{
	/**
	* If this is a male (sex == false) we quietly let it vanish, since we do not model adult males.
	*/
	if (m_Sex) {
		/**
		* If parasitised then first determine the result of the parasitism
		*/
		if (m_ParasitoidStatus != TTypeOfOsmiaParasitoids::topara_Unparasitised)
		{
			/**
			/switch (m_ParasitoidStatus)
			{
			case TTypeOfOsmiaParasitoids::topara_Bombylid:
				m_OurNest->KillAllSubsequentCells(this);
				m_OurParasitoidPopulationManager->AddParasitoid(TTypeOfOsmiaParasitoids::topara_Bombylid, m_Location_x, m_Location_y);
				break;
			case TTypeOfOsmiaParasitoids::topara_Cleptoparasite:
				m_OurParasitoidPopulationManager->AddParasitoid(TTypeOfOsmiaParasitoids::topara_Cleptoparasite, m_Location_x, m_Location_y);
				break;
			}
			*/
			return toOsmias_Die; // ***WIP*** Right now they die, but we could add the fact that they may emerge smaller - if so can we find parameters [Ela: for later model version]

		}
		/**
		* Creates a new Osmia adult object and passes the data from the pupa to it, then signals young object removal.
		*/
		struct_Osmia sO;
		sO.OPM = m_OurPopulationManager;
		sO.L = m_OurLandscape;
		sO.age = 0; // Reset the age so we count adult days from now
		sO.x = m_Location_x;
		sO.y = m_Location_y;
		sO.nest = nullptr; //no nest for females
		sO.parasitised = TTypeOfOsmiaParasitoids::topara_Unparasitised;
		sO.sex = m_Sex;
		/**
		* Osmia_Female mass can be calculated from the Osmia_InCocoon mass as follows:\n
		* bee_mass = 4.0 + cocoon_mass * 0.8
		*
		* The relation between cocoon mass and provisioning mass is:
		* CfgLinear Cfg_OsmiaCocoonMassFromProvMass_Female = [1/3.247, 0]
		* cocoon_mass = provision *1/3.247
		* So we can calculate the combination of the two linear relationships to get female mass from provision mass by:
		* mass = 0.246381*provision_mass + 4.0
		*/
		sO.mass = m_OsmiaFemaleMassFromProvMassSlope * m_Mass + m_OsmiaFemaleMassFromProvMassConst;
		m_OurPopulationManager->CreateObjects(TTypeOfOsmiaLifeStages::to_OsmiaFemale, this, &sO, 1);
		#ifdef __OSMIATESTING
		m_OurPopulationManager->RecordInCocoonLength(m_Age - m_StageAge);
		#endif
	}

	KillThis(); // sets current state to -1 and StepDone to true;
	m_OurNest->RemoveCell(this);
	return toOsmias_Emerged; // This is just to have a return value, it is not used
}
//--------------------------------------------------------------------------------------------------------------------------------

bool Osmia_InCocoon::WinterMortality()
{
	/**
	* Osmia in cocoon is immobile and overwinters in the nest so only call this once at the end of overwintering
	* Overwintering mortality depends on pre-wintering degree-days accumulation, DDPrewinter
	* with a baseline temperature T0 = 15 C degrees, and only for days when Tavg – T0 >= 0
	*/
	//std::cout<<m_OsmiaInCocoonWinterMortSlope * m_DDPrewinter + m_OsmiaInCocoonWinterMortConst<<std::endl;
	if (g_random_fnc(100) < (m_OsmiaInCocoonWinterMortSlope * m_DDPrewinter + m_OsmiaInCocoonWinterMortConst)) return true;
	else return false;
}
//---------------------------------------------------------------------------

//********************************************************************************************************************************
//**************************************** Osmia_Female Definition ******************************************************************
//*******************************************************************************************************************************/

void Osmia_Female::ReInit(struct_Osmia* data)
{
	/** 
	* ReInit is used to enable the object pool, the method used to prevent many new/delete calls for objects that reuseable.
	*/
	Osmia_InCocoon::ReInit(data); 
	Init(data->mass);
	#ifdef __POLLINERA_TKTD
	resetExposure(); //reset the pesticide exposure
	#endif
}
//--------------------------------------------------------------------------------------------------------------------------------

Osmia_Female::Osmia_Female(struct_Osmia* data) : Osmia_InCocoon(data)
{
	/**
	* Constructor needs to initiate reproductive flags and set the number of eggs that can be produced. This is done by Init - which is shared with ReiInit
	*/
	ReInit(data);
}
//--------------------------------------------------------------------------------------------------------------------------------

// Shared per-thread scratch buffer for the transient "pesticide foraged this reproductive step" vector.
// It is only ever live within a single Osmia_Female::st_ReproductiveBehaviour() call (filled by Forage,
// consumed by LayEgg, both on the same thread), and is memset to 0 at the start of each use -- so it does
// NOT need to be a per-female heap allocation. thread_local makes it safe under the parallel Step loop and
// removes ~1M new[]/delete[] pairs + NoPPPs*8 bytes from every female's footprint (relevant at 1M bees).
static thread_local std::vector<double> s_foraged_resource_pesticide_scratch;

Osmia_Female::~Osmia_Female(void)
{
	// m_foraged_resource_pesticide points at the shared thread_local scratch -- do NOT delete it.
}
//--------------------------------------------------------------------------------------------------------------------------------

void Osmia_Female::Init(double a_mass)
{
	/**
	* Constructor needs to initiate reproductive flags and set the number of eggs that can be produced. This is done by Init - which is shared with ReiInit
	*/
	m_EmergeAge = 0; // Set the age to count adult days
	m_CellOpenDays = 0;
	m_ToDisperse = false;
	m_CurrentNestLoc.m_x = -1;
	m_ProvisioningTime = 9999;
	m_FlyingCounter = 0;
	m_OurNest = nullptr;
	m_Mass = a_mass;
	m_ForageLoc = false;
	m_ForageLocPoly = -1;
	m_ForageLocX = -1;
	m_ForageLocY = -1;
	m_NestProvisioningPlan = {};
	m_NestProvisioningPlanSex = {};
	m_NestOrder = 0;
	m_BeeSizeScore1 = int(floor((m_Mass - m_FemaleMinMass)/((m_FemaleMaxMass - m_FemaleMinMass) / 3.0)+0.5)); // Scores bee size into four classes
	if ((m_Mass < m_FemaleMinMass) || (m_Mass > m_FemaleMaxMass))
	{
		// Checks that our size classes make sense
		g_msg->Warn(WARN_BUG, "Osmia_Female::Init(double a_mass)  - mass out of range: ", int(a_mass));
		std::exit(TOP_Osmia); // Osmia exits return TOP_Osmia (10), in case anyone looks
	}
	const double mass_class_step = cfg_OsmiaAdultMassCategoryStep.value();
	m_BeeSizeScore2 = int(floor((m_Mass - m_FemaleMinMass) / mass_class_step + 0.5)); // Creates mass_class_step sized bee classes
	const int maximum_mass_class = int(floor((m_FemaleMaxMass - m_FemaleMinMass) / mass_class_step));
	if (m_BeeSizeScore2 > maximum_mass_class) m_BeeSizeScore2 = maximum_mass_class;
	CalculateEggLoad(); //
	// m_foraged_resource_pesticide is (re)pointed at the CURRENT thread's scratch at the start of each
	// st_ReproductiveBehaviour() (a bee may Step on a different thread than it was constructed on), so no
	// per-female allocation is needed here. Leave the pointer null until first use.
	m_foraged_resource_pesticide = nullptr;

#ifdef __OSMIARECORDFORAGE
	m_foragesum == 0;
	m_foragecount == 0;
#endif
#ifdef __OSMIATESTING
	m_firstnestflag = true;
	// Record the bee size
	m_OurPopulationManager->SetFemaleWeightLock();
	m_OurPopulationManager->RecordFemaleWeight(a_mass);
	m_OurPopulationManager->ReleaseFemaleWeightLock();
#endif // __OSMIATESTING
}
//--------------------------------------------------------------------------------------------------------------------------------

void Osmia_Female::BeginStep(void)
{
	/** 
	* The only function here is to record the current location for the density calucations.
	* These density estimates are not 100% accurate since the location may change later in the day, but with 1km should be reasonable.
	* The other fudge here is that if the landscape size is not divisible exactly by 1000 then the excess will be added into the last cells in rows and columns,
	* so if a landscape is made at 10999 m wide, the last row will actually have the bees from 1999x1000 area.
	*/
	//m_DensityIndex = m_OurPopulationManager->AddToDensityGrid(SupplyPoint());
	CheckManagement();

	if(l_pest_enable_pesticide_engine.value()){
		//threshold based mortaliy
		if(cfg_OsmiaFemaleThresholdBasedPesticideResponse.value()){
			//do decay
			doDecay(m_OsmiaPPPDecayRate);
			if(supplyTotalPesticide()>=m_OsmiaPPPThreshold){
				if(g_rand_uni_fnc() <= m_OsmiaPPPEffectProb){
					m_CurrentOState = toOsmias_Die;
					if(cfg_OsmiaStorePopulationDynamics.value()) m_OurPopulationManager->AddOneDeathPesticide(int(TTypeOfOsmiaLifeStages::to_OsmiaFemale), omp_get_thread_num());
				}		
			}
		}

		//Damage based mortality
		if(cfg_OsmiaFemaleDamageBasedPesticideResponse.value()){
			#ifdef __POLLINERA_TKTD
			updateBuffer();
			updateDamage();
			updateHazard();
			if(g_rand_uni_fnc() <= calMortalityBufferBased()){
				m_CurrentOState = toOsmias_Die;
				if(cfg_OsmiaStorePopulationDynamics.value()) m_OurPopulationManager->AddOneDeathPesticide(int(TTypeOfOsmiaLifeStages::to_OsmiaFemale), omp_get_thread_num());
			}
			#else
			if(g_rand_uni_fnc() <= doToxicity(m_OsmiaPPPDecayRate)){
				m_CurrentOState = toOsmias_Die;
				if(cfg_OsmiaStorePopulationDynamics.value()) m_OurPopulationManager->AddOneDeathPesticide(int(TTypeOfOsmiaLifeStages::to_OsmiaFemale), omp_get_thread_num());
			}
			#endif		
		}	
	}
}


bool Osmia_Female::OnFarmEvent( FarmToDo event ){
   bool result=false;
   #ifdef __OSMIA_PESTICIDE_STORE
   ofstream oversprayfile("osmia_overspray.txt", ios::app);
   #endif
   switch ( event )
   {
    case product_treat:
    	if(l_pest_enable_pesticide_engine.value()){
			result=true;
			doOverspray(m_OsmiaPPPOversprayBodySurface, m_OsmiaPPPOversprayChance, m_OsmiaPPPAbsorptionRateOverspray);
			break;
		}
	#ifdef __OSMIA_PESTICIDE_STORE
		oversprayfile << g_date->GetYear() << '\t' << g_date->DayInYear() << '\t' << m_animal_id << endl;
		break;
	#endif
		// With the pesticide engine disabled, product_treat has no biological effect.
		break;
    case insecticide_treat:
    case trial_insecticidetreat:
    case trial_toxiccontrol:
    case syninsecticide_treat:
	if(cfg_OsmiaFemaleBcackgroundPeticideResponse.value()){
        if (g_rand_uni_fnc() < cfg_OsmiaInsecticideApplicationMortality.value()){
			m_CurrentOState = toOsmias_Die;
			if(cfg_OsmiaStorePopulationDynamics.value()) m_OurPopulationManager->AddOneDeathPesticide(int(TTypeOfOsmiaLifeStages::to_OsmiaFemale), omp_get_thread_num());
			return true;
		}
	}
        break;
        // Biocide Treatment mortality
    case biocide:
	if(cfg_OsmiaFemaleBcackgroundPeticideResponse.value()){
        if (g_rand_uni_fnc() < (cfg_OsmiaInsecticideApplicationMortality.value()) * (1.0 - cfg_biocide_reduction_val.value())){
			m_CurrentOState = toOsmias_Die;
			if(cfg_OsmiaStorePopulationDynamics.value()) m_OurPopulationManager->AddOneDeathPesticide(int(TTypeOfOsmiaLifeStages::to_OsmiaFemale), omp_get_thread_num());
			return true;
		} 
	}
        break;
   }
   #ifdef __OSMIA_PESTICIDE_STORE
   oversprayfile.close();
   #endif
   return result;
}
//--------------------------------------------------------------------------------------------------------------------------------

void Osmia_Female::Step(void)
{
	/**
	* The Osmia female step code is the main behavioural control for the female Osmia. 
	* The main loop runs through Develop, which calls disperse. If dispersal is needed this is carried out before 
	* reproduction behaviour is called. Completion of this ends the step (each step is assumed to be one day).
	*/
	if (m_StepDone || m_CurrentStateNo == -1) return;
	switch (m_CurrentOState)
	{
	case toOsmias_InitialState: // Initial state always starts with develop, in this case it ages and determines the next action
		m_CurrentOState = toOsmias_Develop;
		break;
	case toOsmias_Develop:
		m_CurrentOState = st_Develop(); // returns toOsmias_ReproductiveBehaviour, toOsmias_Develop, or toOsmias_Die
		if (m_CurrentOState == toOsmias_Develop) m_StepDone = true;
		break;
	case toOsmias_Disperse:
		m_CurrentOState = st_Dispersal(); // Will return toOsmias_Develop
		break;
	case toOsmias_ReproductiveBehaviour:
		m_CurrentOState = st_ReproductiveBehaviour(); // Will return toOsmias_Disperse, toOsmias_Develop, or toOsmias_Die
		m_StepDone = true;
		break;
	case toOsmias_Die:
		st_Dying(); // No return value - no behaviour after this
		m_StepDone = true;
		if(cfg_OsmiaStorePopulationDynamics.value()) m_OurPopulationManager->AddOneDeath(int(TTypeOfOsmiaLifeStages::to_OsmiaFemale), omp_get_thread_num());
		break;
	default:
		m_OurLandscape->Warn("Osmia_Female::Step()", "unknown state - default");
		std::exit(TOP_Osmia);
	}
}
//--------------------------------------------------------------------------------------------------------------------------------

TTypeOfOsmiaState Osmia_Female::st_Develop(void)
{
	PHASE_TIMER(PH_OsmiaFemDevelop);
	/**
	* First we need to check for death causes. If not dead then either the bee is in dispersal or reproduction behaviour. 
	*/
	//if(GetTotalPest()>0) std::cout<<"Female: "<<m_Mass<<"/t"<<GetTotalPest()/m_Mass*1000<<"/t"<<supplyPestMortality()<<std::endl;
	if (g_rand_uni_fnc() < m_OsmiaFemaleBckMort) return toOsmias_Die;
	if(l_pest_enable_pesticide_engine.value()){
		// Pesticide
		if (g_rand_uni_fnc() < supplyPestMortality()) return toOsmias_Die;
	}
	// Here the days should be counted only if wind speed < 8 m/s and temp > 10 C degrees and not raining
	// as bad weather conditions do not allow for flying and feeding
		if (++m_EmergeAge > m_OsmiaFemaleLifespan) return toOsmias_Die; //Age limit
		if (m_OurPopulationManager->GetForageHours()>=1) {
			if (++m_FlyingCounter > m_OsmiaFemalePrenesting) return toOsmias_ReproductiveBehaviour; // pre-nesting time
	}
	return toOsmias_Develop;
}
//--------------------------------------------------------------------------------------------------------------------------------

bool Osmia_Female::FindNestLocation(void)
{
	PHASE_TIMER(PH_OsmiaFindNest);
	/**
	* Finds a suitable location (x,y) in a suitable polygon for placing a nest.
	* Check if we can make the nest here
	* If not makes a movement and check around.
	*
	* Initially we need to get the polygon index for our location, this index was copied to the nest manager.
	* This speeds up the process of getting locations to polygons for nests.
	*/
	int pindex = m_OurLandscape->SupplyPolyRefIndex(m_Location_x, m_Location_y);
	// PERF: the HasRoom() gate is BYTE-IDENTICAL to calling IsOsmiaNestPossible() directly. IsOsmiaNestPossible
	// is (current<max) && (rand<prob); C++ short-circuits &&, so on a FULL polygon it returns false WITHOUT
	// drawing the RNG. Guarding with HasRoom() (a cached bool == current<max) therefore skips only calls that
	// would have returned false without consuming any RNG -> nesting decisions AND the RNG stream are unchanged.
	bool found = m_OurPopulationManager->HasRoom(pindex) && m_OurPopulationManager->IsOsmiaNestPossible(pindex);
	m_ForageLoc = false; // Reset the forage location
	/**
	* SPIRAL search: systematic outward sweep from the bee's current location. One movedist is drawn as the
	* OUTER radius; rings are searched inner->outer at radius r*step, step = movedist / m_OsmiaFindNestAttemptNo,
	* so the bee settles in the NEAREST available nest. The 8 compass directions are probed per ring (random
	* rotation to avoid directional bias). This replaced the old fixed-centre random-radius sampler, which
	* scattered probes at bunched random radii on 8 spokes (finding far, often-full polygons); the spiral finds
	* near free nests on the first rings -> ~6x cheaper per search and nearer/higher nesting success.
	*/
	int movedist = int(m_OsmiaFemaleR50distance * m_generalmovementdistances.Get()); // outer radius, drawn ONCE
	int step = movedist / m_OsmiaFindNestAttemptNo;
	if (step < 1) step = 1;
	for (int r = 1; (!found) && r <= m_OsmiaFindNestAttemptNo; ++r)
	{
		int radius = r * step;
		unsigned dir = g_random_fnc(8); // random rotation of the 8-dir wheel per ring (avoids directional bias)
		for (int d = 0; d < 8; d++) {
			dir = (dir + d) & 7;
			int x = m_Location_x + g_vector_x[dir] * radius;
			int y = m_Location_y + g_vector_y[dir] * radius;
			m_OurLandscape->CorrectCoords(x, y); // For wrap around
			pindex = m_OurLandscape->SupplyPolyRefIndex(x, y);
			// Skip full polygons cheaply (no RNG); only draw the thinning RNG where there is actually room.
			if (!m_OurPopulationManager->HasRoom(pindex)) continue;
			if (m_OurPopulationManager->IsOsmiaNestPossible(pindex)) {
				m_Location_x = x;
				m_Location_y = y;
				m_OurPopulationManager->UpdateGuardMap(m_Location_x, m_Location_y, m_guard_cell_x, m_guard_cell_y);
				found = true;
				break;
			}
		}
	}
	if (found) {
		/**
		* We have found a current nest location so need to create the nest in the nest population manager and remember where it is, and how to access it
		*/
		m_CurrentNestLoc.m_x = m_Location_x;
		m_CurrentNestLoc.m_y = m_Location_y;
		m_OurNest = m_OurPopulationManager->CreateNest(m_Location_x, m_Location_y, pindex);
		m_CellOpenDays = 0;
		m_OurNest->m_owner = this;
		return true;
	}

	return false;
}
//--------------------------------------------------------------------------------------------------------------------------------

TTypeOfOsmiaState Osmia_Female::st_Dispersal(void)
{
	PHASE_TIMER(PH_OsmiaDispersal);
	/**
	* This is a single random direction jump
	*/
	int movedist = int(m_OsmiaFemaleR90distance * m_dispersalmovementdistances.Get());
	unsigned dir = g_random_fnc(8);
	int x = m_Location_x + g_vector_x[dir] * movedist;
	int y = m_Location_y + g_vector_y[dir] * movedist;
	m_OurLandscape->CorrectCoords(x, y); // For wrap around
	m_Location_x = x;
	m_Location_y = y;
	m_OurPopulationManager->UpdateGuardMap(m_Location_x, m_Location_y, m_guard_cell_x, m_guard_cell_y);
	m_EmergeAge++; // Cost of dispersal on lifespan
	return toOsmias_Develop;
}
//--------------------------------------------------------------------------------------------------------------------------------

double Osmia_Female::Forage(void)
{
	/**
	* The forage algorithm is very important in determining the efficiency of Osmia foraging and therefore the survival of the population in a patchy landscape.
		* The current implementation scans one or more square windows around the nest by calling
		* Landscape::SupplyLocMaxPollen; the pre-computed ring masks are not used.
		* Osmia_Female::m_ForageLoc signals whether a usable foraging location has been retained.
	* If we don't have a good place to forage then look around, otherwise use the one we have have until it is depleted or we start a new nest. Depletion is 
	* determined by Osmia_Female::m_pollengiveupreturn, which is set at a proportion (e.g. 75%) of the starting value when found.
	* We have a minimum acceptance level set by pollenfound2, initially set to 1.0. 
	* The whole mask is searched from a random location near m_Location_x m_Location_y and the best source accepted.
	* The search is from centre outwards so closer sources will be preferred if at the same level of resource.
	* Once found the value of the location is returned (scaled by a calibration factor: Osmia_Female::m_PollenScoreToMg ), and subject to a maximum
	* amount the bee can forage per hour in cfg_OsmiaMaxPollen .
	* This value is then multiplied by the number of forage hours and the efficiency of foraging at the bees current age.
	*/
#ifdef __CJT_OSMIAFIXEDRESOURCE
	double provisioning_mg = 40;
#else
	double pollenfound = 0.0;
	double pollenfound2 = 1.0;
	if (!m_ForageLoc) {
		//Get the forage mask(s) and search it(them), can be 1 or 2 or 4 masks with wrapping around
		auto temp_masks = m_OurLandscape->CalculateMaskCorners(m_CurrentNestLoc.m_x, m_CurrentNestLoc.m_y, cfg_OsmiaMaxHalfWidthForageMask.value());
		for(int i = 0; i<temp_masks.size();i++){
			pollenfound = m_OurLandscape->SupplyLocMaxPollen(m_ForageLocX, m_ForageLocY, temp_masks[i][0], temp_masks[i][1], temp_masks[i][2], temp_masks[i][3], cfg_OsmiaForageMaskStep.value());
			if(pollenfound>pollenfound2){
				pollenfound2 = pollenfound;
				m_ForageLoc = true;
				m_ForageLocPoly = m_OurLandscape->SupplyPolyRef(m_ForageLocX, m_ForageLocY);
			}
		}
		//cout<<"mask size: "<<temp_masks.size()<<" pollen found: "<<pollenfound2<<endl;
		m_currentpollenlevel = pollenfound2 * m_pollengiveupthreshold; // Will switch if the patch declines to this value
	}

	double  provisioning_mg = 0.0;
	if (m_ForageLoc) {
		provisioning_mg = m_OurLandscape->SupplyPollenQuantity(m_ForageLocPoly);
		if ((provisioning_mg < m_currentpollenlevel) || (provisioning_mg < m_pollengiveupreturn)) {
			m_ForageLoc = false;
			provisioning_mg = 0;
		}
		provisioning_mg *= m_PollenScoreToMg;
		if (provisioning_mg > m_OsmiaMaxPollen)
		{
			provisioning_mg = m_OsmiaMaxPollen;
		}
	}
#endif
	double forage_amount = provisioning_mg* m_FemaleForageEfficiency[m_EmergeAge];
	return forage_amount;
}

void Osmia_Female::GetPollenInPolygon(double& a_required_amount, double& a_foraged_amount, int a_polygon, int a_loc_x, int a_loc_y){
	PHASE_TIMER(PH_OsmiaPollenPoly);
	//Let's get enough pollen for the day
	double temp_pollen_quantity = m_OurLandscape->SupplyPollenQuantity(a_polygon);
	int temp_num_cells_required = ceil(a_required_amount / temp_pollen_quantity);
	//cout<<"temp_num_cells_required: "<<a_required_amount<<" "<<temp_pollen_quantity<<" "<<temp_num_cells_required<<endl;

	a_foraged_amount = a_required_amount;
	/*
	double available_pollen = 0.0;
	while(a_required_amount>0){
		available_pollen = m_OurLandscape->SupplyPollenQuantity(a_loc_x, a_loc_y);
		if(a_required_amount<=available_pollen){
			//if the pollen available is enough for the day, forage the pollen
			a_foraged_amount += a_required_amount;
			//reduce the pollen in the cell
			m_OurLandscape->RemovePollenInLoc(a_loc_x, a_loc_y, a_required_amount);
			a_required_amount = 0;
			break;
		}
		else{
			a_foraged_amount += available_pollen;
			m_OurLandscape->RemovePollenInLoc(a_loc_x, a_loc_y, available_pollen);
			a_required_amount -= available_pollen;
		}
	}*/
}

void Osmia_Female::DoPesticideContact(int a_x, int a_y){
	if(l_pest_enable_pesticide_engine.value()){
		doContact(m_OsmiaPPPContactBodySurface, m_OsmiaPPPAbsorptionRateContact, a_x, a_y);
	}
}

TTypeOfOsmiaState Osmia_Female::st_ReproductiveBehaviour(void)
{
	PHASE_TIMER(PH_OsmiaRepro);
	/**
	* Finds out how many foraging hours there are in the day.
	* This is based on wind, rain and insolation and is determined as an input.
	* If there is no time to forage skip this behaviour for today.
	* * There is a reduction of forage hours if there are more than 6 to allow for closing nest cells. This is calculated from
	* 3hrs 40 mins per cell provisioning, and 50mins closing = 240/(240+50)
	*/

	// Point the transient pesticide buffer at THIS thread's scratch (a bee may Step on a different thread
	// than it was constructed on). Sized once per thread. See s_foraged_resource_pesticide_scratch.
	if ((int)s_foraged_resource_pesticide_scratch.size() < g_pest_NoPPPs)
		s_foraged_resource_pesticide_scratch.resize(g_pest_NoPPPs);
	m_foraged_resource_pesticide = s_foraged_resource_pesticide_scratch.data();

	memset(m_foraged_resource_pesticide, 0, g_pest_NoPPPs * sizeof(double)); //reset the pesticide in the foraged resource be zero
	
	m_foragehours = m_OurPopulationManager->GetForageHours();
	if (m_foragehours < 1) return toOsmias_Develop;
	/**
	* Has some time to forage so if she does not have a nest then first she needs to find one.
	*/
	if (m_OurNest == nullptr)
	{
		if (!FindNestLocation()) return toOsmias_Disperse; // Failed try again tomorrow (if she lives that long)
		/** After a nest site is found she develops a new egg-number plan for that nest.
		 * PlanEggsPerNest() makes an independent calibrated draw for every nest. The draw is
		 * reduced by two eggs for each preceding nest to retain the calibrated nest-order decline.
		 */
		m_EggsThisNest = PlanEggsPerNest() - (2 * m_NestOrder);
		if (m_EggsThisNest < m_OsmiaFemaleMinEggsPerNest) m_EggsThisNest = m_OsmiaFemaleMinEggsPerNest;
		m_NestOrder++;
		m_CurrentProvisioning = 0;

		/**
		* We assume that in a given nest, the mass of cocoons in consecutive cells is decreasing with a fixed rate, higher among daughters and lower among sons.
		* We also assume that the difference between maximum and minimum female cocoon mass(the first and last cell with female offspring) is constant in all nests, and equals to 15 mg + / -5 mg:
		* female_total_mass_loss = 15*m_CocoonToProvisionMass + np.random.uniform(-5*m_CocoonToProvisionMass, 5*m_CocoonToProvisionMass)
		* To avoid calculating these everytime they are made static member variables
		* Number of female eggs in the nest is calculated based on sex ratio :
		* no_female = int(round(no_eggs_in_nest * sex_ratio))
		*/
		int no_female_eggs = int(floor(m_EggsThisNest * m_OurPopulationManager->GetSexRatioEggsAgeMass(m_BeeSizeScore2, m_EmergeAge) + 0.5));
		double female_step_prov_mass_loss = 0.0;
		if (no_female_eggs > 0) {
			female_step_prov_mass_loss = (m_TotalProvisioningMassLoss + (g_rand_uni_fnc() * m_TotalProvisioningMassLossRangeX2) - m_TotalProvisioningMassLossRange) / no_female_eggs;
		}
		/**
		* For each egg we need to assign and record the target provisioning mass.
		* Each female egg is reduced in mass by female_step_mass_loss after the first_female_cocoon_mass
		*
		*/
		double cocoon_prov_mass = m_OurPopulationManager->GetFirstCocoonProvisioningMass(m_EmergeAge, m_BeeSizeScore2);
		//double cocoon_prov_mass = m_OurPopulationManager->GetFirstCocoonProvisioningMass(3, 8);
		for (int egg = 0; egg < no_female_eggs; egg++)
		{
			m_NestProvisioningPlan.push_back(cocoon_prov_mass);
			m_NestProvisioningPlanSex.push_back(true);
#ifdef __OSMIATESTING
			if (m_firstnestflag)
			{
				m_target.m_cell_provision.push_back(cocoon_prov_mass);
			}
#endif // __OSMIATESTING
			cocoon_prov_mass -= female_step_prov_mass_loss;

		}
		cocoon_prov_mass = m_MaleMinTargetProvisionMass; 
		int no_male_eggs = m_EggsThisNest - no_female_eggs;
		for (int egg = 0; egg < no_male_eggs; egg++)
		{
			m_NestProvisioningPlan.push_back(cocoon_prov_mass); 
			m_NestProvisioningPlanSex.push_back(false);
#ifdef __OSMIATESTING
			if (m_firstnestflag)
			{
				m_target.m_cell_provision.push_back(m_MaleMinTargetProvisionMass);
			}
#endif // __OSMIATESTING
			cocoon_prov_mass -= female_step_prov_mass_loss; // using the same rate of mass loss as for the females
		}

#ifdef __OSMIATESTING
		m_OurPopulationManager->m_egghistogram[0][m_EggsThisNest]++;
		m_OurPopulationManager->m_egghistogram[1][no_female_eggs]++;
		if (m_firstnestflag)
		{
			m_target.m_no_eggs = m_EggsThisNest;
			m_target.m_no_females = no_female_eggs;
			m_achieved.m_no_eggs = 0;
			m_achieved.m_no_females = 0;
		}

#endif // __OSMIATESTING
		m_CellCarryOver = 0.0; // this helps us track partly created cells
	}
	// We reached here we must have a m_OurNest* value that is not NULL (set in FindNestLocation or already set)
	/**
	* Next the pollen availability near the nest is determined. This is the total pollen available within the nest area scaled to the maximum amount possible (0 to 1).\n
	* The pollen score is then multiplied by a fitting parameter cfg_PollenScoreToMg to obtain the pollen actually provisioned per day per bee. This value is stored as a static member m_PollenScoreToMg.
	*/
	/**
	* If the day has some flying hours then the bee is assumed to gather provisioning_mg pollen over these hours.
	* This is added to the provisioning for the egg.
	* There are two stopping rules for the egg provisioning - 1. max time per egg reached or 2. provisioning achieved.
	* There is also a test for giving up a nest location if pollen availability is too low.
	* If the first stopping rule is triggered then if there is enough pollen for a female egg then this is laid, otherwise a male egg is laid
	* and if so all the next eggs in the nest should be MALE!!!
	* If there is more pollen collected than needed. A new cell can be started.
	*/

	double provisioning_mg = Forage(); // This returns a value for the polygon, but needs adjusting for the number of hours available
#ifdef __OSMIARECORDFORAGE
	m_foragesum += provisioning_mg;
	m_foragecount++;
#endif
	/**
	* For the nest provsioning plan we follow a number of steps to test for and deal with deviations
	* First the number days the nest is open is recorded
	*/
	m_CellOpenDays++; // starts with 1 (set at zero when nest site found)
	/**
	* The possible foraging hours are used to calculate the maximum amount of pollen possible. This is based on the fact that it takes 3hrs 40 mins for a cell provisioning, and 50 mins to close it.
	* Hence the flying hours need to be reduced to get foraging time. by 220/(220+50) = 0.815
	*/
	//Get the possible pollen for the day
    double temp_pollen_can_forage = provisioning_mg*m_foragehours*0.815;
	//cout<<"temp_pollen_can_forage: "<<temp_pollen_can_forage<<endl;
	double temp_pollen_foraged = 0.0;
	double temp_pollen_foraged_total = 0.0;
	//check how many cells are needed to collect enough pollen
	//get a random cell from the foraging polygon
	if(temp_pollen_can_forage>0){
		PHASE_TIMER(PH_OsmiaForage);
		m_OurLandscape->SetPolygonLock(m_ForageLocPoly);
		APoint temp_start_loc;
		//GetPollenInPolygon(temp_pollen_can_forage, temp_pollen_foraged, m_ForageLocPoly, m_ForageLocX, m_ForageLocY);
		//Let's get the pollen from the foraging polygon
		//reset the pesticide in the foraged resource be zero
		memset(m_foraged_resource_pesticide, 0, g_pest_NoPPPs * sizeof(double));
		while(temp_pollen_can_forage>0){
			temp_start_loc = m_OurLandscape->SupplyARandomLocPollenPoly(m_ForageLocPoly);
			if(temp_start_loc.m_x<0 || temp_start_loc.m_y<0){
				m_ForageLoc = false;	
				break;
			}
			//cout<<"temp_start_loc: "<<temp_start_loc.m_x<<' '<<temp_start_loc.m_y <<endl;
			double temp_foraged_square = 0;
			temp_pollen_foraged = m_OurLandscape->SupplyPollenAtLocInPoly(m_ForageLocPoly, temp_start_loc, temp_pollen_can_forage, temp_foraged_square, m_foraged_resource_pesticide);


			//cout<<"temp_pollen_foraged: "<<temp_pollen_foraged<<" required: "<<temp_pollen_can_forage<<endl;
			if(temp_pollen_foraged<=0.0001 || m_OurLandscape->SupplyTotalPollen(m_ForageLocPoly)<0.0001) {
				m_ForageLoc = false;	
				break;
			} //get a new location
			//temp_pollen_foraged *= m_PollenCompetitionsReductionScaler;
			temp_pollen_can_forage -= temp_pollen_foraged;
			temp_pollen_foraged_total += temp_pollen_foraged;
		}

		//do the contact pesticide
		if(temp_start_loc.m_x>0 && temp_start_loc.m_y>0){
			DoPesticideContact(temp_start_loc.m_x, temp_start_loc.m_y);
		}

		//When the adults are foraging for pollen, they will eat nectar
		double nectar_quality = m_OurLandscape->SupplyNectar(m_ForageLocPoly).m_quality;
		if(l_pest_enable_pesticide_engine.value() && temp_start_loc.m_x>=0 && temp_start_loc.m_y>=0 && nectar_quality>0){
			//get the squares for the required amount of sugar
			double temp_squares_needed = m_OsmiaSugarPerDay / nectar_quality;

			//get the pesticide in the foraging polygon
			const int n_ppp = g_pest_NoPPPs;
			for(int pest_idx = 0; pest_idx<n_ppp; pest_idx++){
				double temp_pest = m_OurLandscape->SupplyPesticideNectar(m_ForageLocPoly, pest_idx) * temp_squares_needed;
				if(temp_pest > 0){
					addPesticide(temp_pest, pest_idx);
					#ifdef __POLLINERA_TKTD
					updateIntakeExposure(temp_pest/m_OsmiaSugarPerDay, pest_idx);
					#endif
				}
				#ifdef __OSMIA_PESTICIDE_STORE
				//contact
				double temp_pest_contact = m_OurLandscape->SupplyPesticidePlantSurface(m_ForageLocPoly, pest_idx);
				if(temp_pest_contact>0){
					ofstream contactfile("osmia_contact.txt", ios::app);
					contactfile << g_date->GetYear() << '\t' << g_date->DayInYear() << '\t' << m_animal_id << '\t' << temp_pest_contact  << endl;
					contactfile.close();
				}
				if(temp_pest>0){
					ofstream intakefile("osmia_pest_intake.txt", ios::app);
					intakefile << g_date->GetYear() << '\t' << g_date->DayInYear() << '\t' << m_animal_id << '\t' << temp_pest << '\t' << cfg_OsmiaSugarPerDay.value() << endl;
					intakefile.close();
				}
				#endif
			}
		}
		m_OurLandscape->ReleasePolygonLock(m_ForageLocPoly);
	}
	//if(temp_pollen_foraged_total>0) cout<<"temp_pollen_foraged: "<<temp_pollen_foraged<<endl;
	m_CurrentProvisioning += temp_pollen_foraged_total; // We have total possible pollen because we do not deplete it (so its OK to 'waste' it).
	m_CellCarryOver += m_foragehours;
	while ((m_NestProvisioningPlan.size() > 0) && ((m_CurrentProvisioning) > m_NestProvisioningPlan[0]) && (m_CellCarryOver >= 4.3))
	{
		// Make the egg and link the object to the nest
		LayEgg(); // will remove pollen from m_CurrentProvisioning
		m_CellCarryOver -= 4.3;
		m_CellOpenDays = 0;
		m_NestProvisioningPlan.pop_front();
		m_NestProvisioningPlanSex.pop_front();
		/** After an egg is laid, if there are no more then die */
		if (--m_EggsToLay < 1) {
			m_OurNest->CloseNest();
			m_OurNest = nullptr;
			break;
		}
		else
		{
			/** If there are more eggs to lay then the plan is checked - are we finished with this nest or not? */
			if (m_NestProvisioningPlan.size() < 1) {
				/** If yes finsished this one, start to look for the next tomorrow */
				m_OurNest->CloseNest();
				m_OurNest = nullptr;
				m_CellCarryOver = 0.0;
				m_CurrentProvisioning = 0.0;
				break;
			}
			else {
				m_CellOpenDays = 1; // Just in case we have spare pollen
			}
		}
	}
	if (m_CellOpenDays >= m_MaximumCellConstructionTime && m_OurNest!=nullptr)
	{
		// Hit the end of the road, decide on male or female or give up
		if (m_CurrentProvisioning >= m_MaleMinTargetProvisionMass)
		{
			LayEgg(); // Make a male or female egg depending on provisioning mass
			// Reset counters ready for the next egg
			m_CellOpenDays = 0;
			m_CellCarryOver = 0.0;
			m_CurrentProvisioning = 0.0;
			m_NestProvisioningPlan.pop_front();
			m_NestProvisioningPlanSex.pop_front();
			/** After an egg is laid, if there are no more then die */
			if (--m_EggsToLay < 1) {
				m_OurNest->CloseNest();
				m_OurNest = nullptr;
			}
			else
			{
				/** If there are more eggs to lay then the plan is checked - are we finished with this nest or not? */
				if (m_NestProvisioningPlan.size() < 1) {
					/** If yes finsished this one, start to look for the next tomorrow */
					// If the nest has no cells we can remove it now.
					m_OurNest->CloseNest();
					m_OurNest->SetCellLock();
					if (m_OurNest->GetNoCells() < 1) {
						m_OurNest->ReleaseCellLock();
						m_OurPopulationManager->ReleaseOsmiaNest(m_OurNest->GetPolyRef(), m_OurNest);
					}
					else {
						m_OurNest->ReleaseCellLock();
					}
					m_OurNest = nullptr;
				}
			}
		}
		else {
			// If the nest has no cells we can remove it now.
			m_OurNest->SetCellLock();
			if (m_OurNest->GetNoCells() < 1)
			{
				m_OurNest->ReleaseCellLock();
				m_OurPopulationManager->ReleaseOsmiaNest(m_OurNest->GetPolyRef(), m_OurNest);
			}
			else
			{
				m_OurNest->ReleaseCellLock();
			}
			// Give up the nest
			m_OurNest = nullptr;
		}
	}
#ifdef __OSMIATESTING
	if (m_OurNest == nullptr)
	{
		m_OurPopulationManager->m_egghistogram[2][m_achieved.m_no_eggs]++;
		m_OurPopulationManager->m_egghistogram[3][m_achieved.m_no_females]++;
		if (m_firstnestflag)
		{
			if (g_date->DayInYear() > May + 14) m_OurPopulationManager->WriteNestTestData(m_target, m_achieved);
			m_target.m_cell_provision.clear();
			m_achieved.m_cell_provision.clear();
			m_achieved.m_no_eggs = 0;
			m_achieved.m_no_females = 0;
			m_firstnestflag = false;
		}
	}
#endif // __OSMIATESTING
	if (m_EggsToLay < 1)
	{
		return toOsmias_Die;
	}
	return toOsmias_Develop;
}
//--------------------------------------------------------------------------------------------------------------------------------

void Osmia_Female::LayEgg()
{
	double provisionMass = m_CurrentProvisioning;
	/**
	* Creates an egg based on current pollen provisioning plan and dependent on the amount of pollen collected.
	*/
	struct_Osmia sO;
	if ((m_NestProvisioningPlanSex[0]) && (m_CurrentProvisioning > m_FemaleMinTargetProvisionMass))
	{
		/**
		* If there is enough pollen for a female cell, then the actual amount of pollen to be used is calculated.
		* The bee will first use the target amount, and then add an amount of extra pollen based on how much it has and a stochastic chance.
		*/
		if (m_CurrentProvisioning > m_NestProvisioningPlan[0]) {
			// We have reached the target
			if (m_CurrentProvisioning > m_FemaleMaxTargetProvisionMass) provisionMass = m_FemaleMaxTargetProvisionMass;
			provisionMass = m_NestProvisioningPlan[0] + (m_exp_ZeroToOne.Get() * (provisionMass - m_NestProvisioningPlan[0]));
		}
		// If we have not reached the target that is OK, we still have enough for a female cell
		sO.sex = true;
#ifdef __OSMIATESTING
		if (m_firstnestflag)
		{
			m_achieved.m_cell_provision.push_back(provisionMass);
			m_achieved.m_no_eggs++;
			m_achieved.m_no_females++;
		}
#endif
	}
	else
	{
		sO.sex = false;
		if (provisionMass > m_MaleMaxTargetProvisionMass) provisionMass = m_MaleMaxTargetProvisionMass;
		provisionMass = m_NestProvisioningPlan[0] + (m_exp_ZeroToOne.Get() * (provisionMass - m_NestProvisioningPlan[0]));

#ifdef __OSMIATESTING
		if (m_firstnestflag)
		{
			m_achieved.m_cell_provision.push_back(provisionMass);
			m_achieved.m_no_eggs++;
		}
#endif
		// for all the rest of the plan MinMaleProvisioningMass
		for (auto it = begin(m_NestProvisioningPlan); it != end(m_NestProvisioningPlan); ++it) {
			(*it) = m_MaleMinTargetProvisionMass;
		}
		for (auto it = begin(m_NestProvisioningPlanSex); it != end(m_NestProvisioningPlanSex); ++it) {
			(*it) = false;
		}
	}
	sO.mass = provisionMass;
	sO.parasitised = CalcParaistised(double(m_CellOpenDays));
	sO.OPM = m_OurPopulationManager;
	sO.L = m_OurLandscape;
	sO.age = 0;
	sO.x = m_CurrentNestLoc.m_x;
	sO.y = m_CurrentNestLoc.m_y;
	sO.nest = m_OurNest;

	if(l_pest_enable_pesticide_engine.value()){
		double temp_pest_to_egg = 0.0;
		double temp_pest_sum = 0.0;
		double ratio = provisionMass/m_CurrentProvisioning;
		const int n_ppp = g_pest_NoPPPs;
		const double zero_threshold = l_pest_zero_threshold_animal.value();
		for(int i=0; i<n_ppp; i++){
			temp_pest_to_egg = m_foraged_resource_pesticide[i]*ratio;
			//remove from the store
			m_foraged_resource_pesticide[i] *= (1-ratio);
			if(m_foraged_resource_pesticide[i] < zero_threshold){
				m_foraged_resource_pesticide[i] = 0;
			}
			temp_pest_sum += temp_pest_to_egg;
		}

		if(temp_pest_sum > m_OsmiaEggPPPThreshold){ //set the one time pesticide effect
			sO.pest_mortality = m_OsmiaEggPPPEffectProb;
			//std::cout<<"EGG: "<<temp_pest_sum<<std::endl;
		}
	}
	// Make the egg and link the object to the nest
	m_OurPopulationManager->CreateObjects(TTypeOfOsmiaLifeStages::to_OsmiaEgg, NULL, &sO, 1);
	// Correct todays pollen mass
	m_CurrentProvisioning -= provisionMass;
}
//--------------------------------------------------------------------------------------------------------------------------------
	/** \brief Determines the type of parasitoid if any */
TTypeOfOsmiaParasitoids Osmia_Female::CalcParaistised(double a_daysopen)
{
	TTypeOfOsmiaParasitoids::topara_Unparasitised;
	if (!m_UsingMechanisticParasitoids)
	{
		/**	
		* For statistical assumptions behind parastisation chance:\n
		* According to Seidelmann(2006), the risk of open - cell parasitism increases with the time the cell is open :\n
		* probability of cell being parasitised = 0.022 * time the cell is open[h] \n
		* We distinguished so far two modes of open - cell parasitism : \
		* (1) Cleptoparasitism by flies and wasps(e.g.C.indagator fly or Sapygid wasps) which consume the provision causing death of the larva or resulting in smaller larva depending on number of eggs laid in the host cell by the parasite.\n
		* (2) Open - cell parasitism by flies (e.g.A.Anthrax bombylid fly), resulting in death of the larva in the cell being parasitisedand all the larva / bees in cocoons on the way to the
		* nest entrance(destroyed by parasite pupa moving towards nest entrance) which happens in May - June in Poland.\n
		* Besides open - cell parasitism we could also add mite parasitism (causing death of bee larva in the cell being parasitised), which can be related to weather conditions - but not included currently.
		*/
		if (g_rand_uni_fnc() < (a_daysopen * (m_ParasitismProbToTimeCellOpen * 24)))
		{
			/* The egg is parasitised */
			if (g_rand_uni_fnc() < m_BombylidProbability) return TTypeOfOsmiaParasitoids::topara_Bombylid; else return TTypeOfOsmiaParasitoids::topara_Cleptoparasite;
		}
		else return TTypeOfOsmiaParasitoids::topara_Unparasitised;
	}
	else {
		/**
		* The chance of each type of parasitism depends on the number of parasitoids locally, plus may depend on how long the cell is open.
		* Each type of parasitoid has its own probability calculation.
		*/
		unsigned notypes = static_cast<int>(TTypeOfOsmiaParasitoids::topara_foobar) - 1;
		array<double, static_cast<unsigned>(TTypeOfOsmiaParasitoids::topara_foobar)>  parasitoid_densities = m_OurParasitoidPopulationManager->GetParasitoidNumbers(m_OurNest->Supply_m_Location_x(), m_OurNest->Supply_m_Location_y());
		for (unsigned ps = 0; ps < notypes-1; ps++) // iterate 1 less because the first type is unparasitised
		{
			// This is simply a linear relationship to start with
			double prob = parasitoid_densities[ps] * m_ParasitoidAttackChance[ps];
			if (g_rand_uni_fnc() < prob) return static_cast<TTypeOfOsmiaParasitoids>(ps);
		}
		return TTypeOfOsmiaParasitoids::topara_Unparasitised;
	}
}
//--------------------------------------------------------------------------------------------------------------------------------

void Osmia_Female::st_Dying(void)
{
	// this one is needed because all other stages free a nest space when they die, so this is different
	if (m_OurNest != nullptr)
	{
		m_OurNest->CloseNest();
		m_OurNest->SetCellLock();
		if (m_OurNest->GetNoCells() < 1)
		{
			m_OurNest->ReleaseCellLock();
			m_OurPopulationManager->ReleaseOsmiaNest(m_OurNest->GetPolyRef(), m_OurNest);
		}
		else{
			m_OurNest->ReleaseCellLock();
		}
		
	}
	KillThis(); // this will kill the animal object and free up space
}


int Osmia_Female::PlanEggsPerNest()
{
	/**
	 * Draws the planned egg number independently for each new nest. The BETA distribution and
	 * the additional two-egg shift applied with probability 0.45 are calibrated components of
	 * the nest-size model. Maternal size alters the upper scale through m_BeeSizeScore1. The
	 * caller applies the deterministic reduction of two eggs for each preceding nest and the
	 * configured minimum after this draw.
	 */
	int shift = 0;
	if (g_rand_uni_fnc() > 0.55) shift = 2;
	return shift + m_OsmiaFemaleMinEggsPerNest + int(floor((0.5 + m_OsmiaFemaleMaxEggsPerNest + m_BeeSizeScore1 - m_OsmiaFemaleMinEggsPerNest)* m_eggspernestdistribution.Get()));
}
//--------------------------------------------------------------------------------------------------------------------------------

Osmia_Nest::Osmia_Nest(int a_x, int a_y, int a_polyref, Osmia_Nest_Manager* a_manager) :TAnimalToxicity(a_x, a_y)
{
	m_x = a_x;
	m_y = a_y;
	m_PolyRef = a_polyref;
	m_cells.clear(); // Ensures there are no cells occupied before we start
	m_OurManager = a_manager;
	m_isOpen = true;
	m_owner = nullptr;
	m_aspectdelay = g_uni_0to35(g_generator);
	m_cell_lock = new omp_nest_lock_t;
	omp_init_nest_lock(m_cell_lock);
}
//--------------------------------------------------------------------------------------------------------------------------------

void Osmia_Nest::RemoveCell(TAnimal* a_oldpointer)
{
	SetCellLock();

	auto prev_point = m_cells.before_begin();
	for (auto it = m_cells.begin(); it!=m_cells.end(); ++it)
	{
		//cout<< a_oldpointer<<' '<<*it<<endl;
		//cout<<std::distance(std::begin(m_cells), std::end(m_cells))<<endl;
		if((*it) == a_oldpointer) 
		{
			m_cells.erase_after(prev_point);
			//cout<<"cell removed "<<endl;
			if (m_cells.empty())   // O(1) vs the old std::distance() full-list walk
			{
				// Need to remove the nest from the world
				//cout<<"nest needs removed "<<endl;
				//release the lock before we remove the nest
				ReleaseCellLock();
				m_OurManager->ReleaseOsmiaNest(m_PolyRef, this);
			}
			else
			{
				//cout<<"nest not removed "<<endl;
				ReleaseCellLock();
			}
			return;
		}

		prev_point=it;
	}

	ReleaseCellLock();
}

void Osmia_Nest::KillAllSubsequentCells(TAnimal* a_osmia)
{
	/**
	* Searches the nest until if finds a specific Osmia, once found it removes that Osmia and deletes all cells created higher up the tube.
	*/
	auto prev_point = m_cells.before_begin();
	for (auto it = m_cells.begin(); it!=m_cells.end(); ++it)
	{
		if(*it == a_osmia) 
		{
			m_cells.erase_after(prev_point, m_cells.end());
			if (std::distance(std::begin(m_cells), std::end(m_cells)) < 1)
			{
				// Need to remove the nest from the world
				m_OurManager->ReleaseOsmiaNest(m_PolyRef, this);
			}
			return;
		}
		prev_point=it;
	}
}

int Osmia_Nest::GetNoNests()
{
	return m_OurManager->GetNoNests(m_PolyRef);
}
