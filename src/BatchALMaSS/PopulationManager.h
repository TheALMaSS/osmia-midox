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
*/ /**
\file
\brief
<B>PopulationManager.h This is the header file for the population manager and associated classes</B> \n
*/
/**
\file
 Modified by Chris J. Topping  and Xiaodong Duan\n
 Version of Feb. 2024 \n
 \n
*/
//---------------------------------------------------------------------------

#ifndef PopulationManagerH
#define PopulationManagerH

#include "../Landscape/pesticide.h"

#include <omp.h>
#include <forward_list>
// Forwards
class TAnimal;
class AlleleFreq;
class AOR_Probe;

// Start defines
using AnsiString = char*;
using TListOfAnimals = vector<TAnimal*>;
// END defines

//------------------------------------------------------------------------------
/**
* \brief An enum to hold all the possible types of population handled by a Population_Manager class
*/
enum TTypesOfPopulation :int {
	TOP_NoSpecies = -1,
	TOP_Skylark = 0,
	TOP_Vole,
	TOP_Erigone,
	TOP_Bembidion,
	TOP_Hare,
	TOP_Partridge,
	TOP_Goose,
	TOP_RoeDeer,
	TOP_Rabbit,
	TOP_Newt,
	TOP_Osmia,
	TOP_ApisRAM,
	TOP_Oedothorax,
	TOP_Bombus, // RESERVED: Bombus removed from the build (not compiled/selectable). Kept to preserve the
	            // integer value of every TOP_ below it, since SIMULATION_SPECIES maps to these ordinals.
	TOP_Aphid,
	TOP_Ladybird,
	//17
	TOP_PoecilusCupreus,
	TOP_Collembola,
	// Above this line are used for menu items and have to be in this order and present
	TOP_Predators,
	TOP_Hunters,
	TOP_OsmiaParasitoids,
	TOP_Beetle,
	TOP_Disturbers,
	TOP_foobar
};

//------------------------------------------------------------------------------

enum to_BeforeStepActions :unsigned {
	toBSA_Shuffle = 0,
	toBSA_SortX,
	toBSA_SortY,
	toBSA_Nothing,
	toBSA_ShuffleEvery500,
	toBSA_foobar
};

/**
\brief
A struct of 100 ints
*/
struct IntArray100 {
public:
	int n[100];
};

//------------------------------------------------------------------------------

/**
\brief
A struct defining two x,y coordinate sets of positive co-ords only
*/
struct rectangle {
public:
	unsigned m_x1;
	unsigned m_y1;
	unsigned m_x2;
	unsigned m_y2;
};

//------------------------------------------------------------------------------

/**
\brief
The base class of all ALMaSS objects requiring Step code
*/
class TALMaSSObject {
protected:
	/** \brief The basic state number for all objects - '-1' indicates death */
	int m_CurrentStateNo;
	/** \brief Indicates whether the iterative step code is done for this timestep */
	bool m_StepDone;

public:
	/** \brief Returns the current state number */
	int GetCurrentStateNo() const { return m_CurrentStateNo; }
	/** \brief Sets the current state number */
	void SetCurrentStateNo(int a_num) { m_CurrentStateNo = a_num; }
	/** \brief Returns the step done indicator flag */
	bool GetStepDone() const { return m_StepDone; }
	/** \brief Sets the step done indicator flag */
	void SetStepDone(bool a_bool) { m_StepDone = a_bool; }
	/** \brief BeingStep  behaviour - must be implemented in descendent classes */
	virtual void BeginStep() {
	}
	/** \brief Step  behaviour - must be implemented in descendent classes */
	virtual void Step() {
	}
	/** \brief EndStep  behaviour - must be implemented in descendent classes */
	virtual void EndStep() {
	}
	/** \brief Used to initialise an object */
	void ReinitialiseObjectBase() {
		m_StepDone = false;
		m_CurrentStateNo = 0;
	}
	/** \brief The constructor for TALMaSSObject */
	TALMaSSObject();
	/** \brief The destructor for TALMaSSObject */
	virtual ~TALMaSSObject() = default;
	/** \brief Used for debugging only, tests basic object properties */
	static void OnArrayBoundsError();
};

//------------------------------------------------------------------------------

/** \brief A class defining an animals position */
class AnimalPosition
	// used to communicate the position of an animal to inquiring objects
{
public:
	unsigned m_x{0};
	unsigned m_y{0};
	TTypesOfLandscapeElement m_EleType{tole_Foobar};
	TTypesOfVegetation m_VegType{tov_Undefined};
};

//------------------------------------------------------------------------------

/**
* \brief Part of the basic ALMaSS system (obselete) 
* Communicates the range centre, age and size of animals to other objects
*/
class RoeDeerInfo : public AnimalPosition {
public:
	double m_Size{0};
	int m_Age{0};
	APoint m_Range;
	APoint m_OldRange;
};

//------------------------------------------------------------------------------

