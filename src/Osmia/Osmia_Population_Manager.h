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
/** \file Osmia_Population_Manager.h 
Version of  May 2017 \n
By Chris J. Topping \n \n
*/

#include "../BatchALMaSS/lock_profile.h"
#include <forward_list>
#include <list>   // m_NestList -- std::list gives O(1) erase via the nest's stored iterator

//---------------------------------------------------------------------------
#ifndef Osmia_Population_ManagerH
#define Osmia_Population_ManagerH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

class Osmia;
class OsmiaParasitoid_Population_Manager;


/**
\brief Osmia life stages modelled
*/
enum class TTypeOfOsmiaLifeStages : int // int is used because this may be used as an index to an array passed in CreateObjects
{
	to_OsmiaEgg = 0,
	to_OsmiaLarva,
	to_OsmiaPrepupa,
	to_OsmiaPupa,
	to_OsmiaInCocoon,
	to_OsmiaFemale
};

typedef vector<double>  eggsexratiovsagelogisticcurvedata;
typedef vector<double>  femalecocoonmassvsagelogisticcurvedata;
//------------------------------------------------------------------------------
/**
 * \class OsmiaPollenNectarThresholds
 * \brief Stores one monthly pollen and nectar threshold pair.
 *
 * <b>Implementation Approach:</b>
 * A small value type populated from @c cfg_OsmiaPollenThresholds and @c cfg_OsmiaNectarThresholds during
 * Osmia_Population_Manager::Init().
 *
 * \warning The resulting values are stored in Osmia_Population_Manager::m_PN_thresholds but are not
 *          consulted by the supplied foraging implementation. They currently have no effect on
 *          model behaviour.
 *
 * \see Osmia_Female, Osmia_Population_Manager
 */
class OsmiaPollenNectarThresholds
{
public:
	double m_pollenTqual = 0.0;
	double m_nectarTqual = 0.0;
	double m_pollenTquan = 0.0;
	double m_nectarTquan = 0.0;
};
//------------------------------------------------------------------------------

/**
 * \class OsmiaParasitoidSubPopulation
 * \brief One parasitoid type as an aggregate sub-population rather than as individuals.
 *
 * <b>Biological Basis:</b>
 * <i>O. bicornis</i> brood cells are attacked by cleptoparasites and bombylid flies while the cell
 * remains open. Attack risk rises with the time the cell is exposed, which is a property of the
 * bee's provisioning behaviour rather than of the parasitoid's search.
 *
 * <b>Implementation Approach:</b>
 * A counter of individuals per type with monthly mortality and simple dispersal, rather than
 * individual agents. Reproduce() is deliberately empty: because attack probability is treated as a
 * property of the bee, the parasitoid population needs no reproduction step of its own.
 *
 * <b>Limitations:</b>
 * - This mechanistic treatment extends beyond the Formal Model and is <b>disabled by default</b>
 *   (@c cfg_UsingMechanisticParasitoids is false). The statistical treatment in Osmia_Female is used
 *   instead.
 * - Nothing here is calibrated; calibration and evaluation of parasitism remain outstanding.
 *
 * \warning DailyMortality() indexes m_MortalityPerMonth with m_ThisMonth, which is initialised to
 *          -1 and never assigned: SetThisMonth() has no call site anywhere in the model. Enabling
 *          the mechanistic parasitoids therefore produces an out-of-bounds read on the first call.
 *          This path is unreachable while @c cfg_UsingMechanisticParasitoids remains false.
 *
 * \see OsmiaParasitoid_Population_Manager, Osmia_Female
 * \par References:
 * - Seidelmann, K. (2006) Open-cell parasitism shapes maternal investment patterns in the Red Mason
 *   bee <i>Osmia rufa</i>. <i>Behavioral Ecology</i> 17(5): 839-848.
 *   https://doi.org/10.1093/beheco/arl017
 * - Ziolkowska, E., Bednarska, A.J., Laskowski, R. & Topping, C.J. (2023) The Formal Model for the
 *   solitary bee <i>Osmia bicornis</i> L. agent-based model. <i>Food and Ecological Systems
 *   Modelling Journal</i> 4: e102102. https://doi.org/10.3897/fmj.4.102102
 */
