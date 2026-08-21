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
/** \file Osmia.h
\brief <B>The main source code for all Osmia life stage and population manager classes</B>
*/
/**  \file Osmia.h
Version of  2 August 2019 \n
By Chris J. Topping \n \n
*/

//---------------------------------------------------------------------------
#ifndef OsmiaH
#define OsmiaH
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#include "../BatchALMaSS/lock_profile.h"
#include "../BatchALMaSS/PesticideToxicity.h" // TAnimalToxicity (base of Osmia_Base / Osmia_Nest)
#include <forward_list>
#include <list>   // OsmiaPolygonEntry::m_NestList -- std::list gives O(1) erase via a stored iterator

class Osmia_Population_Manager;
class OsmiaParasitoid_Population_Manager;
class Osmia_Nest_Manager;
class PollenMap_centroidbased;
class probability_distribution;
class Osmia_Egg;
class Osmia_Female;
class struct_Osmia;
class Osmia_Base;

//------------------------------------------------------------------------------
/**
Used for the population manager's list of Osmia
*/
//typedef vector<Osmia*> TListOfOsmia;
//---------------------------------------------------------------------------

#define __OSMIA_DIST_SIZE 10000

/** \brief The assumed total mass loss first cocoon to last per nest in terms of female cocoon mass */
static CfgFloat cfg_OsmiaTotalCocoonMassLoss("OSMIATOTALCOCOONMASSLOSS", CFG_CUSTOM, 15.0);
/** \brief The range aroudn the assumed total mass loss first cocoon to last per nest in terms of female cocoon mass */
static CfgFloat cfg_OsmiaTotalCocoonMassLossRange("OSMIATOTALCOCOONMASSLOSSRANGE", CFG_CUSTOM, 5.0);

/**
\brief The possible behavioural states for Osmia classes 
*/
enum TTypeOfOsmiaState
{
	toOsmias_InitialState = 0,
	toOsmias_Develop,
	toOsmias_NextStage,
	toOsmias_Disperse,
	toOsmias_NestProvisioning,
	toOsmias_ReproductiveBehaviour,
	toOsmias_Emerged,
	toOsmias_Die
};



/**
\brief The types of parasitism possible for Osmia eggs/larvae
*/
enum class TTypeOfOsmiaParasitoids : unsigned // unsigned is used because this may be used as an index to an array
{
	topara_Unparasitised = 0, 
	topara_Bombylid,
	topara_Cleptoparasite,
	topara_foobar
};

/**
 * \class OsmiaNestData
 * \brief Testing-only record of egg number, female number and cell provision masses for one nest.
 *
 * <b>Implementation Approach:</b>
 * Instances are compiled only into the testing pathway in Osmia_Female and are written by
 * Osmia_Population_Manager::WriteNestTestData(). They do not control nesting behaviour.
 *
 * \see Osmia_Nest_Manager, OsmiaPolygonEntry
 */
class OsmiaNestData
{
public:
	int m_no_eggs;
	int m_no_females;
	vector<double> m_cell_provision;
};

/**
 * \class Osmia_Nest
 * \brief One nest cavity: a linear series of brood cells, provisioned and closed in order.
 *
 * <b>Biological Basis:</b>
 * <i>O. bicornis</i> nests in a linear cavity, provisioning and sealing one cell at a time from the
 * back forward. The order matters biologically as well as structurally: the cells completed last
 * stay open longest and so carry the greatest exposure to open-cell parasitism, which is the
 * gradient underlying the model's maternal investment and sex allocation rules.
 *
 * <b>Implementation Approach:</b>
 * Holds the cell contents as a list of TAnimal pointers, added at the front as construction
 * proceeds, together with the polygon reference and open/closed state. Each nest carries its own
 * OpenMP lock, which is the model's principal concurrency mechanism: females on different threads
 * may contend for the same nest.
 *
 * <b>Key Assumptions:</b>
 * - Cavities are interchangeable except in their occupancy state; diameter and depth are not
 *   represented, so cell number is limited by the female's plan rather than by the cavity.
 *
 * \see Osmia_Nest_Manager, Osmia_Female, OsmiaPolygonEntry
 * \par References:
 * - Seidelmann, K. (2006) Open-cell parasitism shapes maternal investment patterns in the Red Mason
 *   bee <i>Osmia rufa</i>. <i>Behavioral Ecology</i> 17(5): 839-848.
 *   https://doi.org/10.1093/beheco/arl017
 * - Ziolkowska, E., Bednarska, A.J., Laskowski, R. & Topping, C.J. (2023) The Formal Model for the
 *   solitary bee <i>Osmia bicornis</i> L. agent-based model. <i>Food and Ecological Systems
 *   Modelling Journal</i> 4: e102102. https://doi.org/10.3897/fmj.4.102102
 */
class Osmia_Nest : public TAnimalToxicity
{
	/**
	* The Osmia_Nest class contains a list of Osmia_Egg pointers to objects it contains.
	* It is descended from TAnimal which means it has a location and has access to the TALMaSSObject Step code if needed..however, right now this is not implemented in the population manager.
	* All information about the egg state is held by the egg itself e.g. sex, parasitoids, so this is simply a list of eggs in order of them being added to the nest
	* The nest can however supply the number of current eggs it holds
	*/
protected:
	/** \brief x-location */
	int m_x;
	/** \brief y-location */
	int m_y;
	/** \brief polygon reference to where the nest is located */
	int m_PolyRef;
	/** \brief list of egg objects */
	std::forward_list<TAnimal*>m_cells;
	/** \brief The lock to for egg cells operation.*/
	omp_nest_lock_t* m_cell_lock;
	/** \brief A pointer to the one and only nest population manager */
	static Osmia_Nest_Manager* m_OurManager;
	/** \brief Signals that the nest is closed or open for adding new cells */
	bool m_isOpen;
	/** \brief Simulates the natural variation assumed per nest location based on aspect, exposure etc.. */
	int m_aspectdelay;
public:
	/** \brief This nest's own position in its polygon's OsmiaPolygonEntry::m_NestList.
	 * Stored when the nest is added (IncOsmiaNesting) so that removal (ReleaseOsmiaNest) is O(1)
	 * list::erase instead of an O(n) linear search of the polygon's whole nest list. That search used to
	 * run while holding the per-polygon nest lock, making release cost O(n) under contention -- measured as
	 * ~48% of all core time in a 30-year run. std::list iterators stay valid across other insert/erase, so
	 * the handle remains correct for the nest's whole lifetime. */
	std::list<Osmia_Nest*>::iterator m_nest_handle;
	/** \brief True once m_nest_handle has been set (guards against erasing with an unset handle). */
	bool m_nest_handle_set = false;
	/**
	 * @brief Creates an empty, open nest at a landscape location.
	 * @param a_x Landscape x-coordinate in metres.
	 * @param a_y Landscape y-coordinate in metres.
	 * @param a_polyref Polygon index used by the nest manager.
	 * @param a_manager Manager that owns and ultimately releases the nest.
	 */
	Osmia_Nest(int a_x, int a_y, int a_polyref, Osmia_Nest_Manager* a_manager);
	/** @brief Destroys the per-nest OpenMP lock; brood objects are owned by the population manager. */
	virtual ~Osmia_Nest(){
		omp_destroy_nest_lock(m_cell_lock);
		delete m_cell_lock;
	}
	/** \brief Set the cell lock.*/
	void SetCellLock(void) { LP_WAIT(LPK_NestCell); omp_set_nest_lock(m_cell_lock); }
	/** \brief Release the cell lock.*/
	void ReleaseCellLock(void) { omp_unset_nest_lock(m_cell_lock); }
	/** \brief Adds a cocoon to the nest, this is only used to initialise the simulation.*/
	void AddCocoon (TAnimal* a_cocoon) {
		m_cells.push_front(a_cocoon);
	}
	/** \brief Adds an egg to the nest */
	void AddEgg(TAnimal* a_egg) { m_cells.push_front(a_egg); }
	/**
	 * @brief Replaces a brood-stage pointer without changing its position in the linear nest.
	 * @param a_oldpointer Pointer to the life-stage object being replaced.
	 * @param a_newpointer Pointer to the successor life-stage object.
	 */
	void ReplaceNestPointer(TAnimal* a_oldpointer, TAnimal* a_newpointer) {
		std::replace(m_cells.begin(), m_cells.end(), a_oldpointer, a_newpointer); // replaces the old life stage pointer with the new one in-place
	}
	/**
	 * @brief Removes a brood object from the nest and releases the nest if it becomes empty.
	 * @param a_oldpointer Pointer to the brood object to remove.
	 * @note The method acquires and releases the nest cell lock internally.
	 */
	void RemoveCell(TAnimal* a_oldpointer);
	/** \brief Debug function - Is this osmia present? */
	bool Find(TAnimal* a_osmia)
	{
		if (std::find(m_cells.begin(), m_cells.end(), a_osmia) != m_cells.end()) return true;
		else {
			return false;
		}
	}
	/** \brief Removes all Osmia from the nest from a target to the end of the tube */
	void KillAllSubsequentCells(TAnimal* a_osmia);
	/** \brief Get the polyref */
	int GetPolyRef() { return m_PolyRef; }
	/** \brief For debug - get the number of nests currently for this polygon */
	int GetNoNests();
	/** \brief Get the number of cells for this nest */
	int GetNoCells() { return std::distance(std::begin(m_cells), std::end(m_cells)); };
	/**
	 * @brief Returns true when the nest contains at least one cell.
	 * @warning The method name suggests the opposite result. It is used only by the diagnostic
	 *          Osmia_Nest_Manager::CheckZeroNests().
	 */
	bool ZeroCells() {
		if (std::distance(std::begin(m_cells), std::end(m_cells)) < 1) return false; else return true;
	}
	/** @brief Returns the nest-specific emergence delay, in days. */
	int GetAspectDelay() { return m_aspectdelay; }
	/** \brief Tells us whether the nest is finished for additions = false or can be added too = true */
	bool GetIsOpen() { return m_isOpen; }
	/** \brief Signals that the nest is closed */
	void CloseNest() { m_isOpen = false; }
	// Debug
	Osmia_Female* m_owner;
};

