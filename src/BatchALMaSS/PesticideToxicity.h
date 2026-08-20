//
// PesticideToxicity.h
//
/*
*******************************************************************************************************
Copyright (c) 2023, Xiaodong Duan
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

#ifndef PesticideToxicityH
#define PesticideToxicityH
#include <numeric>
#include <array>
#include <vector>
#include "PopulationManager.h" // for TAnimal (base class)

#ifdef __POLLINERA_TKTD
/** \brief Index of each per-CAG TKTD parameter within the shared s_tktd table. */
enum TktdParam { TKTD_k_d = 0, TKTD_m, TKTD_b, TKTD_w_o, TKTD_w_t, TKTD_COUNT };
#endif

/**
 * \brief A TAnimal that carries pesticide/toxicity state and behaviour.
 *
 * This class merges the former PesticideStore (pesticide storage + accumulation) and PesticideToxicity
 * (TKTD dynamics + host-animal mortality) directly into a TAnimal subclass. Only species that are exposed
 * to pesticide (currently Osmia, HoneyBee and Newt) derive from TAnimalToxicity; every other species
 * derives from TAnimal directly and pays no memory/ctor cost for the ~19 pesticide vectors.
 *
 * Because the toxicity object IS the animal now, the former `m_my_animal_host` pointer is gone -- host
 * queries (location, vitality) are direct calls on `this`.
 *
 * All storage/allocation is gated on the pesticide engine flag (l_pest_enable_pesticide_engine): with the
 * engine off, the ctor allocates nothing and the functioning methods are simply never called by the bees.
 */
class TAnimalToxicity : public TAnimal
{
protected:
	/** \brief The vector to store the pesticide amount for each pesticide. */
	std::vector<double> m_pest_amount_vec;
	#ifdef __POLLINERA_TKTD
	/** \brief Shared, immutable per-CAG TKTD parameters: rows = pesticide slot / eCAG (g_pest_NoPPPs),
	 *  cols = {k_d, m, b, w_o, w_t} (see TktdParam). Loaded ONCE at model setup via InitTKTDParams(). */
	static std::vector<std::array<double, TKTD_COUNT>> s_tktd;
	static bool s_tktd_loaded;
	/** \brief Pesticide concentration for intake from the environment (reset each time step). */
	std::vector<double> m_pest_intake_concentration_vec;
	std::vector<double> m_pest_intake_concentration_vec_previous;
	std::vector<double> m_pest_contact_concentration_vec;
	std::vector<double> m_pest_contact_concentration_vec_previous;
	std::vector<double> m_pest_topical_concentration_vec;
	std::vector<double> m_pest_topical_concentration_vec_previous;
	std::vector<double> m_pest_intake_vec_buffer;
	std::vector<double> m_pest_intake_vec_buffer_previous;
	std::vector<double> m_pest_contact_vec_buffer;
	std::vector<double> m_pest_contact_vec_buffer_previous;
	std::vector<double> m_pest_topical_vec_buffer;
	std::vector<double> m_pest_topical_vec_buffer_previous;
	/** \brief The exponential values for the elimination rates of the buffer. */
	std::vector<double> m_pest_elimination_rate_exp_vec;
	/** \brief The damage caused by each pesticide (and previous time step). */
	std::vector<double> m_pest_damage_vec;
	std::vector<double> m_pest_damage_vec_previous;
	/** \brief The hazard caused by each pesticide (and previous time step). */
	std::vector<double> m_pest_hazard_vec;
	std::vector<double> m_pest_hazard_vec_previous;
	/** \brief The accumulated hazard of the pesticide. */
	double m_pest_acc_hazard = 0.0;
	/** \brief The hazard change from yesterday / for today. */
	double m_pest_hazard_change_yesterday = 0.0;
	double m_pest_hazard_change_today = 0.0;
	/** \brief The mortality rate of this animal caused by pesticide body burden. */
	double m_my_animal_mortality = 0.0;
	#endif
	/** \brief Allocate + zero the pesticide storage vectors. No-op unless the engine is on. Called by ctors. */
	void initPesticideStorage();

public:
	/** \brief Constructors mirror TAnimal's. Allocate the pesticide vectors only when the engine is on. */
	TAnimalToxicity(int x, int y, Landscape* L);
	TAnimalToxicity(int x, int y);

	#ifdef __POLLINERA_TKTD
	/** \brief Fill the shared per-CAG TKTD table (s_tktd) once from the CAG JSON. Call this ONCE at model
	 * setup (single-threaded, right after the CAGs JSON is loaded and g_pest_NoPPPs is known), NOT lazily
	 * from a ctor -- ctors run inside the parallel animal/nest-creation region, where a lazy static fill
	 * races and segfaults. No-op if already loaded or the running species has no TKTD. */
	static void InitTKTDParams();
	#endif