class OsmiaParasitoidSubPopulation
{
protected:
	// Attributes
	/** \brief The total number of parasitoids in this population */
	double m_NoParasitoids;
	/** \brief The time-step related total dispersal loss */
	double m_DiffusionRate;
	/** \brief The distance related dispersal loss constants */
	double m_DiffusionConstant; 
	/** \brief The index of the surrounding cells, a speed optimisation */
	int m_CellIndexArray[8]; 
	/** \brief cell_index x */
	int m_x;
	/** \brief cell_index y */
	int m_y;
	/** \brief A pointer to the owning population manager */
	OsmiaParasitoid_Population_Manager* m_OurPopulationManager;
	/** \brief Daily mortality rates each month */
	static array<double,12> m_MortalityPerMonth;
	/** \brief records the month (optimsation) */
	static int m_ThisMonth;
	// Methods
public:
	/**
	 * @brief Creates one spatial parasitoid sub-population.
	 * @param a_dispersalfraction Fraction entering dispersal per daily update.
	 * @param a_startno Initial number of parasitoids.
	 * @param a_x Cell x-index.
	 * @param a_y Cell y-index.
	 * @param a_wide Number of cells across the parasitoid grid.
	 * @param a_high Number of cells down the parasitoid grid.
	 * @param a_popman Owning parasitoid population manager.
	 */
	OsmiaParasitoidSubPopulation(double a_dispersalfraction, double a_startno, int a_x, int a_y, int a_wide, int a_high, OsmiaParasitoid_Population_Manager* a_popman);
	/** @brief Destroys the aggregate sub-population. */
	~OsmiaParasitoidSubPopulation();
	/** @brief Adds parasitoids to this grid cell. */
	void Add(double a_change) { m_NoParasitoids += a_change; }
	/** @brief Removes parasitoids from this grid cell. */
	void Remove(double a_change) { m_NoParasitoids -= a_change; }
	/** @brief Returns the current aggregate abundance in this grid cell. */
	double GetSubPopnSize() { return m_NoParasitoids; }
	/** \brief Removes parasitoids killed by daily mortality */
	void DailyMortality();
	/** \brief Moves parasitoids that move by dispersal */
	void Dispersal();
	/** \brief Carries out any reproduction possible */
	void Reproduce();
	/** @brief Runs mortality, dispersal and the currently empty reproduction step in that order. */
	virtual void DoFirst() {
		/** Removes parasitoids killed by daily mortality */
		DailyMortality();
		/** Moves parasitoids that move by dispersal */
		Dispersal();
		/** Carries out any reproduction possible */
		Reproduce();
	}
	/** @brief Sets the zero-based month index used by DailyMortality(). */
	void SetThisMonth(int a_month) { m_ThisMonth = a_month;  }
	/** @brief Replaces the twelve monthly daily-mortality values shared by all parasitoid cells. */
	void SetMortalities(array<double, 12> a_morts) {
		m_MortalityPerMonth = a_morts;
	}
};
//------------------------------------------------------------------------------

/**
 * \class OsmiaParasitoid_Population_Manager
 * \brief Coordinates the parasitoid sub-populations across the landscape.
 *
 * <b>Implementation Approach:</b>
 * Holds one OsmiaParasitoidSubPopulation per parasitoid type per location and provides the add,
 * remove and query interface used by Osmia_Female when a cell is parasitised. Derives from
 * Population_Manager for the ALMaSS scheduling contract, but manages counters rather than agents.
 *
 * <b>Limitations:</b>
 * - Disabled by default with the rest of the mechanistic parasitoid treatment; see
 *   OsmiaParasitoidSubPopulation for the warning that applies when it is enabled.
 *
 * \see OsmiaParasitoidSubPopulation, Osmia_Female
 */
class OsmiaParasitoid_Population_Manager : public Population_Manager
{
protected:
	// Attributes
	/** \brief The array storing the sub-populations of parasitoids */
	vector<OsmiaParasitoidSubPopulation*>m_SubPopulations;
	/** \brief A useful pointer to the lanscape object */
	Landscape* m_TheLandscape;
	/** \brief the number of subpopulation cells wide */
	unsigned m_Wide;
	/** \brief the number of subpopulation cells high */
	unsigned m_High;
	/** \brief Width of the cell in m (they are square) */
	unsigned m_CellSize;
	/** \brief Total cells */
	unsigned m_Size;
	// Methods

public:
	/**
	 * @brief Creates the spatial grids for all modelled parasitoid types.
	 * @param a_landscape Landscape providing simulation dimensions.
	 * @param a_cellsize Width and height of each square parasitoid grid cell, in metres.
	 */
	OsmiaParasitoid_Population_Manager(Landscape* a_landscape, int a_cellsize);
	/** @brief Deletes all parasitoid sub-populations owned by the manager. */
	~OsmiaParasitoid_Population_Manager();
	/** @brief Adds dispersing parasitoids to a flattened type-and-cell index. */
	void AddDispersers(int a_ref, double a_dispersers) {
		m_SubPopulations[a_ref]->Add(a_dispersers);
	}
	/** @brief Removes dispersing parasitoids from a flattened type-and-cell index. */
	void RemoveParasitoids(int a_ref, double a_dispersers) {
		m_SubPopulations[a_ref]->Remove(a_dispersers);
	}
	/** @brief Returns abundance for a flattened type-and-cell index. */
	double GetSize(int a_ref) { return m_SubPopulations[a_ref]->GetSubPopnSize(); }
	/** @brief Returns abundance from the first parasitoid grid at a cell coordinate. */
	double GetSize(int a_x, int a_y) { return m_SubPopulations[a_x+a_y*m_Wide]->GetSubPopnSize(); }
	/** \brief returns an array with the parasitoid numbers in the cell at the location given by a_x, a_y */
	array<double, static_cast<unsigned>(TTypeOfOsmiaParasitoids::topara_foobar)> GetParasitoidNumbers(int a_x, int a_y);
	/** @brief Adds one individual of a parasitoid type to the cell containing a landscape coordinate. */
	void AddParasitoid(TTypeOfOsmiaParasitoids a_type, int a_x, int a_y) 
	{
		int subpop = ((a_x / m_CellSize) + (a_y / m_CellSize) * m_Wide) + (static_cast<unsigned>(a_type)-1) * m_Size;
		m_SubPopulations[subpop]->Add(1);
	}
};
//------------------------------------------------------------------------------
/**
 * \class struct_Osmia
 * \brief The state record passed when one life stage creates its successor.
 *
 * <b>Implementation Approach:</b>
 * A life-stage transition in this model is not a change of state in place but the construction of a
 * new object of the next class. The individual copies its state into a struct_Osmia record,
 * Osmia_Population_Manager::CreateObjects() builds the successor from it, and the predecessor is
 * killed. This record is therefore the sole channel by which state crosses a stage boundary: any
 * attribute not present here is lost at the transition.
 *
 * \see Osmia_Population_Manager::CreateObjects, Osmia_Base
 */