/**
 * \class Osmia_Base
 * \brief Common state, static species parameters and the shared interface for every
 *        <i>Osmia bicornis</i> life stage.
 *
 * Osmia_Base is the root of the life-stage chain and holds everything that does not vary between
 * individuals: the degree-day thresholds and sums, mortality rates, mass relationships and
 * behavioural constants that define the species. These are declared \c static, so one copy is
 * shared by the whole population and a configuration change applies to every individual at once.
 * Per-individual state - age, mass, location, parasitism status, nest - is held per object.
 *
 * <b>Biological Basis:</b>
 * The model represents <i>O. bicornis</i> as a univoltine solitary bee whose development from egg
 * to adult is temperature-driven and whose adult phase is resource-limited. Separating species
 * constants from individual state mirrors the biological distinction between what is true of the
 * species and what is true of one bee.
 *
 * <b>Implementation Approach:</b>
 * Derives from TAnimalToxicity, which supplies the ALMaSS object lifecycle, spatial position and
 * pesticide body burden. BeginStep(), Step() and EndStep() are deliberately empty here: Osmia_Base
 * is never instantiated directly and each life stage supplies its own. Static parameters are loaded
 * once by SetParameterValues() from the configuration variables declared at the top of Osmia.cpp.
 * SetTemp() is called once per day by Osmia_Population_Manager::DoFirst before any individual
 * steps, which is what makes a static daily temperature safe.
 *
 * <b>Key Assumptions:</b>
 * - Individuals of a stage are identical except in their state variables.
 * - All individuals experience the same daily mean temperature; no microclimate is represented.
 * - Species parameters are constant within a simulation run.
 *
 * <b>Limitations:</b>
 * - The life stages form an inheritance <b>chain</b> - Osmia_Egg derives from Osmia_Base and each
 *   subsequent stage from its predecessor - so the hierarchy encodes the ontogenetic sequence
 *   rather than a taxonomy of stage types. Osmia_Female is therefore, in C++ terms, a descendant of
 *   Osmia_Egg. Later stages carry members meaningful only earlier, and a reused member may change
 *   meaning between stages: m_AgeDegrees holds degree-days in the egg, larval and pupal stages,
 *   normalised development days in Osmia_Prepupa, and overwintering degree-days in Osmia_InCocoon.
 * - Because the species parameters are static, two populations with different parameterisations
 *   cannot be run in the same process.
 *
 * \see Osmia_Egg, Osmia_Female, Osmia_Population_Manager
 * \par References:
 * - Ziolkowska, E., Bednarska, A.J., Laskowski, R. & Topping, C.J. (2023) The Formal Model for the
 *   solitary bee <i>Osmia bicornis</i> L. agent-based model. <i>Food and Ecological Systems
 *   Modelling Journal</i> 4: e102102. https://doi.org/10.3897/fmj.4.102102
 * - Topping, C.J., Hansen, T.S., Jensen, T.S., Jepsen, J.U., Nikolajsen, F. & Odderskaer, P. (2003)
 *   ALMaSS, an agent-based model for animals in temperate European landscapes. <i>Ecological
 *   Modelling</i> 167(1-2): 65-82. https://doi.org/10.1016/S0304-3800(03)00173-X
 */