/**
\brief
The base class for all ALMaSS animal classes.
Includes all the functionality required to be handled by classes derived from Population_Manager, hence a number of empty methods that MUST be reimplemented in descendent classes e.g. CopyMyself()
*/
class TAnimal : public TALMaSSObject {
public:
	/** \brief The TAnimal constructor saving the x,y, location and the landscape pointer */
	TAnimal(int x, int y, Landscape* L);
	/** \brief The TAnimal constructor saving the x,y used if landscape is already set */
	TAnimal(int x, int y);
	/** \brief Set the guard map index, this is used to avoid two animals operating in the same location when using multithread */
	void SetGuardMapIndex(int a_index_x, int a_index_y) {
		m_guard_cell_x = a_index_x;
		m_guard_cell_y = a_index_y;
	}
	/**
	 * \brief Get the current location farm ref if any
	 */
	unsigned SupplyFarmOwnerRef() const;
	/**
	 * \brief Returns the objects location and habitat type and veg type
	 */
	AnimalPosition SupplyPosition() const;
	/**
	 * \brief Returns the objects location in ALMaSS coordinates
	 */
	APoint SupplyPoint() const {
		APoint p(m_Location_x, m_Location_y);
		return p;
	}
	/**
	 * \brief Returns the polygon reference where the object is located
	 */
	int SupplyPolygonRef() const { return m_OurLandscape->SupplyPolyRef(m_Location_x, m_Location_y); }
	/**
	 * \brief Returns the polygon type where the object is located
	 */
	TTypesOfLandscapeElement SupplyPolygonType() const {
		return m_OurLandscape->GetOwner_tole(m_Location_x, m_Location_y);
	}
	/**
	 * \brief Returns the ALMaSS x-coordinate
	 */
	int Supply_m_Location_x() const { return m_Location_x; }
	/**
	 * \brief Returns the ALMaSS y-coordinate
	 */
	int Supply_m_Location_y() const { return m_Location_y; }
	/**
	 * \brief Returns the x-index to the guard cell
	 */
	int SupplyGuardCellX() const { return m_guard_cell_x; }
	/**
	 * \brief Returns the y-index to the guard cell
	 */
	int SupplyGuardCellY() const { return m_guard_cell_y; }
	/**
	 * \brief Returns the animals age in days
	 */
	int SupplyAge() const { return m_AgeDays; }
	/**
	 * \brief Sets the animals age in days
	 */
	void SetAge(int a_age) { m_AgeDays = a_age; }
	/**
	 * \brief Sets all parameters ready for object destruction
	 */
	virtual void KillThis() {
		m_CurrentStateNo = -1;
		m_StepDone = true;
		// NB: pesticide/toxicity state lives in TAnimalToxicity (bee species only); its KillThis override
		// resets m_my_pesticide. TAnimal itself no longer carries a PesticideToxicity member.
	}
	/**
	 * \brief Used to copy the object details to another in descendent classes
	 */
	virtual void CopyMyself() { }
	/**
	 * \brief Sets the x-coordinate
	 */
	void SetX(const int a_x) { m_Location_x = a_x; }
	/**
	 * \brief Sets the y-coordinate
	 */
	void SetY(const int a_y) { m_Location_y = a_y; }
		virtual void ReinitialiseObject(int a_x, int a_y, Landscape* a_l_ptr) {
		m_OurLandscape = a_l_ptr;
		m_Location_x = a_x;
		m_Location_y = a_y;
		m_AgeDays = 0;
		ReinitialiseObjectBase();
	}
	/**
	 * 	\brief Used to re-use an object - must be implemented in descendent classes 
	 */
	virtual void ReinitialiseObject(int a_x, int a_y) {
		m_Location_x = a_x;
		m_Location_y = a_y;
		m_AgeDays = 0;
		ReinitialiseObjectBase();
	}
	/**
	 * \brief Returns the objects current state number
	 */
	virtual int WhatState() { return 0; }
	/**
	 * \brief A wrapped for KillThis - ideally should not be used
	 */
	virtual void Dying() { KillThis(); }
	/**
	 * \brief Used to start a check for any management related effects at the objects current location
	 */
	void CheckManagement();
	/**
	 * \brief Used to start a check for any management related effects at x,y
	 */
	void CheckManagementXY(int a_x, int a_y);
	/**
	 * \brief Must be reimplemented if used in descendent classes. Sets the action on a management event
	 */
	virtual bool OnFarmEvent(FarmToDo /* event */) { return false; }
	/**
	 * \brief Sets the simulation width
	 */
	static void SetSimulationWidth(int a_value) { m_SimulationWidth = a_value; }
	/**
	 * \brief Sets the simulation height
	 */
	static void SetSimulationHeight(int a_value) { m_SimulationHeight = a_value; }
	/**
	 * \brief Sets the day in year attribute
	 */
	static void SetDayInYear(int a_value) { m_DayInYear = a_value; }
	/**
	 * \brief Sets the landscape pointer
	 */
	static void SetOurLandscape(Landscape* a_value) { m_OurLandscape = a_value; }
	/**
	 * \brief Sets the temperature today attribute
	 */
	static void SetTempToday(double a_value) { m_TemperatureToday = a_value; }
	/** \brief The function to do contact pesticide. */
	virtual void DoPesticideContact(int a_x = -1, int a_y = -1) {;}
	/** \brief Supply the vitality value.*/
	virtual double SupplyVitality() { return -1; }
protected:
	/**
	 * \brief A pointer to the landscape object shared with all TAnimal objects
	 */
	static Landscape* m_OurLandscape;
	/**
	 * \brief A static member for the simulation width because it is often used by descendent classes
	 */
	static int m_SimulationWidth;
	/**
	 * \brief A static member for the simulation height because it is often used by descendent classes
	 */
	static int m_SimulationHeight;
	/**
	 * \brief A holder for the temperature today shared with all TAnimal objects
	 */
	static double m_TemperatureToday;
	/**
	 * \brief A holder for the day in year shared with all TAnimal objects
	 */
	static int m_DayInYear;
	/**
	 * \brief The objects ALMaSS x coordinate
	 */
	int m_Location_x;
	/**
	 * \brief The objects ALMaSS y coordinate
	 */
	int m_Location_y;
	/**
	 * @brief The index x to the guard cell
	 */
	int m_guard_cell_x;
	/**
	 * @brief The index y to the guard cell
	 */
	int m_guard_cell_y;
	/**
	 * \brief To hold the age in days
	 */
	int m_AgeDays {0};
	/**
	 * \brief Corrects wrap around co-ordinate problems
	 */
	void CorrectWrapRound() {
		/**
		Does the standard wrap around testing of positions. Uses the addition and modulus operators to avoid testing for negative or > landscape extent.
		This would be an alternative that should be tested for speed at some point.
		However, the subtraction method will fail if x is >=2*width or y >= 2*height
		*/
		m_Location_x = (m_Location_x + m_OurLandscape->SupplySimAreaWidth()) % m_OurLandscape->SupplySimAreaWidth();
		m_Location_y = (m_Location_y + m_OurLandscape->SupplySimAreaHeight()) % m_OurLandscape->SupplySimAreaHeight();
	}
};