class struct_Osmia
{
 public:
  /** \brief x-coord */
  int x;
  /** \brief y-coord */
  int y;
  /** \brief age */
  int age;
  /** \brief the sex of the Osmia */
  bool sex;
  /** \brief Landscape pointer */
  Landscape* L;
  /** \brief Osmia_Population_Manager pointer */
  Osmia_Population_Manager * OPM;
  /** \brief a pointer to a nest */
  Osmia_Nest* nest;
  /** \brief Are we parasitised */
  TTypeOfOsmiaParasitoids parasitised;
  /** \brief The mass of the Osmia in mg */
  double mass;
  /** \brief The state tof the Osmia, this is used to set a dead egg because of pesticide.*/
  double pest_mortality = 0;
  /** \brief This is used to set accumulated overwinter degree days when starting the simulation, it always should be zero otherwise.*/
  double overwintering_degree_days = 0;
};

/**
 * \class OsmiaPolygonEntry
 * \brief The nests held by one landscape polygon, with its capacity and occupancy bookkeeping.
 *
 * <b>Biological Basis:</b>
 * Nest-site availability is a property of the habitat patch. A polygon of a given landscape element
 * type offers a number of cavities proportional to its area, and that number caps how many females
 * can nest there simultaneously.
 *
 * <b>Implementation Approach:</b>
 * Holds the polygon's nest list, its maximum capacity derived from area and per-habitat density,
 * and a cached HasRoom flag kept consistent with capacity so that the common test is cheap.
 *
 * <b>Limitations:</b>
 * - Capacity is computed once and does not change with management or season.
 * - The absolute capacity depends on density figures whose magnitude is not empirically grounded.
 * - The current implementation intentionally multiplies every input density by a hard-coded 0.001
 *   operational scaling factor.
 *
 * \see Osmia_Nest_Manager, Osmia_Nest
 */
class OsmiaPolygonEntry
{
protected:
    /** \brief The polygon's nests. std::list (not forward_list) so a nest can be removed in O(1) via the
     * iterator it stores in Osmia_Nest::m_nest_handle -- the old forward_list required an O(n) linear
     * search under the per-polygon nest lock, which dominated runtime at high nest densities. */
    std::list<Osmia_Nest*> m_NestList;
	/** \brief to record the chance of osmia nesting */
	double m_OsmiaNestProb;
	/** \brief to record the number of possible osmia nests */
	double m_MaxOsmiaNests; // Needs to be double because its used to divide later
	/** \brief to record the number of actual osmia nests (only used for speed - it is the same as the size of the vector m_NestList */
	int m_CurrentOsmiaNests;
	/** \brief Cached deterministic capacity gate: true iff m_CurrentOsmiaNests < m_MaxOsmiaNests.
	 * Maintained live at every mutation of current/max (Inc/Release/SetMax). Lets FindNestLocation
	 * skip the blind retry loop over polygons that are FULL (can never yield a nest this day) WITHOUT
	 * touching the stochastic rand<m_OsmiaNestProb thinning -- so it is behaviour-neutral (fate/RNG identical). */
	bool m_HasRoom;
	/** \brief The polygon area */
	int m_Area;
	/** \brief The polygon reference used by the Landscape and Osmia_Nest_Manager */
	int m_Polyindex;
public:
	/** \brief The constructor for OsmiaPolygonEntry */
	OsmiaPolygonEntry()
	{
		m_CurrentOsmiaNests = 0;
		m_MaxOsmiaNests = 0;
		m_OsmiaNestProb = 0;
		m_HasRoom = false; // max==0 -> no room
		m_Polyindex = -1;
		m_Area = -1;
	}
	/** @brief Constructs an empty polygon entry with a known polygon index and area. */
	OsmiaPolygonEntry(int a_index, int a_area)
	{
		m_CurrentOsmiaNests = 0;
		m_MaxOsmiaNests = 0;
		m_OsmiaNestProb = 0;
		m_HasRoom = false; // max==0 -> no room
		m_Polyindex = a_index;
		m_Area = a_area;
	}