class Osmia_Base : public TAnimalToxicity
{
	/**
	A Osmia must have some simple functionality:
	Inititation and development
	And some simple characteristics, herein age.
	Inherits m_Location_x, m_Location_y, m_OurLandscape from TAnimal
	NB All areas are squares of size length X length
	*/

protected:
	/** \brief Variable to record current behavioural state */
	TTypeOfOsmiaState m_CurrentOState;
	/** \brief A typical member variable - this one is the age in days */
	int m_Age;
	/** \brief This is a time saving pointer to the correct population manager object */
	Osmia_Population_Manager* m_OurPopulationManager;
	/** \brief This is a time saving pointer to the parasitoid population manager object */
	static OsmiaParasitoid_Population_Manager* m_OurParasitoidPopulationManager;
	/** \brief The temperature today. This is static because there is only one temperature today. */
	static double m_TempToday;
	/** \brief The temperature today to the nearest degree. This is static because there is only one temperature today. This is mostly for use in temperature indexed arrays (if used) */
	static int m_TempTodayInt;
	/** \brief This holds the daily mortality for eggs */
	static double m_DailyDevelopmentMortEggs;
	/** \brief This holds the daily mortality for larvae */
	static double m_DailyDevelopmentMortLarvae;
	/** \brief This holds the daily mortality for pre-pupae */
	static double m_DailyDevelopmentMortPrepupae;
	/** \brief This holds the daily mortality for pupae */
	static double  m_DailyDevelopmentMortPupae;
	/** \brief Is the number of day degrees needed for egg development to hatch */
	static double m_OsmiaEggDevelTotalDD;
	/** \brief Is temperature developmental threshold for egg development */
	static double m_OsmiaEggDevelThreshold;
	/** \brief Is the number of day degrees needed for larval hatch above the developmental threshold for larvae */
	static double m_OsmiaLarvaDevelTotalDD;
	/** \brief Is temperature developmental threshold for larval development */
	static double m_OsmiaLarvaDevelThreshold;
	/** \brief Is the number of day degrees needed for pupal hatch above the developmental threshold for pupae */
	static double m_OsmiaPupaDevelTotalDD;
	/** \brief Is temperature developmental threshold for pupal development */
	static double m_OsmiaPupaDevelThreshold;
	/** \brief Number of days for prepupal development */
	static double m_OsmiaPrepupalDevelTotalDays;
	/** \brief 10% of the number of days for prepupal development - just for speed */
	static double m_OsmiaPrepupalDevelTotalDays10pct;
	/** \brief holds the value for the InCocoon overwintering temperature threshold */
	static double m_OsmiaInCocoonOverwinteringTempThreshold;
	/** \brief holds the value for the InCocoon emergence temperature threshold */
	static double m_OsmiaInCocoonEmergenceTempThreshold;
	/** \brief holds the value for the InCocoon prewintering temperature threshold */
	static double m_OsmiaInCocoonPrewinteringTempThreshold;
	/** \brief holds the constant term value for the InCocoon winter mortality calculation */
	static double m_OsmiaInCocoonWinterMortConst;
	/** \brief holds the coefficient value for the InCocoon winter mortality calculation */
	static double m_OsmiaInCocoonWinterMortSlope;
	/** \brief holds the constant term value for the InCocoon emergence counter calculation */
	static double m_OsmiaInCocoonEmergCountConst;
	/** \brief holds the coefficient value for the InCocoon emergence counter calculation */
	static double m_OsmiaInCocoonEmergCountSlope;
	/** \brief holds the constant term value for the female mass calculation from provision mass */
	static double m_OsmiaFemaleMassFromProvMassConst;
	/** \brief holds the coefficient value for the female mass calculation from provision mass */
	static double m_OsmiaFemaleMassFromProvMassSlope;
	/** \brief The minimum target provisioning for a male cell */
	static double m_MaleMinTargetProvisionMass;
	/** \brief The maximum target provisioning for a male cell */
	static double m_MaleMaxTargetProvisionMass;
	/** \brief The minimum target provisioning for a female cell */
	static double m_FemaleMinTargetProvisionMass;
	/** \brief The maximum target provisioning for a female cell */
	static double m_FemaleMaxTargetProvisionMass;
	/** \brief The maximum female mass */
	static double m_FemaleMaxMass;
	/** \brief The minimum female mass */
	static double m_FemaleMinMass;
	/** \brief The maximum male mass */
	static double m_MaleMaxMass;
	/** \brief The conversion rate from pollen availability score to mg pollen provisioned per day */
	static double m_PollenScoreToMg;
	/** \brief A parameter to link linear reduction in pollen availability to Osmia numbers per 1km2 */
	static double m_DensityDependentPollenRemovalConst;
	/** \brief The shortest possible construction time for a cell - normally 1 day */
	static double m_MinimumCellConstructionTime;
	/** \brief The longest possible construction time for a cell */
	static double m_MaximumCellConstructionTime;
	/** \brief The maximum number of nests possible for a bee */
	static int m_TotalNestsPossible;
	/** \brief holds the probability of bombylid fly parasitism if open nest parasitoid */
	static double m_BombylidProbability;
	/** \brief holds the ratio of open cell parasitism to time cell is open */
	static double m_ParasitismProbToTimeCellOpen;
	/** \brief Holds the probability per capita of parasitoid attack for a subpopulation sized cell */
	static vector<double> m_ParasitoidAttackChance;
	/** \brief holds the value for the female typical homing distance */
	static double m_OsmiaFemaleR50distance;
	/** \brief holds the value for the female max homing distance */
	static double m_OsmiaFemaleR90distance;
	/** \brief holds the value for the duration of prenesting */
	static int m_OsmiaFemalePrenesting;
	/** \brief holds the value for max female lifespan */
	static int m_OsmiaFemaleLifespan;
	/** \brief Static instance of the probability_distribution class of distance probablilties for nest searching and foraging */
	static probability_distribution  m_generalmovementdistances;
	/** \brief Static instance of the probability_distribution class of distance probablilties for dispersal */
	static probability_distribution  m_dispersalmovementdistances;
	/** \brief Static instance of the probability_distribution class of distance probablilties for numbers of eggs per nest */
	static probability_distribution  m_eggspernestdistribution;
	/** \brief Static instance of the probability_distribution class of distance probablilties for approx exponential probs zero to 1.0 */
	static probability_distribution  m_exp_ZeroToOne;
	/** \brief The ratio of cocoon to provision mass */
	static double m_CocoonToProvisionMass;
	/** \brief The ratio of provison to cocoon mass */
	static double m_ProvisionToCocoonMass;
	/** \brief The total provision mass loss expected first cocoon to last */
	static double m_TotalProvisioningMassLoss;
	/** \brief The variability around the expected total mass loss */
	static double m_TotalProvisioningMassLossRange;
	/** \brief Twice the variability around the expected total mass loss */
	static double m_TotalProvisioningMassLossRangeX2;
	/** \brief Flag for switching between ways of calculating parasitism */
	static bool m_UsingMechanisticParasitoids;
	/** \brief Holds the parameter for unspecified female daily mortality */
	static double m_OsmiaFemaleBckMort;
	/** \brief Holds the parameter for minimum planned eggs per nest */
	static int m_OsmiaFemaleMinEggsPerNest;
	/** \brief Holds the parameter for number of find nest tries */
	static int m_OsmiaFindNestAttemptNo;
	/** \brief Holds the parameter for maximum planned eggs per nest */
	static int m_OsmiaFemaleMaxEggsPerNest;
	/** \brief Static instance of the probability_distribution class of emergence day */
	static probability_distribution m_emergenceday;
	/** \brief Holds the parasitoid status - only one is allowed */
	TTypeOfOsmiaParasitoids m_ParasitoidStatus;
	/** \brief A useful pointer to the current nest with double use, for up to InCocoon its the nest where they are, for females its the current being provisioned */
	Osmia_Nest* m_OurNest;
	/** \brief An attribute to record the mass of the bee or provisioned pollen depending on life-stage */
	double m_Mass;
	/** \brief A variable to hold the number of available forage hours left in a day */
	int m_foragehours;

public:
	/** \brief Returns the configured max female lifespan (OSMIA_LIFESPAN) */
	/** @brief Returns the configured maximum adult female lifespan in days. */
	static int GetFemaleLifespan() { return m_OsmiaFemaleLifespan; }
	/** \brief Osmia constructor */
	Osmia_Base(struct_Osmia* data);
	/** \brief Osmia reinitialise object methods */
	void ReInit(struct_Osmia* data);
	/** \brief Osmia destructor */
	virtual ~Osmia_Base();
	/** \brief Behavioural state dying */
	virtual void st_Dying(void);
	/** \brief The BeginStep is the first 'part' of the timestep that an animal can behave in. It is called once per timestep. */
	virtual void BeginStep(void) { ; } // NB this is not used in the Osmia_Base code
	/** \brief The Step is the second 'part' of the timestep that an animal can behave in. It is called continuously until all animals report that they are 'DONE'. */
	virtual void Step(void) { ; } // NB this is not used in the Osmia_Base code
	/** \brief The EndStep is the third 'part' of the timestep that an animal can behave in. It is called once per timestep. */
	virtual void EndStep(void) { ; } // NB this is not used in the Osmia_Base code
	/** \brief A typical interface function - this one returns the age */
	int GetAge() { return m_Age; }
	/** \brief A typical interface function - this one sets the age */
	void SetAge(int a_age) { m_Age = a_age; }
	/** \brief Returns the bee's mass */
	double GetMass() { return m_Mass; }
	/** \brief Sets the bee's mass */
	void SetMass(double a_mass) { m_Mass = a_mass; }
	/** \brief Set the parasitised status */
	void SetParasitised(TTypeOfOsmiaParasitoids a_status) { 
		m_ParasitoidStatus = a_status; 
	}
	/** \brief Set the parasitised status */
	TTypeOfOsmiaParasitoids GetParasitised( void ) { return m_ParasitoidStatus; }
	/** @brief Returns the nest associated with this life stage, or nullptr for a female without a current nest. */
	Osmia_Nest* GetNest() { return m_OurNest; }
	/** \brief Used to populate the static members holding mortality and development parameters */
	static void SetParameterValues();
			 /* Not currently using temperature related mortality
		 void SetNestMortality(double a_MortsByTemp[3][80]) {
			for (int i = 0; i < 80; i++) {
			m_DailyDevelopmentMortEggToCocoon[i] = a_MortsByTemp[0][i];
			m_DailyDevelopmentMortPupa[i] = a_MortsByTemp[1][i];
			m_DailyDevelopmentMortOverWinter[i] = a_MortsByTemp[2][i];
			*/
	static void SetTemp(double a_temperature) { 
		m_TempToday = a_temperature;
		m_TempTodayInt = int(floor(a_temperature + 0.5)); 
	}
	/** Set method for m_OsmiaParasitoid_Population_Manager */
	static void SetParasitoidManager(OsmiaParasitoid_Population_Manager* a_popman) 
	{ 
		m_OurParasitoidPopulationManager = a_popman; 
	}
};