//------------------------------------------------------------------------------
// TAnimalToxicity (a TAnimal that carries pesticide/toxicity state) is defined beside this class in
// BatchALMaSS/PesticideToxicity.h. Species with pesticide exposure behaviour derive from it; species
// without that behaviour derive from TAnimal directly.
//------------------------------------------------------------------------------

/**
* \brief
* Test the given animal's x-coordinate against the given x-coordinate
*/
class ReturnLessThanX {
public:
	bool operator()(TAnimal* a_a1, int a_x) const { return a_a1->Supply_m_Location_x() < a_x; }
};

//---------------------------------------------------------------------------

/**
* \brief
* Test the given animal's y-coordinate against the given y-coordinate
*/
class ReturnMoreThanX {
public:
	bool operator()(TAnimal* a_a1, int a_x) const { return a_a1->Supply_m_Location_x() > a_x; }
};
//---------------------------------------------------------------------------

/**
* \brief
* Data structure to hold & output probe data probe data is designed to be used to return the number of objects in a given area or areas in specific element or vegetation types or farms
*/
class Probe_Data {
protected:
	ofstream* m_MyFile;
	int m_Time;
	char m_MyFileName[255];

public:
	bool m_FileRecord;
	bool m_FullLandscapeProbe;
	// true if the probe is a full landscape probe, then no need to check the position and we can just ask PM for numbers: most of the time is like that
	unsigned m_ReportInterval; // from 1-10
	unsigned m_NoAreas; // from 1-10
	rectangle m_Rect[16]; // can have up to thirty two areas
	unsigned m_NoEleTypes;
	unsigned m_NoVegTypes;
	unsigned m_NoFarms;
	TTypesOfVegetation m_RefVeg[25]; // up to 25 reference types
	TTypesOfLandscapeElement m_RefEle[25]; // up to 25 reference types
	unsigned m_RefFarms[25]; // up to 25 reference types
	// Species Specific Code below:
	bool m_TargetTypes[16]; // eggs,nestlings,fledgelings,males,females etc.
	void FileOutput(int No, int time, int ProbeNo) const;
	void FileAppendOutput(int No, int time) const;
	Probe_Data();
	void SetFile(ofstream* F);
	ofstream* OpenFile(const string& Nme);

	bool OpenForAppendToFile() {
		m_MyFile = new ofstream(m_MyFileName, ios::app);
		if (!(*m_MyFile).is_open())
		{
			g_msg->Warn(WARN_FILE, "PopulationManager::AppendToFile() Unable to open file for append: ", m_MyFileName);
			exit(1);
		}
		return true;
	}
	void CloseFile() const;
	~Probe_Data() = default;
};

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