	/** \brief The destructor for OsmiaPolygonEntry */
	~OsmiaPolygonEntry()
	{
		//for (std::vector<Osmia_Nest*>::iterator it = m_NestList.begin(); it != m_NestList.end(); ++it) {
		for(auto it : m_NestList){
			if(it!=NULL)
				delete it;
		}
		m_NestList.clear();
	}
	/** \brief Test to see if a nest is found */
	bool IsOsmiaNestPossible() {
		if ((m_CurrentOsmiaNests < m_MaxOsmiaNests) && (g_rand_uni_fnc() < m_OsmiaNestProb)) return true;
		return false;
	}
	/** \brief Cheap deterministic capacity gate (no RNG draw). True iff this polygon still has free nest
	 * capacity. Used by FindNestLocation to early-out of the blind retry loop when all reachable polygons
	 * are full. Does NOT change the stochastic thinning done by IsOsmiaNestPossible. */
	bool HasRoom() const { return m_HasRoom; }
	/** \brief Release an Osmia nest that is no longer used */
	void ReleaseOsmiaNest(Osmia_Nest* a_nest) {
		m_CurrentOsmiaNests--;
		m_HasRoom = true; // freeing a nest always leaves room (current < max now)
		// O(1) removal: the nest knows its own position in this list (set in IncOsmiaNesting), so we erase
		// directly instead of linearly searching for it. std::list iterators remain valid across other
		// inserts/erases, so the stored handle is still correct however much the list has changed.
		if (a_nest->m_nest_handle_set) {
			m_NestList.erase(a_nest->m_nest_handle);
			a_nest->m_nest_handle_set = false;
			delete a_nest;
		}
	}
	/** \brief Recalculate the nest finding probability */
	void UpdateOsmiaNesting()
	{
		if (m_MaxOsmiaNests <= 0) m_OsmiaNestProb = 0.0;
		else {
			m_OsmiaNestProb = 1.0 - (m_CurrentOsmiaNests / m_MaxOsmiaNests);
		}
	}
	/** \brief Add an occupied nest */
	void IncOsmiaNesting(Osmia_Nest* a_nest)
	{
		m_NestList.push_front(a_nest);
		a_nest->m_nest_handle = m_NestList.begin(); // remember where it is -> O(1) removal later
		a_nest->m_nest_handle_set = true;
		m_CurrentOsmiaNests++;
		m_HasRoom = (m_CurrentOsmiaNests < m_MaxOsmiaNests); // may have just filled up
	}
	/** \brief Sets the max number of Osmia nests for this LE */
	void SetMaxOsmiaNests(double a_noNests)
	{
		double maxnests = a_noNests * m_Area;
		if (maxnests> 2147000000 ) maxnests= 2147000000;
		m_MaxOsmiaNests = int(maxnests);
		if (m_MaxOsmiaNests < 1) m_MaxOsmiaNests = 0;
		m_HasRoom = (m_CurrentOsmiaNests < m_MaxOsmiaNests); // keep cache consistent with new capacity
	}
	/** \brief Sets the area attribute */
	void SetAreaAttribute(int a_area) { m_Area = a_area; }
	/** \brief Sets the polygon index attribute */
	void SetIndexAttribute(int a_index) { m_Polyindex = a_index; }
	/** \brief Returns the number of current nests */
	int GetNoNests() { return m_CurrentOsmiaNests; }
	/** \brief Returns the max number of nests */
	int GetMaxNoNests() { return m_MaxOsmiaNests; }
	/** \brief Returns the number of current nests */
	bool SanityCheck() { return m_CurrentOsmiaNests == static_cast<int>(m_NestList.size()); } // list::size() is O(1)
	/** \brief Checks for zero cell nests */
	bool SanityCheck2() {
		for (auto it = m_NestList.begin(); it!=m_NestList.end(); ++it)
		{
			if((*it)->ZeroCells()) 
			{
				return false;
			}
		}  

		return true;
	}
};

/**
 * \class Osmia_Nest_Manager
 * \brief Landscape-wide nest-site availability: how many cavities each polygon offers, and how many
 *        are currently occupied.
 *
 * <b>Biological Basis:</b>
 * Solitary bees nest in pre-existing cavities - hollow stems, beetle holes, artificial nest boxes -
 * whose abundance varies with landscape element type. Nest-site limitation is a landscape property
 * rather than a property of any individual bee, which is why it is managed here and not in
 * Osmia_Female.
 *
 * <b>Implementation Approach:</b>
 * InitOsmiaBeeNesting() reads per-habitat densities from the file named by
 * @c cfg_OsmiaNestByLE_Datafile and converts them to a capacity per polygon, held in OsmiaPolygonEntry.
 * Occupancy is claimed when a female settles and released when the nest is abandoned or the
 * offspring emerge. Access is guarded per nest by an OpenMP lock.
 *
 * <b>Key Assumptions:</b>
 * - Cavities within a polygon are interchangeable; only their number matters.
 * - Capacity is fixed for the run and does not respond to management or season.
 *
 * <b>Limitations:</b>
 * - The nest-density parameterisation is calibrated rather than literature-derived and is not
 *   empirically grounded in its absolute magnitude. Nest limitation should be read as a relative
 *   constraint, not an absolute one.
 *
 * \note InitOsmiaBeeNesting() applies a developer-confirmed x0.001 operational scaling factor to
 *       every density read from the input file. Realised nest densities are therefore three orders
 *       of magnitude below the unscaled values, which must be considered when interpreting absolute
 *       nest numbers.
 *
 * \see OsmiaPolygonEntry, Osmia_Nest, OsmiaNestData
 * \par References:
 * - Ziolkowska, E., Bednarska, A.J., Laskowski, R. & Topping, C.J. (2023) The Formal Model for the
 *   solitary bee <i>Osmia bicornis</i> L. agent-based model. <i>Food and Ecological Systems
 *   Modelling Journal</i> 4: e102102. https://doi.org/10.3897/fmj.4.102102
 */