/**
 * \class Osmia_Egg
 * \brief The egg stage: temperature-driven development from laying to hatching.
 *
 * <b>Biological Basis:</b>
 * Development in <i>O. bicornis</i> is temperature-dependent and is represented, following the
 * Formal Model, as linear degree-day accumulation above a lower developmental threshold. Laboratory
 * rearing at controlled temperatures underpins the stage durations.
 *
 * <b>Implementation Approach:</b>
 * st_Develop() adds max(0, T - @c cfg_OsmiaEggDevelThreshold) to m_AgeDegrees each day and signals the
 * transition once @c cfg_OsmiaEggDevelTotalDD is reached. DailyMortality() applies a constant daily
 * probability, independent of temperature.
 *
 * <b>Key Assumptions:</b>
 * - Development rate is linear above the threshold and zero below it; a non-linear response was not
 *   adopted in the current implementation.
 * - Mortality is constant across the stage, and independent of temperature and condition.
 *
 * <b>Limitations:</b>
 * - The threshold and the degree-day sum are <b>calibrated, not measured</b>, and are jointly
 *   identifiable rather than separately determined. The pair must be changed together, and neither
 *   value should be interpreted on its own.
 * - The calibrated pair departs substantially from the Formal Model's a priori values.
 *
 * \see Osmia_Base, Osmia_Larva
 * \par References:
 * - Radmacher, S. & Strohm, E. (2011) Effects of constant and fluctuating temperatures on the
 *   development of the solitary bee <i>Osmia bicornis</i>. <i>Apidologie</i> 42(6): 711-720.
 *   https://doi.org/10.1007/s13592-011-0078-9
 * - Giejdasz, K. & Wilkaniec, Z. (2002) Individual development of the red mason bee <i>Osmia rufa</i>
 *   L. (Megachilidae) under natural and laboratory conditions.
 * - Ziolkowska, E., Bednarska, A.J., Laskowski, R. & Topping, C.J. (2023) The Formal Model for the
 *   solitary bee <i>Osmia bicornis</i> L. agent-based model. <i>Food and Ecological Systems
 *   Modelling Journal</i> 4: e102102. https://doi.org/10.3897/fmj.4.102102
 */
class Osmia_Egg : public Osmia_Base
{
protected:
	/** \brief This contains the age in degrees for development */
	double m_AgeDegrees = 0.0;
	/** \brief Holds the sex of the egg. Female = fertilized = true */
	bool m_Sex;
	/** \brief Holds the age when the stage was initiated */
	int m_StageAge;
	/** \brief Holds the mortality caused by pesticide*/
	double m_egg_pest_mortality;
public:
	/** @brief Constructs an egg and initialises its sex, nest, mass and pesticide mortality state. */
	Osmia_Egg(struct_Osmia* data);
	/** \brief Osmia_Egg ReInit for object pool */
	virtual void ReInit(struct_Osmia* data);
	/** \brief Osmia_Egg destructor */
	virtual ~Osmia_Egg();
	/** \brief The BeginStep is the first 'part' of the timestep that an animal can behave in. It is called once per timestep. */
	virtual void BeginStep(void); // NB this is used for egg pesticide death.
	/** \brief The Step is the second 'part' of the timestep that an animal can behave in. It is called continuously until all animals report that they are 'DONE'. */
	virtual void Step(void);
	/** \brief A typical interface function - this one returns the agedegrees */
	double GetAgeDegrees() { return m_AgeDegrees; }
	/** \brief A typical interface function - this one sets the agedegrees */
	void SetAgeDegrees(unsigned a_agedegrees) { m_AgeDegrees = a_agedegrees; }
protected:
	/** @brief Applies daily egg mortality and degree-day development. */
	virtual TTypeOfOsmiaState st_Develop(void);
	/** \brief Behavioural state hatch */
	virtual TTypeOfOsmiaState st_Hatch(void);
	/** \brief Daily mortality test for eggs */
	virtual bool DailyMortality() { if (g_rand_uni_fnc() < m_DailyDevelopmentMortEggs) return true; else return false; }
};

/**
 * \class Osmia_Larva
 * \brief The larval stage: feeding on the pollen provision and spinning the cocoon.
 *
 * <b>Biological Basis:</b>
 * The larva consumes the pollen and nectar provision left by its mother and spins a cocoon at the
 * end of the stage. Development is temperature-driven, as for the egg.
 *
 * <b>Implementation Approach:</b>
 * Identical in form to Osmia_Egg - degree-day accumulation above @c cfg_OsmiaLarvaDevelThreshold
 * toward @c cfg_OsmiaLarvaDevelTotalDD - with st_Prepupate() signalling the transition. Derives from
 * Osmia_Egg rather than from Osmia_Base: see the note on the inheritance chain in Osmia_Base.
 *
 * <b>Key Assumptions:</b>
 * - Development rate is linear above the threshold and zero below it; a non-linear response was not
 *   adopted in the current implementation.
 * - Mortality is constant across the stage, and independent of temperature and condition.
 *
 * <b>Limitations:</b>
 * - The threshold and the degree-day sum are <b>calibrated, not measured</b>, and are jointly
 *   identifiable rather than separately determined. The pair must be changed together, and neither
 *   value should be interpreted on its own.
 * - The calibrated pair departs substantially from the Formal Model's a priori values.
 *
 * \note The provision mass a larva received is not re-read here. It has already been converted to
 *       the eventual adult mass at cell closure, so larval growth is not modelled explicitly.
 *
 * \see Osmia_Egg, Osmia_Prepupa
 * \par References:
 * - Radmacher, S. & Strohm, E. (2011) Effects of constant and fluctuating temperatures on the
 *   development of the solitary bee <i>Osmia bicornis</i>. <i>Apidologie</i> 42(6): 711-720.
 *   https://doi.org/10.1007/s13592-011-0078-9
 * - Giejdasz, K. & Wilkaniec, Z. (2002) Individual development of the red mason bee <i>Osmia rufa</i>
 *   L. (Megachilidae) under natural and laboratory conditions.
 * - Ziolkowska, E., Bednarska, A.J., Laskowski, R. & Topping, C.J. (2023) The Formal Model for the
 *   solitary bee <i>Osmia bicornis</i> L. agent-based model. <i>Food and Ecological Systems
 *   Modelling Journal</i> 4: e102102. https://doi.org/10.3897/fmj.4.102102
 */