class SimpleStatistics {
	/**
	* This class is designed to provide the facility to create simple stats from data that comes in incrementally.
	* It can provide the mean, variance of the data set at any point in time
	*/
protected:
	double m_K{0};
	double m_n{0};
	double m_Sum{0};
	double m_SumX{0};
	double m_SumX2{0};

public:
	/** \brief SimpleStatistics constructor */
	SimpleStatistics() = default;
	/** \brief Add a value */
	void add_variable(double x) {
		// This uses the computing shifted data equation. 
		if (m_n < 1) m_K = x;
		m_n++;
		m_Sum += x;
		m_SumX += x - m_K;
		m_SumX2 += (x - m_K) * (x - m_K);
	}
	/** \brief Remove a value */
	void remove_variable(double x) {
		m_n--;
		m_Sum -= x;
		m_SumX -= x - m_K;
		m_SumX2 -= (x - m_K) * (x - m_K);
	}
	/** \brief Returns the number of values */
	double get_N() const { return m_n; }
	/** \brief Returns the mean */
	double get_Total() const { return m_Sum; }
	/** \brief Returns the mean */
	double get_meanvalue() const {
		if (m_n < 1) return -1;
		return m_K + m_SumX / m_n;
	}
	/** \brief Returns the population variance */
	double get_varianceP() {
		if (m_n < 2)
		{
			return -1; // Ilegal n value, but don't want to exit
		}
		return (m_SumX2 - m_SumX * m_SumX / m_n) / m_n;
	}
	/** \brief Returns the sample variance */
	double get_varianceS() {
		if (m_n < 2)
		{
			return -1; // Ilegal n value, but don't want to exit
		}
		return (m_SumX2 - m_SumX * m_SumX / m_n) / (m_n - 1);
	}
	/** \brief Returns the sample standard deviation */
	double get_SD() {
		if (m_n < 2)
		{
			return -1; // Ilegal n value, but don't want to exit
		}
		return sqrt(get_varianceS());
	}
	/** \brief Returns the sample standard error */
	double get_SE() {
		if (m_n < 2)
		{
			return -1; // Ilegal n value, but don't want to exit
		}
		return sqrt(get_varianceS() / get_N());
	}
	/** \brief Clears the data */
	void ClearData() {
		m_K = 0;
		m_n = 0;
		m_Sum = 0;
		m_SumX = 0;
		m_SumX2 = 0;
	}
};

//------------------------------------------------------------------------------
/**
\brief
Base class for all population managers
*/
/**
The core of the handling of animal populations. All time-step code and most input/output is handled by this class and its descendents. This class effectively implements a state machine to facilitate simulation of animal behaviours and handle potential issues with concurrency. The PopulationManager class is never instantiated but must be used by deriving a descendent class.
*/
class Population_Manager_Base {
public:
	// Methods
	/** \brief The Population_Manager_Base constructor */
	Population_Manager_Base(Landscape* L);
	/** \brief Must be re-implemented in descendent classes. Gets the number of 'live' objects */
	virtual unsigned GetLiveArraySize(int a_listindex) { return 0; }
	/** \brief Must be re-implemented in descendent classes */
	virtual unsigned GetPopulationSize(int a_listindex) { return 0; }
	/** \brief Must be re-implemented in descendent classes */
	virtual void Catastrophe(int /* a_mort */) {
	}
	/** \brief Returns landscape width in m */
	int SupplySimW() const { return SimW; }
	/** \brief Returns landscape height in m */
	int SupplySimH() const { return SimH; }
	/** \brief Returns half the landscape width in m */
	int SupplySimWH() const { return SimWH; }
	/** \brief Returns half the landscape height in m */
	int SupplySimHH() const { return SimHH; }
	/** \brief A place holder for the run function for all population managers, this must be reimplemented in descendent classes */
	virtual void Run(int a_NoTSteps) {
		for (int i = 0; i < a_NoTSteps; i++)
		{
		}
	}
	/** \brief A stub to build  specfic probes and functions */
	virtual void SetNoProbesAndSpeciesSpecificFunctions(int /*a_pn*/) {
	}
	/** \brief A stub to build  specfic probes and functions */
	virtual int ProbeFileInput(char* p_Filename, int p_ProbeNo);
	/** \brief A stub to build  specfic reporting */
	virtual char* SpeciesSpecificReporting(int /*a_species*/, int /*a_time*/) { return nullptr; }
	/** \brief Holds a list of pointers to standard output probes */
	Probe_Data* TheProbe[100] = {nullptr}; // lct: Array of pointers
	int SupplyListNameLength() const { return m_ListNameLength; }
	/** \brief Get a list name from the list */
	const char* SupplyListName(int a_i) const { return m_ListNames[a_i]; }
	/** \brief A stub for identifying an individual at a location */
	virtual void SupplyLocXY(unsigned /*listindex */, unsigned /*j*/, int& /*x*/, int& /*y*/) {
	}
	/** \brief A stub for identifying an individual at a location */
	virtual TAnimal* FindClosest(int /*x*/, int /*y*/, unsigned /*Type*/) { return nullptr; }
	std::string SupplySimulationName() { return m_SimulationName; }
	/** \brief An empty method that needs to be here because the population manager lists might contain sub-population models which will skip Population_Manager functionality */
	virtual void OpenTheAOROutputProbe(const string& a_string) {
	}