class Osmia_Nest_Manager
{
public:
	/** \brief Osmia nest manager constructor */
	Osmia_Nest_Manager()
	{
		;
	}
	/** \brief Osmia nest manager destructor */
	~Osmia_Nest_Manager()
	{
		int nopolys = m_PolyList.size();
		for (int e = 0; e < nopolys; e++) {
			omp_destroy_nest_lock(m_PolyListLocks[e]);
			delete m_PolyListLocks[e];
		}
		m_PolyList.clear();
	}
	/** \brief Read in the Osmia nest density files and allocate to each LE object */
	void InitOsmiaBeeNesting();
	/** \brief Tell all LE objects to update their osmia nest status */
	void UpdateOsmiaNesting() {
		/**
		Loops through all landscape element objects and updates their Osmia nesting status
		*/
		for (unsigned int s = 0; s < m_PolyList.size(); s++) {
			m_PolyList[s].UpdateOsmiaNesting();
		}
	}
	// Fix 2B: m_PolyListLocks[poly] is now the SINGLE lock for all per-polygon nest-list access
	// (create / release / is-possible). Previously only ReleaseOsmiaNest took it while CreateNest and
	// IsOsmiaNestPossible touched the same m_CurrentOsmiaNests/m_NestList unlocked (a data race), and the
	// caller wrappers additionally took the landscape POLYGON lock -- the same lock 50k bees hold in
	// Forage -- so nest ops serialised behind foraging. The wrappers no longer take the polygon lock, so
	// this dedicated nest lock decouples nest management from foraging AND closes the race.
	/** \brief Find out whether an osmia nest can be made here.
	 * Intentionally LOCK-FREE: this is called in a tight retry loop in FindNestLocation (up to
	 * ~20 tries x 8 directions per bee per day), and it only reads m_CurrentOsmiaNests/m_MaxOsmiaNests
	 * (a plain int comparison) plus a per-thread RNG draw. A torn read of m_CurrentOsmiaNests at worst
	 * makes a bee occasionally mis-judge availability by one -- harmless in this probabilistic model,
	 * and the authoritative m_CurrentOsmiaNests++ happens under lock in CreateNest/IncOsmiaNesting.
	 * Locking it (measured) exploded acquisitions 1.6M->286M and PolyList wait 412s->2831s, so we don't. */
	bool IsOsmiaNestPossible(int a_polyindex)
	{
		return m_PolyList[a_polyindex].IsOsmiaNestPossible();
	}
	/** \brief Deterministic capacity gate (no RNG). True iff this polygon still has free nest capacity.
	 * Lock-free for the same reason as IsOsmiaNestPossible: a torn read of the cached bool at worst makes a
	 * bee mis-judge by one for one attempt; the authoritative fill happens under lock in Inc/Release. */
	bool HasRoom(int a_polyindex)
	{
		return m_PolyList[a_polyindex].HasRoom();
	}
	/** \brief Create the osmia nest here  */
	Osmia_Nest* CreateNest(int a_x, int a_y, int a_polyindex)
	{
		Osmia_Nest* a_nest = new Osmia_Nest(a_x, a_y,a_polyindex, this);
		{ LP_WAIT(LPK_PolyList); omp_set_nest_lock(m_PolyListLocks[a_polyindex]); }
		m_PolyList[a_polyindex].IncOsmiaNesting(a_nest);
		omp_unset_nest_lock(m_PolyListLocks[a_polyindex]);
		return a_nest;
	}
	/** \brief release nest here  */
	void ReleaseOsmiaNest(int a_polyindex, Osmia_Nest* a_nest)
	{
		{ LP_WAIT(LPK_PolyList); omp_set_nest_lock(m_PolyListLocks[a_polyindex]); }
		m_PolyList[a_polyindex].ReleaseOsmiaNest(a_nest);
		omp_unset_nest_lock(m_PolyListLocks[a_polyindex]);
	}
	/** \brief Is a nest possible in this polytype */
	bool GetNestPossible(TTypesOfLandscapeElement index) { return m_PossibleNestType[int(index)]; }

		/** @brief Returns the current occupied-nest count for a polygon. */
		int GetNoNests(int a_polyindex) {
		return m_PolyList[a_polyindex].GetNoNests(); 
	}

		/** @brief Tests whether the cached nest count equals the nest-list size for a polygon. */
		bool SanityCheck(int a_polyindex) {
		return m_PolyList[a_polyindex].SanityCheck();
	}

		/**
		 * @brief Runs the nest-cell diagnostic across every polygon entry.
		 * @warning Osmia_Nest::ZeroCells() returns true for a non-empty nest, so the current diagnostic
		 *          returns false when it encounters a normal occupied nest. Its return value is ignored.
		 */
		bool CheckZeroNests() {
		for (unsigned int s = 0; s < m_PolyList.size(); s++) {
			if (!m_PolyList[s].SanityCheck2()) return false;
		}
		return true;
	}
protected:
	vector<OsmiaPolygonEntry> m_PolyList;
	vector<omp_nest_lock_t*> m_PolyListLocks;
	/** \brief Holds a set of flags indicating whether an Osmia nest is possible (true) or not (false) */
	bool m_PossibleNestType[tole_Foobar];
};