class Osmia_Larva : public Osmia_Egg
{
protected:
public:
	/** @brief Constructs a larva from the state transferred by an egg. */
	Osmia_Larva(struct_Osmia* data);
	/** \brief Osmia_Larva ReInit for object pool */
	virtual void ReInit(struct_Osmia* data);
	/** @brief Destroys the larval stage object. */
	virtual ~Osmia_Larva();
	/** \brief The Step is the second 'part' of the timestep that an animal can behave in. It is called continuously until all animals report that they are 'DONE'. */
	virtual void Step(void);
	/** \brief The BeginStep is the first 'part' of the timestep that an animal can behave in. It is called once per timestep. */
	virtual void BeginStep(void) { ; } // NB this is not used
protected:
	/** @brief Applies daily larval mortality and degree-day development. */
	virtual TTypeOfOsmiaState st_Develop(void);
	/** \brief Behavioural state pupate */
	virtual TTypeOfOsmiaState st_Prepupate(void);
	/** \brief Daily mortality test for larvae */
	virtual bool DailyMortality() { if (g_rand_uni_fnc() < m_DailyDevelopmentMortLarvae) return true; else return false; }
};

/**
 * \class Osmia_Prepupa
 * \brief Summer dormancy: a temperature-modulated but largely time-based stage.
 *
 * <b>Biological Basis:</b>
 * The prepupal stage is a summer diapause whose duration regulates the synchronisation of adult
 * diapause with winter temperatures. Unlike the egg, larval and pupal stages it is not well
 * described by degree-day accumulation: development is fastest at an intermediate optimum and slows
 * at both higher and lower temperatures.
 *
 * <b>Implementation Approach:</b>
 * A quadratic rate function q(T), defined by @c cfg_OsmiaPrepupalRateA, @c cfg_OsmiaPrepupalRateB and
 * @c cfg_OsmiaPrepupalRateC and normalised to 1.0 at @c cfg_OsmiaPrepupalRateTOpt, scales the base
 * duration @c cfg_OsmiaPrepupaDevelTotalDays. Stage duration is therefore
 * OSMIA_PREPUPADEVELDAYS * q(Topt)/q(T).
 *
 * <b>Key Assumptions:</b>
 * - The thermal response is symmetric about the optimum, as a quadratic implies.
 * - Individual variation is +/-10%, assumed by the Formal Model without data for fitting.
 *
 * <b>Limitations:</b>
 * - Only the scale is calibrated; the three shape coefficients and the optimum are fixed by the
 *   Formal Model.
 *
 * \warning st_Develop() previously evaluated its completion test as
 *          <tt>if (m_AgeDegrees++ > ...)</tt>. The post-increment added a flat 1.0 per day on top of
 *          the temperature-dependent rate, roughly halving the stage's thermal sensitivity: the
 *          10 C to 22 C duration ratio was 1.53 as implemented against 3.28 as specified. The term
 *          has been removed and the rate function is now evaluated continuously rather than from a
 *          42-entry lookup table. @c cfg_OsmiaPrepupaDevelTotalDays had been set to 45 in the presence
 *          of the flat term and must not be restored to that value without it.
 *
 * \see Osmia_Larva, Osmia_Pupa
 * \par References:
 * - Sgolastra, F. et al. (2012) Duration of prepupal summer dormancy regulates synchronization of
 *   adult diapause with winter temperatures in bees of the genus <i>Osmia</i>. <i>Journal of Insect
 *   Physiology</i>. https://doi.org/10.1016/j.jinsphys.2012.04.008
 * - Radmacher, S. & Strohm, E. (2011) Effects of constant and fluctuating temperatures on the
 *   development of the solitary bee <i>Osmia bicornis</i>. <i>Apidologie</i> 42(6): 711-720.
 *   https://doi.org/10.1007/s13592-011-0078-9
 * - Ziolkowska, E., Bednarska, A.J., Laskowski, R. & Topping, C.J. (2023) The Formal Model for the
 *   solitary bee <i>Osmia bicornis</i> L. agent-based model. <i>Food and Ecological Systems
 *   Modelling Journal</i> 4: e102102. https://doi.org/10.3897/fmj.4.102102
 */
class Osmia_Prepupa : public Osmia_Larva
{
public:
	/** @brief Constructs a prepupa and draws its individual development target within ±10% of the configured mean. */
	Osmia_Prepupa(struct_Osmia* data);
	/** \brief Osmia_Prepupa ReInit for object pool */
	virtual void ReInit(struct_Osmia* data);
	/** @brief Destroys the prepupal stage object. */
	virtual ~Osmia_Prepupa();
	/** \brief The Step is the second 'part' of the timestep that an animal can behave in. It is called continuously until all animals report that they are 'DONE'. */
	virtual void Step(void);
	/** \brief The BeginStep is the first 'part' of the timestep that an animal can behave in. It is called once per timestep. */
	virtual void BeginStep(void) { ; } // NB this is not used
protected:
	/** @brief Applies daily prepupal mortality and accumulates the temperature-dependent development rate. */
	virtual TTypeOfOsmiaState st_Develop(void);
	/** \brief Behavioural state for emerging from the pupa */
	virtual TTypeOfOsmiaState st_Pupate(void);
	/** \brief Daily mortality test for prepupae */
	virtual bool DailyMortality() { if (g_rand_uni_fnc() < m_DailyDevelopmentMortPrepupae) return true; else return false; }
	/** /brief A target for day/temp related development */
	double m_myOsmiaPrepupaDevelTotalDays;
};

/**
 * \class Osmia_Pupa
 * \brief Metamorphosis to the adult form, temperature-driven.
 *
 * <b>Biological Basis:</b>
 * Pupal development completes the transformation to the adult, which then remains within the cocoon
 * over winter. As for the egg and larva, the rate is temperature-dependent.
 *
 * <b>Implementation Approach:</b>
 * Degree-day accumulation above @c cfg_OsmiaPupaDevelThreshold toward @c cfg_OsmiaPupaDevelTotalDD,
 * with st_Emerge() creating the Osmia_InCocoon that follows.
 *
 * <b>Key Assumptions:</b>
 * - Development rate is linear above the threshold and zero below it; a non-linear response was not
 *   adopted in the current implementation.
 * - Mortality is constant across the stage, and independent of temperature and condition.
 *
 * <b>Limitations:</b>
 * - The threshold and the degree-day sum are <b>calibrated, not measured</b>, and are jointly
 *   identifiable rather than separately determined. The pair must be changed together, and neither
 *   value should be interpreted on its own.
 * - The calibrated pair departs substantially from the Formal Model's a priori values.
 *
 * \see Osmia_Prepupa, Osmia_InCocoon
 * \par References:
 * - Radmacher, S. & Strohm, E. (2011) Effects of constant and fluctuating temperatures on the
 *   development of the solitary bee <i>Osmia bicornis</i>. <i>Apidologie</i> 42(6): 711-720.
 *   https://doi.org/10.1007/s13592-011-0078-9
 * - Ziolkowska, E., Bednarska, A.J., Laskowski, R. & Topping, C.J. (2023) The Formal Model for the
 *   solitary bee <i>Osmia bicornis</i> L. agent-based model. <i>Food and Ecological Systems
 *   Modelling Journal</i> 4: e102102. https://doi.org/10.3897/fmj.4.102102
 */