	/** \brief  Set step counter in a day*/
	void SetStepCounterInADay(int a_counter) {m_step_counter_in_a_day = a_counter;}
	/** \brief  Return the sep counter in a day*/
	int GetStepCounterInADay(void) {return m_step_counter_in_a_day;}
	/** \brief Is it the first step in a day. */
    bool IsFirstStepInaDay(){return m_step_counter_in_a_day==0;}
	/** \brief Supply the step counting in a day. */
    int SupplyStepNumInaDay(){return m_step_counter_in_a_day;}
    /** \brief Supply the maximum number of steps in a day, starting from zero. */
    int SupplyMaxStepNumInaDay(){return m_max_step_in_a_day;}
	/** \brief Set the maximum number of steps in a day.*/
	void SetMaxStepNumInaDay(int a_max_step_in_a_day){m_max_step_in_a_day = a_max_step_in_a_day;}
    /** \brief Is it the last step in a day. */
    bool IsLastStepInaDay(){return m_step_counter_in_a_day==m_max_step_in_a_day;}
	/** \brief Increase the step counter in an hour by 1, resets to zero when it reaches 6.*/
	void IncreaseStepCounterInAnHour(){
		m_step_counter_in_an_hour++;
		if(m_step_counter_in_an_hour == 6){
			m_step_counter_in_an_hour = 0;
			m_hour_in_a_day++;
			if(m_hour_in_a_day == 24)
				m_hour_in_a_day = 0;
		}
	}
	/** \brief Return the step counter in an hour. */
	int GetStepCounterInAnHour(){return m_step_counter_in_an_hour;}
	/** \brief Is it the first step in an hour. */
	bool IsFirstStepInAnHour(){return m_step_counter_in_an_hour==0;}
	/** \brief Is it the last step in an hour. */
	bool IsLastStepInAnHour(){return m_step_counter_in_an_hour==5;}
	/** \brief Supply the current hour for today. */
    int SupplyHourToday(void) {return m_hour_in_a_day;}

	/** \brief Return the aphid density, it is only used when aphid simulations are running. */
	virtual double GetAphidDensity(int /*a_x*/, int /*a_y*/) { return 0; }
	/** \brief Return the animal pointer for the give life stage and number. */
	virtual TAnimal* SupplyAnimalPtr(unsigned int a_index, unsigned int a_animal) { return nullptr; }
	/** \brief Return the guard map cell.*/
	virtual void UnsetGuardMapElement(unsigned a_x_index, unsigned a_y_index) {return;}

	/** \brief Set the given guard map cell.*/
	virtual void  SetGuardMapElement(unsigned a_x_index, unsigned a_y_index) {return;}

protected:
	~Population_Manager_Base() = default;
	/** \brief stores the simulation height */
	int SimH;
	/** \brief stores the simulation width */
	int SimW;
	/** \brief stores the simulation height halved */
	int SimHH;
	/** \brief stores the simulation width halved */
	int SimWH;
	/** \brief stores the simulation name */
	string m_SimulationName;
	/** \brief holds an internal pointer to the landscape */
	Landscape* m_TheLandscape;
	/** \brief the number of life-stages simulated in the population manager */
	int m_ListNameLength{};
	/** \brief A list of life-stage names */
	const char* m_ListNames[32]{};
	/** \brief name of the AOR Probe file */
	std::string m_AORProbeFileName{"AOR_Probe.txt"};
	/** \brief name of the Probe file */
	std::string m_ProbeFileName{"Probe.res"};
	/** \brief Step counter in a day, this is used for the animals with finer step size other than one day.*/
	int m_step_counter_in_a_day = 0;
	/** \brief This is used to record the maximum step number in a day, starting from zero, by default, it is set to be zero. For species running in 10 mins, it is 143. */
    int m_max_step_in_a_day;
	/** \brief This is used to track the step number in an hour. It is used to calculation the hour in a day, this is only used for the species running with 10 mins time step.. */
    int m_step_counter_in_an_hour = 0;
	/** \brief This is used to track the hour in a day (0 to 23) */
    int m_hour_in_a_day = 0;
};
//------------------------------------------------------------------------------

/**
\brief
Base class for all population managers for agent based models
*/
/**
The core of the handling of animal populations. All time-step code and most input/output is handled by this class and its descendents. This class effectively implements a state machine to facilitate simulation of animal behaviours and handle potential issues with concurrency. The PopulationManager class is never instantiated but must be used by deriving a descendent class.
*/
class Population_Manager : public Population_Manager_Base {
public:
	// Methods
	Population_Manager(Landscape* a_l_ptr, int a_numberLifeStages = 12);
	virtual ~Population_Manager(void);

	/** \brief Sets up probe and species specifics */
	void SetNoProbesAndSpeciesSpecificFunctions(int a_pn) override;
	/** \brief Opens the AOR output file */
	void OpenTheAOROutputProbe(const string& a_AORFilename) override;