/**
 * \class Osmia_Population_Manager
 * \brief Owns the <i>Osmia</i> population and drives the daily schedule for every individual.
 *
 * <b>Biological Basis:</b>
 * Population dynamics in this model are emergent: there is no population-level equation. This class
 * exists to give individuals a consistent order of execution and to hold the few quantities that are
 * genuinely properties of the population rather than of a bee.
 *
 * <b>Implementation Approach:</b>
 * Implements the ALMaSS scheduling contract through DoFirst(), DoBefore(), DoAfter() and DoLast().
 * DoFirst() updates the day's temperature and season state before any individual steps, which is
 * what makes the static temperature in Osmia_Base safe to read. Sex-ratio and cocoon-mass surfaces
 * are pre-computed once into lookup tables indexed by maternal mass class and age, rather than
 * evaluated per egg. Nest availability is delegated to Osmia_Nest_Manager.
 *
 * <b>Key Assumptions:</b>
 * - Within a phase, the order of individuals does not affect the outcome; where it might, access is
 *   guarded by per-nest locks.
 *
 * <b>Limitations:</b>
 * - Population-level behaviour has not been calibrated or validated, so population-scale
 *   predictions should not yet be treated as quantitative.
 *
 * \warning Stage counts are exposed to the framework's population reporting through m_ListNames in a
 *          fixed order, which must match the TTypeOfOsmiaLifeStages enumeration. Changing one
 *          without the other silently mislabels every subsequent output column.
 *
 * \see Osmia_Base, Osmia_Female, Osmia_Nest_Manager
 * \par References:
 * - Topping, C.J., Hansen, T.S., Jensen, T.S., Jepsen, J.U., Nikolajsen, F. & Odderskaer, P. (2003)
 *   ALMaSS, an agent-based model for animals in temperate European landscapes. <i>Ecological
 *   Modelling</i> 167(1-2): 65-82. https://doi.org/10.1016/S0304-3800(03)00173-X
 * - Ziolkowska, E., Bednarska, A.J., Laskowski, R. & Topping, C.J. (2023) The Formal Model for the
 *   solitary bee <i>Osmia bicornis</i> L. agent-based model. <i>Food and Ecological Systems
 *   Modelling Journal</i> 4: e102102. https://doi.org/10.3897/fmj.4.102102
 */
class Osmia_Population_Manager : public Population_Manager
{
public:
	//code used for pestiside store
	#ifdef __OSMIA_PESTICIDE_STORE
	unsigned int m_female_count = 0;
	#endif

	// Methods
	   /** \brief Osmia_Population_Manager Constructor */
	Osmia_Population_Manager(Landscape* L);
	/** \brief Used to collect data initialisation together */
	void Init();
	/** \brief Osmia_Population_Manager Destructor */
	virtual ~Osmia_Population_Manager(void);
	/** \brief Output method */
	void TheAOROutputProbe() override;
	/** \brief Method for creating a new individual Osmia */
	void CreateObjects(TTypeOfOsmiaLifeStages ob_type, TAnimal* pvo, struct_Osmia* data, int number);

	#ifdef __OSMIATESTING
		/** \brief Add a new egg production to the stats record */
		void RecordEggProduction(int a_eggs);
		/** @brief Records one completed egg-stage duration in days. */
		void RecordEggLength(int a_length);
		/** @brief Records one completed larval-stage duration in days. */
		void RecordLarvalLength(int a_length);
		/** @brief Records one completed prepupal-stage duration in days. */
		void RecordPrePupaLength(int a_length);
		/** @brief Records one completed pupal-stage duration in days. */
		void RecordPupaLength(int a_length);
		/** @brief Records one completed adult-in-cocoon-stage duration in days. */
		void RecordInCocoonLength(int a_length);
		/** @brief Adds one emerging female mass observation, in milligrams, to the testing record. */
		void RecordFemaleWeight(double a_mass) { m_FemaleWeights.push_back(a_mass); };
		/** @brief Acquires the lock protecting the female-mass testing record. */
		void SetFemaleWeightLock(void) {omp_set_nest_lock(m_female_weight_record_lock);}
		/** @brief Releases the lock protecting the female-mass testing record. */
		void ReleaseFemaleWeightLock(void) {omp_unset_nest_lock(m_female_weight_record_lock);}
	#endif
	
	/** \brief Returns flag to denore the end of prewintering, if ended it is set to true */
	bool IsEndPreWinter() { return m_PreWinteringEndFlag; }
	/** \brief Returns flag to denore the end of overwintering, if ended it is set to true */
	bool IsOverWinterEnd() { return m_OverWinterEndFlag; }
	/** \brief Checks whether a nest is possible here */
	bool IsOsmiaNestPossible(int a_polyindex) { return m_OurOsmiaNestManager.IsOsmiaNestPossible(a_polyindex); }
	/** \brief Deterministic capacity gate (no RNG) - true iff polygon has free nest capacity. */
	bool HasRoom(int a_polyindex) { return m_OurOsmiaNestManager.HasRoom(a_polyindex); }
	/** \brief Performs a sanity check on nest count for the given polygon */
	bool SantityCheck(int a_polyindex) { return m_OurOsmiaNestManager.SanityCheck(a_polyindex); }
	/** \brief Creates an Osmia_Nest in the polygon referenced by the index polyindex */
	Osmia_Nest* CreateNest(int a_x, int a_y, int a_polyindex) {
		// Fix 2B: no landscape polygon lock here -- OsmiaNestManager::CreateNest takes the dedicated
		// per-poly nest lock (m_PolyListLocks). Decouples nest creation from the Forage polygon lock.
		return m_OurOsmiaNestManager.CreateNest(a_x, a_y, a_polyindex);
	}
	/** \brief Release the given nest in the given polygon. */
	void ReleaseOsmiaNest(int a_polyindex, Osmia_Nest* a_nest) {
		// Fix 2B: no landscape polygon lock here -- OsmiaNestManager::ReleaseOsmiaNest takes the dedicated
		// per-poly nest lock (m_PolyListLocks).
		m_OurOsmiaNestManager.ReleaseOsmiaNest(a_polyindex, a_nest);
	}
	/** \brief Return the Osmia flying weather hours */
	int GetForageHours() { return m_FlyingWeather;	}
	