class Osmia_Pupa : public Osmia_Prepupa
{
public:
	/** @brief Constructs a pupa from the state transferred by a prepupa. */
	Osmia_Pupa(struct_Osmia* data);
	/** \brief Osmia_Pupa ReInit for object pool */
	virtual void ReInit(struct_Osmia* data);
	/** @brief Destroys the pupal stage object. */
	virtual ~Osmia_Pupa();
	/** \brief The Step is the second 'part' of the timestep that an animal can behave in. It is called continuously until all animals report that they are 'DONE'. */
	virtual void Step(void);
	/** \brief The BeginStep is the first 'part' of the timestep that an animal can behave in. It is called once per timestep. */
	virtual void BeginStep(void) { ; } // NB this is not used
protected:
	/** @brief Applies daily pupal mortality and degree-day development. */
	virtual TTypeOfOsmiaState st_Develop(void);
	/** \brief Behavioural state for emerging from the pupa */
	virtual TTypeOfOsmiaState st_Emerge(void);
	/** \brief Daily mortality test for pupae */
	virtual bool DailyMortality() { if (g_rand_uni_fnc() < m_DailyDevelopmentMortPupae) return true; else return false; }

};

/**
 * \class Osmia_InCocoon
 * \brief The fully-formed adult within the cocoon, through pre-wintering, overwintering and
 *        emergence.
 *
 * <b>Biological Basis:</b>
 * Pre-wintering and wintering temperature regimes determine weight loss, overwinter survival and
 * the timing of emergence in <i>Osmia</i>. Emergence phenology matters ecologically because it
 * governs the match between bee activity and the flowering of the plants they depend on, and bee
 * emergence and plant flowering do not respond to warming at the same rate.
 *
 * <b>Implementation Approach:</b>
 * The stage proceeds in phases: pre-wintering, overwintering degree-day accumulation above
 * @c cfg_OsmiaInCocoonOverwinteringTempThreshold, spring pre-emergence, and emergence. The emergence
 * counter is a linear function of accumulated overwintering degree-days
 * (@c cfg_OsmiaInCocoonEmergCountConst, @c cfg_OsmiaInCocoonEmergCountSlope) plus an individual draw
 * from @c cfg_OsmiaEmergenceProbArgs. WinterMortality() applies an overwinter survival function.
 *
 * <b>Key Assumptions:</b>
 * - Overwintering is driven by accumulated temperature alone; humidity, desiccation and reserve
 *   depletion are not represented.
 * - The emergence kernel is an empirical distribution held fixed rather than fitted.
 *
 * <b>Limitations:</b>
 * - All four overwintering and emergence parameters are <b>fitted</b> and are not separable from
 *   the in-nest stages.
 * - The emergence counter is degenerate: fitting emergence onset alone admits multiple equally good
 *   solutions, so the constant and the slope are not individually identified.
 *
 * \note WinterMortality()'s constant and slope operate on a percentage scale, which their names do
 *       not indicate.
 *
 * \see Osmia_Pupa, Osmia_Female
 * \par References:
 * - Bosch, J. & Kemp, W.P. (2004) Effect of pre-wintering and wintering temperature regimes on
 *   weight loss, survival, and emergence time in the mason bee <i>Osmia cornuta</i>.
 *   <i>Apidologie</i> 35(5): 469-479. https://doi.org/10.1051/apido:2004035
 * - Kehrberger, S. & Holzschuh, A. (2019) Warmer temperatures advance flowering in a spring plant
 *   more strongly than emergence of two solitary spring bee species. <i>PLOS ONE</i> 14(6): e0218824.
 *   https://doi.org/10.1371/journal.pone.0218824
 * - Ziolkowska, E., Bednarska, A.J., Laskowski, R. & Topping, C.J. (2023) The Formal Model for the
 *   solitary bee <i>Osmia bicornis</i> L. agent-based model. <i>Food and Ecological Systems
 *   Modelling Journal</i> 4: e102102. https://doi.org/10.3897/fmj.4.102102
 */
class Osmia_InCocoon : public Osmia_Pupa
{
protected:
	/** \brief Counter for the number of days remaining until emergence from the nest */
	int m_emergencecounter;

	/** \brief Degree-days accumulated above the configured pre-winter threshold before m_PreWinteringEndFlag is set. */
	double m_DDPrewinter;
public:
	/** @brief Constructs an adult-in-cocoon and initialises its overwintering accumulators. */
	Osmia_InCocoon(struct_Osmia* data);
	/** \brief Osmia_Adult ReInit for object pool */
	virtual void ReInit(struct_Osmia* data);
	/** @brief Destroys the adult-in-cocoon stage object. */
	virtual ~Osmia_InCocoon();
	/** \brief The Step is the second 'part' of the timestep that an animal can behave in. It is called continuously until all animals report that they are 'DONE'. */
	virtual void Step(void);
	/** \brief The BeginStep is the first 'part' of the timestep that an animal can behave in. It is called once per timestep. */
	virtual void BeginStep(void) { ; } // NB this is not used
	/** \brief Set method for m_OverwinteringTempThreshold */
	static void SetOverwinteringTempThreshold(double a_temp) { m_OverwinteringTempThreshold = a_temp; }
	/** \brief Returns pre-winter degree-days accumulated above the configured threshold. */
	double GetDDPreWinter() { return m_DDPrewinter; }
protected:
	/** @brief Advances pre-wintering, overwintering and spring emergence timing. */
	virtual TTypeOfOsmiaState st_Develop(void);
	/** \brief Behavioural state for emerging from the InCocoon */
	virtual TTypeOfOsmiaState st_Emerge(void);
	/** Calculates pupal mortality as a result of prewinter day degrees */
	bool WinterMortality();
	/** \brief Parameter for overwintering day degrees threshold */
	static double m_OverwinteringTempThreshold;
};

/**
 * \class Osmia_Female
 * \brief The free-living adult female: foraging, nest finding, provisioning, sex allocation,
 *        dispersal and egg laying.
 *
 * This is the only stage that acts on the landscape rather than merely developing within it, and it
 * carries the whole of the model's reproductive output.
 *
 * <b>Biological Basis:</b>
 * A female <i>O. bicornis</i> occupies a pre-existing cavity, provisions a linear series of brood
 * cells with pollen and nectar, and lays one egg per cell. Maternal investment is shaped by the risk
 * of open-cell parasitism, which rises with the time a cell stays open, so a female trades
 * provisioning effort against exposure. Sex allocation is conditional on maternal age and mass, with
 * daughters placed in the better-provisioned inner cells. Cell construction and provisioning times
 * are taken from direct observation.
 *
 * <b>Implementation Approach:</b>
 * Behaviour is a state machine over TTypeOfOsmiaState, re-entered repeatedly within a single Step()
 * until the female signals that her day is finished. Provisioning accumulates toward a target
 * derived from the adult mass floor and the mass-from-provision mapping; a cell is laid female only
 * if the accumulated provision exceeds that target, and is switched to male otherwise.
 *
 * <b>Key Assumptions:</b>
 * - One nest is occupied at a time, and no information is carried between nests.
 * - Foraging patch choice is by resource score alone: travel cost does not enter the decision.
 * - No learning, and no interaction between females except through shared resources and nest sites.
 *
 * <b>Limitations:</b>
 * - Foraging, movement, mortality and parasitism are <b>not calibrated</b>; evaluation of those
 *   processes remains outstanding.
 *
 * \note PlanEggsPerNest() makes an independent calibrated draw for each nest. The calibrated
 *       distribution includes a two-egg shift with probability 0.45; st_ReproductiveBehaviour()
 *       then reduces the draw by two eggs for every preceding nest and applies the configured floor.
 * \warning Init() terminates the program with <tt>std::exit()</tt> if a female's mass falls outside
 *          [@c cfg_OsmiaFemaleMassMin, @c cfg_OsmiaFemaleMassMax]. This is presently unreachable, because
 *          the provisioning target and the founding cohort both derive their bounds from the same
 *          configured mass mapping that the check uses. Any change that decouples them reinstates a
 *          hard termination rather than a degradation.
 * \note Init() clamps the calculated maternal mass-class index to the final lookup-table class, so
 *       the permitted 200 mg upper bound uses the 195 mg class rather than indexing beyond the table.
 * \note An all-male nest uses the male minimum provision target for every planned egg; no female
 *       provision-mass decline is calculated when the planned number of female eggs is zero.
 * \note st_ReproductiveBehaviour() tests that the provision plan is non-empty before reading its
 *       first element.
 *
 * \see Osmia_InCocoon, Osmia_Nest, Osmia_Nest_Manager
 * \par References:
 * - Seidelmann, K. (2006) Open-cell parasitism shapes maternal investment patterns in the Red Mason
 *   bee <i>Osmia rufa</i>. <i>Behavioral Ecology</i> 17(5): 839-848.
 *   https://doi.org/10.1093/beheco/arl017
 * - Seidelmann, K., Ulbrich, K. & Mielenz, N. (2010) Conditional sex allocation in the Red Mason
 *   bee, <i>Osmia rufa</i>. <i>Behavioral Ecology and Sociobiology</i> 64(3): 337-347.
 *   https://doi.org/10.1007/s00265-009-0850-2
 * - Raw, A. (1972) The biology of the solitary bee <i>Osmia rufa</i> (L.) (Megachilidae).
 *   <i>Transactions of the Royal Entomological Society of London</i>.
 *   https://doi.org/10.1111/j.1365-2311.1972.tb00364.x
 * - Ziolkowska, E., Bednarska, A.J., Laskowski, R. & Topping, C.J. (2023) The Formal Model for the
 *   solitary bee <i>Osmia bicornis</i> L. agent-based model. <i>Food and Ecological Systems
 *   Modelling Journal</i> 4: e102102. https://doi.org/10.3897/fmj.4.102102
 */