	/** \brief Gets the number of 'live' objects for a list index in the TheArray */
	unsigned GetLiveArraySize(int a_listindex) override { return m_LiveArraySize[a_listindex]; }
	/** \brief Gets the number of species objects
	 * by default it is the same as LiveArray, however it could be different.
	 * This function should be used/overriden when we are interested in the number of the objects for presentation
	 * and not in inner maintenance of the PopulationManager and or TheArray
	 * */
	unsigned GetPopulationSize(int a_listindex) override { return GetLiveArraySize(a_listindex); }
	/** \brief Increments the number of 'live' objects for a list index in the TheArray */
	void IncLiveArraySize(int a_listindex) { 
		TheSubArraysSizes[a_listindex][omp_get_thread_num()]++;
		#pragma omp atomic
		m_LiveArraySize[a_listindex]++; 
	}
	unsigned int FarmAnimalCensus(unsigned int a_farm, unsigned int a_typeofanimal);
	char* SpeciesSpecificReporting(int a_species, int a_time) override;
	char* ProbeReport(int a_time);
	char* ProbeReportTimed(int a_time);
	void ImpactProbeReport(int a_Time);
	bool BeginningOfMonth();
	void LOG(const char* fname);
	int SupplyStepSize() const { return m_StepSize; }
	void Run(int NoTSteps) override;
	virtual unsigned Probe(int ListIndex, Probe_Data* p_TheProbe);
	virtual void ImpactedProbe();
	vector<vector<forward_list<TAnimal*>*>>* SupplyTheSubArrayPtr(){return &TheSubArrays;}
	/** \brief Returns the pointer indexed by a_index and a_animal. Note NO RANGE CHECK */
	virtual TAnimal* SupplyAnimalPtr(unsigned int a_index, unsigned int a_animal) {
		//reset
		if(a_animal == 0 || a_animal < TheSubArrayPreviousIndex[a_index]){
			//let's find the first non-empty subarray
			TheSubArrayPreviousIndex[a_index] = 0;
			TheSubArrayThreadIndex[a_index] = 0;
			TheSubArraysIterators[a_index] = TheSubArrays[a_index][0]->begin();
			for (int j = 0; j < TheSubArraysSizes[a_index].size(); j++){
				if(TheSubArraysSizes[a_index][j] > 0){
					TheSubArrayThreadIndex[a_index] = j;
					TheSubArraysIterators[a_index] = TheSubArrays[a_index][j]->begin();
					break;
				}
			}
			if(a_animal == 0)
				return *(TheSubArraysIterators[a_index]);
		}

		if(a_animal == TheSubArrayPreviousIndex[a_index]){
			return *(TheSubArraysIterators[a_index]);
		}

		//start the search from right after the previous position
		int temp_animal_num = TheSubArrayPreviousIndex[a_index];
		int temp_thread_num = TheSubArrayThreadIndex[a_index];

		TheSubArraysIterators[a_index]++;
		while(temp_animal_num < a_animal){
			for (auto it = TheSubArraysIterators[a_index]; it != TheSubArrays[a_index][temp_thread_num]->end(); ++it)
			{	
				temp_animal_num++;
				if(temp_animal_num == a_animal){
					TheSubArrayPreviousIndex[a_index] = temp_animal_num;
					TheSubArrayThreadIndex[a_index] = temp_thread_num;
					TheSubArraysIterators[a_index] = it;
					return *it;
				}
			}
			//we need to move to the next thread subarray and search from the beginning
			temp_thread_num++;
			TheSubArraysIterators[a_index] = TheSubArrays[a_index][temp_thread_num]->begin();
		}

		//if reach here means error happens,quite 
		exit(1);
	}

	virtual unsigned SupplyListIndexSize() { return static_cast<unsigned>(TheSubArrays.size()); }
	unsigned SupplyListSize(unsigned listindex) const {
		int return_value = 0;
		for (unsigned i = 0; i < TheSubArraysSizes[listindex].size(); i++)
		{
			return_value += TheSubArraysSizes[listindex][i];
		}
		return return_value; 
	}


	void PushIndividual(const unsigned a_listindex, TAnimal* a_individual_ptr);

	const char* SupplyStateNames(int i) const { return StateNames[i]; }
	unsigned SupplyStateNamesLength() const { return StateNamesLength; }
	virtual void DisplayLocations();


	bool ProbesSet{};
	/** Gets a random live individual returned as TAnimal* */
	TAnimal* FindIndividual(unsigned Type, TAnimal* a_me);
	// used to show when probes are needed to be set
	virtual TAnimal* FindClosest(int x, int y, unsigned Type, TAnimal* a_me);

	/** \brief Return the guard map cell.*/
	omp_nest_lock_t* GetGuardMapElement(unsigned a_x_index, unsigned a_y_index)
	{
		return guardAt(a_x_index, a_y_index);
	}

	/** \brief Set the given guard map cell.*/
	virtual void SetGuardMapElement(unsigned a_x_index, unsigned a_y_index)
	{
		omp_set_nest_lock(guardAt(a_x_index, a_y_index));
	}

	/** \brief Unset the given guard map cell.*/
	virtual void UnsetGuardMapElement(unsigned a_x_index, unsigned a_y_index)
	{
		omp_unset_nest_lock(guardAt(a_x_index, a_y_index));
	}

	/** \brief Move the animal's held guard lock to the cell of its new location (a_x,a_y).
	 * Deadlock avoidance: the caller may already hold OTHER locks (polygon/nest) taken earlier in the same
	 * Step, while another thread holds the target guard cell and is itself blocked on a lock we hold ->
	 * a guard<->polygon/nest cycle. So we NEVER block here: TRY to take the new cell; if it is busy, keep
	 * the animal on its current guard cell for this round (behaviour still moves; only the guard bookkeeping
	 * is deferred, and it self-corrects next Step). This makes the pre-Step lock the ONLY blocking guard
	 * acquisition, and that one is taken while holding no other lock -> no cycle can form. */
	void UpdateGuardMap(int a_x, int a_y, int& a_index_x, int& a_index_y)
	{
		int temp_index_x = a_x / m_guard_cell_size;
		int temp_index_y = a_y / m_guard_cell_size;

		//a new cell, try to move the guard to it without blocking
		if(a_index_x != temp_index_x || a_index_y != temp_index_y){
			if (omp_test_nest_lock(guardAt(temp_index_x, temp_index_y))) {
				omp_unset_nest_lock(guardAt(a_index_x, a_index_y));  // release old only after new is held
				a_index_x = temp_index_x;
				a_index_y = temp_index_y;
			}
			// else: new cell busy -> keep the current guard cell this round (no blocking, no cycle).
		}

	}

protected:
	// Attributes
	// Holds the number of live animals represented in each element of vector of vectors TheArray
	unsigned *m_LiveArraySize;
	int m_NoProbes = 0;