	/** \brief Returns the values of provisioning parameters for a adult age */
	double GetProvisioningParams(int a_age) {
		return m_NestProvisioningParameters[a_age];
	}
	/** @brief Returns the first-cocoon provisioning mass for a maternal age and mass class.
	 * @details The age lookup is generated through the configured adult lifespan, with a minimum
	 * extent of 60 days.
	 */
	double GetFirstCocoonProvisioningMass(int a_age, int a_massclass)
	{
		return m_FemaleCocoonMassEqns[a_massclass][a_age]-(m_exp_ZeroTo1.Get() * m_FemaleCocoonMassEqns[a_massclass][a_age] * 0.6);
	}
	/** @brief Returns the planned female-egg proportion for a maternal age and mass class.
	 * @details The age lookup is generated through the configured adult lifespan, with a minimum
	 * extent of 60 days.
	 */
	double GetSexRatioEggsAgeMass(int a_massclass, int a_age)
	{
		return m_EggSexRatioEqns[a_massclass][a_age];
	}
	/** \brief Adds a bee to the density grid at a location */
	int AddToDensityGrid(APoint a_loc)
	{
		int index = (a_loc.m_x / 1000) + (a_loc.m_y / 1000) * m_GridExtent;
		m_FemaleDensityGrid[index]++;
		return index;
	}
	/** \brief Adds a bee to the density grid using the grid index value */
	void AddToDensityGrid(int a_index)
	{
		m_FemaleDensityGrid[a_index]++;
	}
	/** \brief Adds a bee to the density grid using the grid index value */
	void RemoveFromDensityGrid(int a_index)
	{
		m_FemaleDensityGrid[a_index]--;
	}
	/** \brief Gets the number of bees at a location */
	int GetDensity(APoint a_loc)
	{
		int index = (a_loc.m_x / 1000) + (a_loc.m_y / 1000) * m_GridExtent;
		return m_FemaleDensityGrid[index];
	}
	/** \brief Gets the number of bees at a location based on the grid index value */
	int GetDensity(int a_index)
	{
		return m_FemaleDensityGrid[a_index];
	}
	/** \brief Empties the bee density grid */
	void ClearDensityGrid()
	{
		for (int i=0; i< m_FemaleDensityGrid.size(); i++) m_FemaleDensityGrid[i] = 0;
	}
	/** \brief Returns the amount of development a pre-pupal bee will get today based on temperature */
	double GetPrePupalDevelDays() {
		return m_PrePupalDevelDaysToday;
	}
	/** \brief The function to calculate the available foraging hours per day.*/
	void CalForageHours(void);
	/** \brief The function to add one death for the given life stage and thread id. */
	void AddOneDeath(int a_os_type, int a_thread_id) { m_DeadPopulation[a_os_type][a_thread_id]++; } 
	/** \brief The function to add one death because of pesticide for the given life stage and thread id. */
	void AddOneDeathPesticide(int a_os_type, int a_thread_id) { m_DeadPopulationPesticide[a_os_type][a_thread_id]++; }
	/** \brief The function to write the population dynamics. */
	void WritePopulationDynamics();
	//DEBUG//
	#ifdef __OSMIATESTING
public:
	ofstream m_eggsfirstnest;
	double m_egghistogram[4][30];
		/** @brief Writes paired target and achieved first-nest testing records. */
		void WriteNestTestData(OsmiaNestData a_target, OsmiaNestData a_achieved);
#endif // __OSMIATESTING
protected:
	// Attributes
	/** \brief Static instance of the probability_distribution class of distance probabilities for approx exponential probs zero to 1.0 */
	static probability_distribution  m_exp_ZeroTo1;
	#ifdef __OSMIATESTING
	/** \brief A vector for holding female emergence weights */
	vector<double> m_FemaleWeights;
	omp_nest_lock_t *m_female_weight_record_lock;
	/** \brief A class for holding the stats on Osmia egg production */
	SimpleStatistics m_OsmiaEggProdStats;
	/** \brief A class for holding the stats on Osmia egg stage length */
	SimpleStatistics m_EggStageLength;
	/** \brief A class for holding the stats on Osmia larval stage length*/
	SimpleStatistics m_LarvalStageLength;
	/** \brief A class for holding the stats on Osmia pupal stage length*/
	SimpleStatistics m_PrePupaStageLength;
	SimpleStatistics m_PupaStageLength;
	/** \brief A class for holding the stats on Osmia in-cocoon stage length */
	SimpleStatistics m_InCocoonStageLength;
	#endif
	/** \brief A pointer to the pollen map object */
	PollenMap_centroidbased* m_ThePollenMap;
	/** \brief A daily value  to indicate the hours the weather is OK for Osmia adult activity */
	int m_FlyingWeather;
	/** \brief A flag to signal the prewinter phase is over */
	bool m_PreWinteringEndFlag;
	/** \brief A flag to signal the period between onset of wintering and March 1st */
	bool m_OverWinterEndFlag;
	/** \brief This provides the interface to the Osmia_Nests linked to the polgons 
	* - it duplicates some functionality of the Landscape but is held here to prevent bloating of landscape code/footprint for other models
	*/
	Osmia_Nest_Manager m_OurOsmiaNestManager;
	/** \brief A lookup table used to store pre-calculated nest provisioning parameters in advance since they are CPU intensive */
	double m_NestProvisioningParameters[365];
	/** \brief Holds logistic equation values per age/mass of female for egg sex ratios */
	vector<eggsexratiovsagelogisticcurvedata> m_EggSexRatioEqns;
	/** \brief Holds logistic equation values per age/mass of female for first female cocoon mass */
	vector<femalecocoonmassvsagelogisticcurvedata>  m_FemaleCocoonMassEqns;
	/** \brief holds numbers of females per 1km2 */
	vector<int> m_FemaleDensityGrid;
	/** \brief holds numbers of grid cells per row */
	int m_GridExtent;
	/** \brief An attribute used to scale the available pollen based on assumed competetion from other bee species */
	double m_PollenCompetitionsReductionScaler;
	/** \brief Coefficients of the prepupal development rate quadratic, cached for fast access.
	* See the @c cfg_OsmiaPrepupalRateA declaration for the functional form and its provenance. */
	double m_PrePupalRateA;
	double m_PrePupalRateB;
	double m_PrePupalRateC;
	/** \brief q(Topt), the numerator of the normalised prepupal rate. Precomputed in
	* Osmia_Population_Manager::Init() because it is constant for the whole run. */
	double m_PrePupalRateQOpt;
	/** \brief Holds the prepupal development rate for today for fast access */
	double m_PrePupalDevelDaysToday;
	/** \brief Monthly pollen and nectar thresholds loaded at initialisation but not used by current behaviour. */
	vector< OsmiaPollenNectarThresholds> m_PN_thresholds;