	/** \brief Add pesticide based on the eating amount and the amount of food per square meter. */
	void addPesticide(double eating_amount, double amount_per_square, double (Landscape::*a_supply_pest_func_pointer)(int, int, int), int loc_x, int loc_y);
	/** \brief Add pesticide based on the given amount and the given type. */
	void addPesticide(double a_mount, int a_type) {m_pest_amount_vec[a_type] += a_mount;}
	/** \brief Set the pesticide vector based on the given vector. */
	void SetPesticideVector(std::vector<double>& a_pest_amount_vec) { m_pest_amount_vec = a_pest_amount_vec;}
	/** \brief Return the pesticide amount array. */
	std::vector<double>* supplyPesticide(void) {return &m_pest_amount_vec;}
	/** \brief Return the pesticide amount. */
	double supplyPesticide(int a_type = 0) {return(m_pest_amount_vec[a_type]);}
	/** \brief Return the total pesticide amount. */
	double supplyTotalPesticide(void) {return std::accumulate(m_pest_amount_vec.begin(), m_pest_amount_vec.end(), 0.0);}
	/** \brief Default: decay the pesticide body burden. */
	void doDecay(void);
	/** \brief Decay the pesticide body burden using the given decay rate. */
	void doDecay(double a_decay_rate);

	/** \brief Copy pesticide from another toxicity object. */
	void copyPesticide(TAnimalToxicity* a_pest_store);
	/** \brief Overspray this animal when a pesticide application happens where it is. */
	void doOverspray(double a_surface_area, double a_over_spray_chance, double a_absorption_rate, int a_x = -1, int a_y = -1);
	/** \brief Contact for the animal foraging in a contaminated area. */
	void doContact(double a_surface_area, double a_absorption_rate, int a_x = -1, int a_y = -1);
	/** \brief Supply this animal's mortality rate caused by pesticide. */
	double supplyPestMortality(void) {return m_my_animal_mortality;}
	/** \brief Reset the toxicity state; call when the host animal dies. */
	void reset(void);
	/** \brief Default calculation of the toxicity effect. */
	double doToxicity(double a_decay_rate, double a_mass=-1);
	/** \brief Decay the pesticide body burden based on the animal's vitality. */
	void doDecayFromVitality(void);
	/** \brief Decay based on the given vitality value and the animal's age. */
	void doDecayFromVitalityAndAge(double a_vitality, double a_age);
	/** \brief Decay based on the given vitality value, age and parameters. */
	void doDecayParaFromVitalityAndAge(double a_vitality, double a_age, std::vector<double> a_pest_decay_rate, std::vector<double> a_pest_vitality_threshold, std::vector<double> a_pest_vitality_constant);
	/** \brief Calculate the pesticide stress using the default parameters. */
	double doPestStress(void);
	/** \brief Calculate the pesticide stress using the passed parameters. */
	double doPestStressPara(std::vector<double> a_pest_threshold_stress, std::vector<double> a_pest_killing_rate);
	#ifdef __POLLINERA_TKTD
	/** \brief Reset the exposure from the environment. */
	void resetExposure(void){
		m_pest_intake_concentration_vec_previous.assign(m_pest_intake_concentration_vec.begin(), m_pest_intake_concentration_vec.end());
		std::fill(m_pest_intake_concentration_vec.begin(), m_pest_intake_concentration_vec.end(), 0.0);
		m_pest_contact_concentration_vec_previous.assign(m_pest_contact_concentration_vec.begin(), m_pest_contact_concentration_vec.end());
		std::fill(m_pest_contact_concentration_vec.begin(), m_pest_contact_concentration_vec.end(), 0.0);
		m_pest_topical_concentration_vec_previous.assign(m_pest_topical_concentration_vec.begin(), m_pest_topical_concentration_vec.end());
		std::fill(m_pest_topical_concentration_vec.begin(), m_pest_topical_concentration_vec.end(), 0.0);
	}
	/** \brief Update the exposure from intake. */
	void updateIntakeExposure(double a_intake_concentration, int a_intake_type){
		m_pest_intake_concentration_vec[a_intake_type] = a_intake_concentration;
		m_pest_intake_concentration_vec_previous[a_intake_type] = m_pest_intake_concentration_vec[a_intake_type];
	}
	/** \brief Update the exposure from topical. */
	void updateTopicalExposure(double a_topical_concentration, int a_topical_type){
		m_pest_topical_concentration_vec[a_topical_type] = a_topical_concentration;
		m_pest_topical_concentration_vec_previous[a_topical_type] = m_pest_topical_concentration_vec[a_topical_type];
	}
	/** \brief Update the exposure from contact. */
	void updateContactExposure(double a_contact_concentration, int a_contact_type){
		m_pest_contact_concentration_vec[a_contact_type] = a_contact_concentration;
		m_pest_contact_concentration_vec_previous[a_contact_type] = m_pest_contact_concentration_vec[a_contact_type];
	}
	/** \brief Update the buffer. */
	void updateBuffer(void);
	/** \brief Calculate the buffer in the given time. */
	void calculateBuffer(double a_time, int pest_type, double &a_oral, double &a_topical, double &a_contact);
	/** \brief Update the damage. */
	void updateDamage(void);
	/** \brief Calculate the damage in the given time. */
	double calculateDamage(double a_time, int pest_type);
	/** \brief Update the hazard. */
	void updateHazard(void);
	/** \brief Calculate the hazard in the given time. */
	double calculateHazard(double a_time, int pest_type, double a_oral_buffer, double a_topical_buffer, double a_contact_buffer, double a_damage);
	/** \brief Update the mortality rate for the buffer-guts based method. */
	double calMortalityBufferBased(void);
	#endif
};

#endif //end PesticideToxicityH