	/** \brief The array to store the flag of finished all loops. */
	std::vector<std::vector<bool>> m_not_finished_flag_vec;

	/** \brief Guard-map locks: one per (guard_cell_size x guard_cell_size) block of the landscape, so two
	 * animals on different threads at the same area cannot act concurrently. Stored FLAT (row-major,
	 * y*width+x) in one contiguous vector -- no per-cell heap allocation, better cache locality than the
	 * former vector<vector<lock*>>. Index via guardAt(x,y). */
	std::vector<omp_nest_lock_t> m_MapGuard;
	int m_guard_cell_height_num;
	int m_guard_cell_width_num;
	int m_guard_cell_size;

	/** \brief Address of the guard lock covering guard-cell (a_x_index, a_y_index). Row-major flat index. */
	inline omp_nest_lock_t* guardAt(int a_x_index, int a_y_index) {
		return &m_MapGuard[(size_t)a_y_index * m_guard_cell_width_num + a_x_index];
	}

	/** \brief This is used to indicate whether the species is paralleled.*/
	bool m_is_paralleled;

	//FILE* m_AlleleFreqsFile;
	//FILE* m_EasyPopRes;
	//FILE* m_FledgelingFile;
	const char* StateNames[100]{};
	int m_StepSize{};
	/** \brief Hold all the animal pointers.*/
	vector<vector<forward_list<TAnimal*>*>> TheSubArrays;
	/** \brief Hold all the animal sizes in each thread.*/
	vector<vector<int>> TheSubArraysSizes;
	/** \brief The vector to store the life stage order for looping specie life stages.*/
	vector<int> m_LifeStageOrderVec;
	/** \brief the iterator from last time called for SupplyAnimalPtr(), this for fast loop all the animals in the derived class. */
	vector<forward_list<TAnimal*>::iterator> TheSubArraysIterators;
	/** \brief the animal index number from last time called for SupplyAnimalPtr(), this for fast loop all the animals in the derived class. */
	vector<int> TheSubArrayPreviousIndex;
	/** \brief the thread index number from last time called for SupplyAnimalPtr(), this for fast loop all the animals in the derived class. */
	vector<int> TheSubArrayThreadIndex;
	unsigned StateNamesLength;
	FILE* TestFile{};
	FILE* TestFile2{};
	/** \brief Holds the season list of possible before step actions.*/
	vector<unsigned> BeforeStepActions;
	/** \brief Holds the season number. Used when running goose and hunter sims.*/
	int m_SeasonNumber;

	/**	\brief Methods to run the simulations.*/
	virtual bool StepFinished();
	virtual void DoFirst();
	virtual void DoBefore();
	virtual void DoAfter();
	virtual void DoLast();
	virtual void DoSpecialBetweenLifeStages(int a_life_stage) {;}
	virtual void SortX(unsigned Type);
	void SortXIndex(unsigned Type);
	virtual void SortY(unsigned Type);
	void SortState(unsigned Type);
	void SortStateR(unsigned Type);
	virtual unsigned PartitionLiveDead(unsigned Type);
	/** \brief Function to check whether there is an empty subarray, if yes, try to refill it.*/
	void CheckEmptySubArray(unsigned Type);
	virtual void Shuffle_or_Sort(unsigned Type);
	virtual void Shuffle(unsigned Type);
	virtual void Catastrophe();

public:
	// Grid related functions
	bool OpenTheRipleysOutputProbe();
	//virtual void OpenTheAOROutputProbe(string a_AORFilename);
	bool OpenTheMonthlyRipleysOutputProbe();
	bool OpenTheReallyBigProbe();
	virtual void TheAOROutputProbe();
	/**   \brief Special output functionality */
	virtual void TheRipleysOutputProbe(ofstream* a_prb);
	virtual void TheReallyBigOutputProbe();
	void CloseTheMonthlyRipleysOutputProbe() const;
	virtual void CloseTheRipleysOutputProbe();
	virtual void CloseTheReallyBigOutputProbe();
	TTypesOfPopulation GetPopulationType() { return m_population_type; }
	/** \brief Get the season number */
	int GetSeasonNumber() { return m_SeasonNumber; }

protected:
	TTypesOfPopulation m_population_type{TOP_NoSpecies};
	ofstream* AOROutputPrb{};
	ofstream* RipleysOutputPrb{};
	ofstream* RipleysOutputPrb1{};
	ofstream* RipleysOutputPrb2{};
	ofstream* RipleysOutputPrb3{};
	ofstream* RipleysOutputPrb4{};
	ofstream* RipleysOutputPrb5{};
	ofstream* RipleysOutputPrb6{};
	ofstream* RipleysOutputPrb7{};
	ofstream* RipleysOutputPrb8{};
	ofstream* RipleysOutputPrb9{};
	ofstream* RipleysOutputPrb10{};
	ofstream* RipleysOutputPrb11{};
	ofstream* RipleysOutputPrb12{};
	ofstream* ReallyBigOutputPrb{};
	/** \brief A pointer to the AOR probe */
	AOR_Probe* m_AOR_Probe{};