	/** \brief The vector to store the daily dead population. */
	std::vector<std::vector<int>> m_DeadPopulation;
	/** \brief The vector to store the daily dead population because of pesticide. */
	std::vector<std::vector<int>> m_DeadPopulationPesticide;
	/** \brief The vector to store the daily new born population. */
	std::vector<std::vector<int>> m_NewBornPopulation;
	/** \brief The file pointer to store the population dynamics for new-born and death. */
	std::ofstream* m_population_dynamics_file{};

	// Methods
    /** \brief  Things to do before anything else at the start of a timestep  */
	virtual void DoFirst();
	/** \brief Things to do before the Step */
	virtual void DoBefore();
	/** \brief Things to do before the EndStep */
	virtual void DoAfter() {}
	/** \brief Things to do after the EndStep - here calculations related to summing global day degrees */
	virtual void DoLast()
	{
		int today = m_TheLandscape->SupplyDayInYear();
		if (today > September)
		{
			int day = g_date->OldDays() + g_date->DayInYear();
			double t0 = m_TheLandscape->SupplyTempPeriod(day, 1);
			/**
			* If the PostPreWinteringFlag is not set and its after Sept 1st then we need to test for the end of pre-wintering
			*/
			if (!m_PreWinteringEndFlag)
			{
				double t1 = m_TheLandscape->SupplyTempPeriod(day - 1, 1);
				double t2 = m_TheLandscape->SupplyTempPeriod(day - 2, 1);
				double t3 = m_TheLandscape->SupplyTempPeriod(day - 3, 1);
				double t4 = m_TheLandscape->SupplyTempPeriod(day - 4, 1);
				double t5 = m_TheLandscape->SupplyTempPeriod(day - 5, 1);
				/** Based on checking for a sustained and stable drop in autumn temperature */
				if (((t2 < 13.0) && (t1 < 13.0) && (t0 < 13.0)) && (((t5 - t4 > 1.0) && (t4 - t3 > 1.0)) || ((t3 < 13.0) && (t5 - t4 >= 3.0))))
				{
					m_PreWinteringEndFlag = true;
				}
			}
		}
		if (today == March)
		{
			m_OverWinterEndFlag = true;
		}
		if (today == June)
		{
			// No emergence after April so we can safely reset this flag here.
			m_PreWinteringEndFlag = false;
			m_OverWinterEndFlag = false;
		}

		WritePopulationDynamics();

#ifdef __OSMIARECORDFORAGE
		double meanforage = 0.0;
		if (Osmia_Female::m_foragecount > 0) meanforage = Osmia_Female::m_foragesum / Osmia_Female::m_foragecount;
		cout << meanforage << endl;
		Osmia_Female::m_foragesum = 0.0;
		Osmia_Female::m_foragecount = 0.0;
#endif

#ifdef __OSMIATESTING		
		if (today == 364) {
			ofstream file1("OsmiaStageLengths.txt", ios::app);
			file1 << "Year: " << g_date->GetYear() << endl;
			file1 << "Mean egg stage days is:" << '\t'  << m_EggStageLength.get_meanvalue() << endl;
			file1 << "Mean larval stage days is:" << '\t' << m_LarvalStageLength.get_meanvalue() << endl;
			file1 << "Mean prepupal stage days is:" << '\t' << m_PrePupaStageLength.get_meanvalue() << endl;
			file1 << "Mean pupal stage days is:" << '\t' << m_PupaStageLength.get_meanvalue() << endl;
			file1 << "Mean incocoon stage days is:" << '\t' << m_InCocoonStageLength.get_meanvalue() << endl;
			m_EggStageLength.ClearData();
			m_LarvalStageLength.ClearData();
			m_PrePupaStageLength.ClearData();
			m_PupaStageLength.ClearData();
			m_InCocoonStageLength.ClearData();
			file1.close();
		}
#endif
	}
};

#endif