class Osmia_Female : public Osmia_InCocoon
{
public:
#ifdef __OSMIARECORDFORAGE
	static double m_foragesum;
	static int m_foragecount;
#endif
protected:
	//Attributes
	double m_currentpollenlevel;
	/** \brief The change in pollen return that triggers a new search */
	static double m_pollengiveupthreshold;
	/** \brief The minimum pollen return below which a new forage search is triggered */
	static double m_pollengiveupreturn;
	/** \brief Cached cfg_OsmiaMaxPollen (max pollen a bee forages per hour) — set once, avoids per-forage .value(). */
	static double m_OsmiaMaxPollen;
	/** \brief Cached cfg_OsmiaSugarPerDay (nectar/sugar eaten per day) — set once, avoids per-forage .value(). */
	static double m_OsmiaSugarPerDay;
	/** \brief records the current density grid index for fast access */
	//int m_DensityIndex;
	/** \brief Keeps track of the number of days a cell is open */
	int m_CellOpenDays;
	/** \brief Keeps track of any part time cell construction hours */
	double m_CellCarryOver;
	/** \brief Contains the number of all eggs yet to lay */
	int m_EggsToLay;
	/** \brief Keeps a track of the planned number of eggs for this nest */
	int m_EggsThisNest;
	/** \brief Zero-based order of the next nest, used for the calibrated two-egg decline between nests. */
	int m_NestOrder;
	/** \brief a flag determining if dispersal is necessary */
	bool m_ToDisperse;
	/** \brief The number of days of post emergence life */
	int m_EmergeAge;
	/** \brief The location of the current nest, holds -1 in m_x when no nest */
	APoint m_CurrentNestLoc;
	/** \brief  The number of days needed for 1 cell construction */
	int m_ProvisioningTime;
	/** \brief  A counter to keep track of the flying days during nest cell construction*/
	int m_FlyingCounter;
	/** \brief Records the amount of pollen currently provisioned in a cell */
	double m_CurrentProvisioning;
	/** \brief  Used to put the bees into a size class - 0,1,2,3 very small, small, medium, big*/
	int m_BeeSizeScore1;
	/** \brief  Used to put the bees into a smaller size classes than used for m_BeeSizeScore1, size class controlled by cfg_OsmaiAdultMassCategoryStep */
	int m_BeeSizeScore2;
	/** \brief A list of female nest targets */
	deque<double>m_NestProvisioningPlan;
	/** \brief A list of eggs male/female */
	deque<bool>m_NestProvisioningPlanSex;
	/** \brief Flag to indicate whether we have a foraging location (=true) */
	bool m_ForageLoc;
	/** \brief Index to the resource providing polygon lists in the Osmia_Population_Manager */
	int m_ForageLocPoly;
	/** \brief An attribute used to scale the available pollen based on assumed competetion from other bee species */
	static double m_PollenCompetitionsReductionScaler;
	/** \brief A vector holding the age related efficiency of Osmia foraging indexed by day (from Seidelmann 2006) */
	static vector<double> m_FemaleForageEfficiency;
	/** \brief The variable to record the bee's forageing location -- x*/
	int m_ForageLocX;
	/** \brief The variable to record the bee's forageing location -- y*/
	int m_ForageLocY;
	/** \brief The array to store the pesticide in the foraged resource.*/
	double* m_foraged_resource_pesticide;
	

public:
	/** \brief The current pesticide death probability after trigger excedence */
	static double m_OsmiaEggPPPEffectProb;
	/** \brief Pesticide trigger level */
	static double m_OsmiaEggPPPThreshold;
	/** \brief The current pesticide death probability after trigger excedence */
	static double m_OsmiaPPPEffectProb;
	/** \brief Pesticide trigger level */
	static double m_OsmiaPPPThreshold;
	/** \brief Pesticide decay rate in the osmia body.*/
	static double m_OsmiaPPPDecayRate;
	/** \brief Pesticide absorption rate to the osmia body for overspray.*/
	static double m_OsmiaPPPAbsorptionRateOverspray;
	/** \brief Pesticide absorption rate for contact.*/
	static double m_OsmiaPPPAbsorptionRateContact;
	/** \brief Pesticide overspray to the osmia body surface.*/
	static double m_OsmiaPPPOversprayBodySurface;
	/** \brief Pesticide contact to the osmia body surface.*/
	static double m_OsmiaPPPContactBodySurface;
	/** \brief Overspray chance */
	static double m_OsmiaPPPOversprayChance;
protected:
	/** @brief Returns the adult internal pesticide-burden mortality threshold. */
	static double GetPPPThreshold() { return m_OsmiaPPPThreshold; }
	/** \brief Get body PPP death chance */
	static double GetPPPEffectProb() { return m_OsmiaPPPEffectProb; }
	/** \brief Get body PPP decay rate */
	static double GetPPPDecayRate() { return m_OsmiaPPPDecayRate; }
	/** \brief Get the pesticide absoption rate from surface to body.*/
	static double GetPPPAbsorptionRateOverspray() { return m_OsmiaPPPAbsorptionRateOverspray; }
	/** \brief Get the pesticide overspray to the osmia body surface.*/
	static double GetPPPOversprayBodySurface() { return m_OsmiaPPPOversprayBodySurface; }
	/** \brief Get the pesticide contact to the osmia body surface.*/
	static double GetPPPContactBodySurface() { return m_OsmiaPPPContactBodySurface; }


#ifdef __OSMIATESTING
	OsmiaNestData m_target;
	OsmiaNestData m_achieved;
	bool m_firstnestflag;
#endif // __OSMIATESTING