	long int lamdagrid[2][257][257]{}; // THIS ONLY WORKS UP TO 10x10 KM !!!!
public:
	void LamdaDeath(int x, int y) {
		// inlined for speed
		lamdagrid[1][x / __lgridsize][y / __lgridsize]++;
	}
	void LamdaBirth(int x, int y) { lamdagrid[0][x / __lgridsize][y / __lgridsize]++; }
	void LamdaBirth(int x, int y, int z) { lamdagrid[0][x / __lgridsize][y / __lgridsize] += z; }
	void LamdaClear() {
		for (int i = 0; i < 257; i++)
		{
			for (int j = 0; j < 257; j++)
			{
				lamdagrid[0][i][j] = 0;
				lamdagrid[1][i][j] = 0;
			}
		}
	}
	void LamdaDumpOutput();
	// end grid stuff
public: // Special ones for compatability to descended managers
	virtual int SupplyPegPosx(int /*unused*/) { return 0; }
	virtual int SupplyPegPosy(int /*unused*/) { return 0; }
	virtual int SupplyCovPosx(int /*unused*/) { return 0; }
	virtual int SupplyCovPosy(int /*unused*/) { return 0; }
	virtual bool OpenTheFledgelingProbe() { return false; }
	virtual bool OpenTheBreedingPairsProbe() { return false; }
	virtual bool OpenTheBreedingSuccessProbe() { return false; }
	virtual void BreedingPairsOutput(int /*unused*/) {
	}
	virtual int TheBreedingFemalesProbe(int /*unused*/) { return 0; }
	virtual int TheFledgelingProbe() { return 0; }
	virtual void BreedingSuccessProbeOutput(double /*unused*/, int /*unused*/, int /*unused*/, int /*unused*/, int
	                                        /*unused*/, int /*unused*/, int /*unused*/, int /*unused*/) {
	}
	virtual int TheBreedingSuccessProbe(int& /*unused*/, int& /*unused*/, int& /*unused*/, int& /*unused*/, int&
	                                    /*unused*/, int& /*unused*/) { return 0; }
	virtual void FledgelingProbeOutput(int /*unused*/, int /*unused*/) {
	}
	virtual void TheGeneticProbe(unsigned /*unused*/, int /*unused*/, unsigned& /*unused*/) {
	}
	virtual void GeneticsResultsOutput(FILE* /*unused*/, unsigned /*unused*/) {
	}

	void Debug_Shuffle(unsigned int Type);

	virtual void RunStepMethods();
};

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

/**
\brief A class for running simulations without animals
*/
class None_Population_Manager : public Population_Manager {
public:
	None_Population_Manager(Landscape* L) : Population_Manager(L, 0) {
		m_ListNameLength = 0; // This is for life stages
		TheProbe[0] = nullptr;
	}
};

//------------------------------------------------------------------------------

/**
\brief
A small class to hold the pointers to active population managers
*/
class PopulationManagerList {
public:
	PopulationManagerList() {;}
	void SetPopulation(std::shared_ptr<Population_Manager_Base> p_pm, int a_pt) {
		m_populationlist[a_pt].push_back(p_pm);
		if (p_pm != nullptr) m_populationarray.at(a_pt) = true;
		else m_populationarray.at(a_pt) = false;
	}

	void RemovePopulation(std::shared_ptr<Population_Manager_Base> p_pm, int a_pt) {
		for (auto it = m_populationlist[a_pt].begin(); it != m_populationlist[a_pt].end(); ++it)
		{
			if (*it == p_pm)
			{
				m_populationlist[a_pt].erase(it);
				break;
			}
		}
		if (m_populationlist[a_pt].size() < 1) m_populationarray.at(a_pt) = false;
	}


	/**\brief Returns the number of population manager, */
	int GetPopulationManagerNum(int a_pt) { return static_cast<int>(m_populationlist[a_pt].size()); }

	/*
	 * The following function returns raw pointer-- it should never be used
	 * */
	Population_Manager_Base* GetPopulation(int a_pt, int a_index=0) { if( m_populationlist[a_pt].size()<1) return nullptr; return m_populationlist[a_pt][a_index].get(); }
	/*
	 * In the bright future only the next function will be used (or the prototype of the former one will be changed)
	 * */
	std::shared_ptr<Population_Manager_Base> GetPopulation_smart(int a_pt, int a_index=0) { if( m_populationlist[a_pt].size()<1) return nullptr; return m_populationlist[a_pt][a_index]; }
	/**\brief returns the number of active populations O(1) */
	int SupplyActivePopulationsCount();

protected:
	std::vector<std::shared_ptr<Population_Manager_Base>> m_populationlist[static_cast<int>(TOP_foobar)];
	std::array<bool, TOP_foobar> m_populationarray{false};
};

//------------------------------------------------------------------------------

#endif