	//Methods
	/** @brief Closes and, when empty, releases the current nest before removing the female. */
	virtual void st_Dying(void);
	//--------------------------------------------------------------------------------------------------------------------------------
	/** \brief Behavioural state development */
	virtual TTypeOfOsmiaState st_Develop(void);
	/** \brief Find a suitable location for a nest */
	virtual bool FindNestLocation(void);
	/** \brief This checks for the need to dispese and does it if necessary. */
	virtual TTypeOfOsmiaState st_Dispersal(void);
	/** \brief The foraging algorithm for Osmia */
	double Forage(void);
	/** \brief This checks for the need to do something regarding reproduction and does it if necessary. */
	virtual TTypeOfOsmiaState st_ReproductiveBehaviour(void);
	/**
	 * \brief Draws the calibrated planned egg number for one nest before the nest-order reduction.
	 * \return Egg-number draw including maternal-size scaling and the calibrated 0.45 two-egg shift.
	 */
	int PlanEggsPerNest();
	/** \brief This calculates the number of eggs the female should lay. */
	void CalculateEggLoad() {
		/**
		* According to Seidelmann (2010) female of a mass m_mass can produce on average the following number of eggs per nest: \n
		* no_eggs_nest = 0.0371 * m_mass + 2.8399 (+/- 3 eggs)\n
		* If we assume that a bee can colonize a maximum of N nests (onfig variable) in her lifetime, the maximum total number of eggs to lay is defined as:\n
		* total_no_eggs = N * no_eggs_nest
		*/
		m_EggsToLay = int((m_TotalNestsPossible * (0.0371 * m_Mass + 2.8399)) + (g_rand_uni_fnc() * 6) - 3);
		m_EggsThisNest = 0; // Planned independently when each nest is founded.
	}
	/** \brief Determines the type of parasitoid if any */
	TTypeOfOsmiaParasitoids CalcParaistised(double a_daysopen);
	/** \brief Produces an egg */
	void LayEgg();

public:
	/** @brief Constructs an adult female and initialises its reproductive and foraging state. */
	Osmia_Female(struct_Osmia* data);
	/** \brief Osmia_Female ReInit for object pool */
	virtual void ReInit(struct_Osmia* data);
	/** \brief Osmia_Female destructor */
	virtual ~Osmia_Female();
	/** \brief Osmia_Femae initialisation code for Constructor and ReInit */
	virtual void Init(double a_mass);
	/** BeginStep sets up conditions before the Step code */
	virtual void BeginStep(void);
	/** \brief The Step is the second 'part' of the timestep that an animal can behave in. It is called continuously until all animals report that they are 'DONE'. */
	virtual void Step(void);
	/** \brief Record the give up level for pollen from a forage patch we are foraging from */
	static void SetPollenGiveUpThreshold(double a_prop) { m_pollengiveupthreshold = a_prop; }
	/** \brief Record the give up level for pollen from a forage patch we are foraging from */
	static void SetPollenGiveUpReturn(double a_value) { m_pollengiveupreturn = a_value; }
	/** \brief Cache the max-pollen-per-hour and sugar-per-day cfg constants (called once at setup). */
	static void SetMaxPollen(double a_value) { m_OsmiaMaxPollen = a_value; }
	/** @brief Caches the adult daily nectar requirement in the units expected by the landscape nectar layer. */
	static void SetSugarPerDay(double a_value) { m_OsmiaSugarPerDay = a_value; }
	/** \brief Record the daily mortality parameter values */
	static void  SetDailyMort(double a_prob) { m_OsmiaFemaleBckMort = a_prob; }
	/** \brief Record the daily mortality parameter values */
	static void SetNestFindAttempts(int a_no) { m_OsmiaFindNestAttemptNo = a_no; }
	/** \brief Set the min eggs parameter value */
	static void SetMinEggsPerNest(int a_eggs) { m_OsmiaFemaleMinEggsPerNest = a_eggs; }
	/** \brief Set the min eggs parameter value */
	static void SetMaxEggsPerNest(int a_eggs) { m_OsmiaFemaleMaxEggsPerNest = a_eggs; }
	/** \brief Set the conversion ratio cocoon to provisioning */
	static void SetCocoonToProvisionMass(double a_ratio) {
		m_CocoonToProvisionMass = a_ratio;
		m_TotalProvisioningMassLoss = cfg_OsmiaTotalCocoonMassLoss.value() * a_ratio;
		m_TotalProvisioningMassLossRange = cfg_OsmiaTotalCocoonMassLossRange.value() * a_ratio;
		m_TotalProvisioningMassLossRangeX2 = m_TotalProvisioningMassLossRange * 2.0;
	}
	/** \brief Set the conversion ratio provisioning to cocoon */
	static void SetProvisionToCocoonMass(double a_ratio) { m_ProvisionToCocoonMass = a_ratio; }
	/** \brief Set pollen score to provisioned mg value */
	static void SetPollenScoreToMg(double a_ratio) { m_PollenScoreToMg = a_ratio; }
	/** \brief Set the target mass for male cell provisioning - this is a minimum */
	void SetMaleMinTargetProvisionMass(double a_mass) { m_MaleMinTargetProvisionMass = a_mass; }
	/** \brief Set the target mass for female cell provisioning - this is a minimum */
	void SetFemaleMinTargetProvisionMass(double a_mass) { m_FemaleMinTargetProvisionMass = a_mass; }
	/** \brief Set the max target mass for female cell provisioning */
	void SetFemaleMaxTargetProvisionMass(double a_mass) { m_FemaleMaxTargetProvisionMass = a_mass; }
	/** \brief Set minimum cell construction time */
	static void SetMinimumCellConstructionTime(double a_time) { m_MinimumCellConstructionTime = a_time; }
	/** \brief Set maximum cell construction time */
	static void SetMaximumCellConstructionTime(double a_time) { m_MaximumCellConstructionTime = a_time; }
	/** \brief Set the maximum number of nests possible for a bee */
	static void SetTotalNestsPossible(int a_total) { m_TotalNestsPossible = a_total; }
	/** \brief Sets Bombylid probability */
	static void SetBombylidProbability(double a_prob) { m_BombylidProbability = a_prob; }
	/** \brief Set the conversion ratio time cell open to open cell parasitism */
	static void SetParasitismProbToTimeCellOpen(double a_ratio) { m_ParasitismProbToTimeCellOpen = a_ratio; }
	/** \brief Sets the UsingMechanisticParasitoids flag */
	static void SetUsingMechanisticParasitoids(bool a_flag) { m_UsingMechanisticParasitoids = a_flag; }
	/** @brief Stores the per-capita attack coefficients for the optional mechanistic parasitoid path. */
	static void SetParasitoidParameters(vector<double> a_params) { m_ParasitoidAttackChance = a_params; }
	/** \brief sets the m_DensityDependentPollenRemovalConst value */
	void SetDensityDependentPollenRemovalConst(double a_value) { m_DensityDependentPollenRemovalConst = a_value; }
	/** \brief Save a forage efficiency value */
	static void AddForageEfficiency(double a_eff) { m_FemaleForageEfficiency.push_back(a_eff); }
	/**
	 * @brief Legacy, inactive pollen-removal helper.
	 * @warning The only call site is commented out. The current body assigns the requested amount to
	 *          the output without removing pollen from the landscape.
	 */
	void GetPollenInPolygon(double& a_required_amount, double& a_foraged_amount, int a_polygon, int a_loc_x, int a_loc_y);
	/**
	 * @brief Responds to pesticide-related farm events affecting the female's current location.
	 * @param event Management event supplied by the ALMaSS landscape.
	 * @return true when the event was handled as an Osmia pesticide exposure or caused mortality.
	 * @details A `product_treat` event invokes overspray only when the pesticide engine is enabled;
	 * otherwise it has no biological effect. The legacy insecticide and biocide events can apply the
	 * configured background mortality response independently of that engine.
	 */
	virtual bool OnFarmEvent( FarmToDo event );
	/** \brief Override the pesticide contact function.*/
	virtual void DoPesticideContact(int a_x = -1, int a_y = -1);

	//code used for pestiside store
	#ifdef __OSMIA_PESTICIDE_STORE
	unsigned int m_animal_id;
	#endif
};
#endif
