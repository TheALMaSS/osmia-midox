
/**
 * \file ALMaSS_Random.h
 * \author Luminita C. Totu
 * \version August 2023
 *
 * \brief Implementing almass specific random number generation and probability distribution classes, based on/wrapping c ++ std library functionality. Boost library is not used anymore.
 * 
 * \section LICENSE
 *
 * \section DESCRIPTION
 * This files make it possible to use a simple syntax for generating random numbers from a specified distribution
 *
 * \section USAGE-A
 * Steps needed to use an almass probability distribution object  
 *
 * (1) Include the module header file
 *
 *            #include "ALMaSS_Random.h"
 *
 * (2) Define a "probability_distribution" object. Here are some examples:
 *
 *			  probability_distribution p1("DISCRETE", "1.0 2.5 4 5.1 7"); // weights for the variable number of outcomes {0,1,2,...}
 *			  probability_distribution p1("NORMAL", "1.0 3.0"); // mean and standard deviation 
 *			  probability_distribution p3("BETABINOMIAL", "14 2 1 0"); // Outcome max value, alpha, beta from binomial distribution, and the no_zero flag
 *			  probability_distribution p4("UNIREAL", "0 1");  // // [min,max)
 *			  probability_distribution p5("UNIREAL", "0 1");  // // [min.max]
 *
 *  (3) Use the probability distribution to make an random extraction. Here are some examples: 
 *
 *			  int x = p1.geti();   // or  int x = p1.get(); the casting from double to int is automatic
 *			  double x = p2.get();
 *	
 * \section USAGE-B
 * Steps needed to one of the generic random functions g_rand_uni(), g_rand_uni2() or g_rand_uni3() ( backward compatibility for some of the code, also common used distributions )
 *
 * (1) Include the module header file
 *
 *            #include "ALMaSS_Random.h"
 *
 * (2) Use the g_rand_uni()
 *
 *           double x = g_rand_uni();   // random real number between [0,1)
 */

#ifndef ALMASS_RANDOM_H
#define ALMASS_RANDOM_H

#include <cstdint>

/**
 * Frequently used function to return a double between [0,1) with uniform probability
 */
double g_rand_uni_fnc();
/**
 *  The following three functions are for code backward compatibility. Preferably not use for new code.
 */
int g_rand_uni2_fnc();
int g_rand_uni3_fnc();
int g_random_fnc(int a_range);


/**
 *  Function to reset the seed of the generator
 */
void g_generator_fixed_fnc(unsigned int s = 42);

/**
 *  Current global seed, for building deterministic index-seeded sub-streams that
 *  must not depend on the OpenMP thread count. See g_get_global_seed_fnc() in the .cpp.
 */
uint32_t g_get_global_seed_fnc();

/**
	ALMaSS Probability Distribution class, wrapping around the Boost-library random distributions classes
*/
class probability_distribution {

private:

	/**
	 *  DISCRETE, custom build distribution with outcomes {0,1,2,..}
	 *
	 *  Needs as parameters a (not necesarilly normalized) set of weights (real numbers).
	 *  The weights correspond to the outcomes {0,1,2,...} 
	 *  Therefore the number of weights give the number of outcomes, where zero is included 
	 */
	const std::string DISCRETE_DIST_S{ "DISCRETE" };  

	/**
	 *  BETABINOMIAL, a beta-binomial discrete distribution with outcomes {0,1,2,..,N}
	 *
	 *  It needs 4 parameters:
	 *  N (int) - max value of the outcome
	 *  alpha, beta (real) - the beta binomial parameters, determine the shape of probabilities over the outcome space
	 *  no_zero (numerical boolean) - extra parameter to cancel out the probability for the zero value 
	 */
	const std::string BETABINOMIAL_DIST_S{ "BETABINOMIAL" };

    /**
	 *  NORMAL, a continuous normal distribution 
	 *
	 *  It needs two parameters: the mean (real number)  and the standard deviation (real number, >=0)
	 */
	const std::string NORMAL_DIST_S{ "NORMAL" };

    /**
     *  UNIREAL, a continuous uniform distribution
     *
     *  Parametres are min and max, both real values. The outcomes belong to [min,max)
     */
	const std::string UNIREAL_DIST_S{ "UNIREAL" };

	/**
     *  UNIINT, a discrete uniform distribution
     *
     *  Parametres are min and max, both integer values. The outcomes belong to {min, min+1, ... , max}
     */
	const std::string UNIINT_DIST_S{ "UNIINT" };


    /**
     *  BETA, the continuous/real-valued beta distribution. BETA distribution is implemented using two gamma distributions since it is not included in the standard library.
     *
     *  Parametres are alpha and beta. The outcomes belong to [0,1]
     */
    const std::string BETA_DIST_S{ "BETA" };


	/**
	*  GAMMA, the continuous/real-valued cauchy distribution
	*
	*  Parametres are location and scale. The outcomes belong to [0,max]
	*/
	const std::string GAMMA_DIST_S{ "GAMMA" };


	/**
	*  CAUCHY, the continuous/real-valued gamma distribution
	*
	*  Parametres are alpha and beta. The outcomes is any real number.
	*/
	const std::string CAUCHY_DIST_S{ "CAUCHY" };


	/**
	*  EXPONENTIAL, the continuous/real-valued exponential distribution
	*
	*  Parametres are lamda. The outcomes is any real number.
	*/
	const std::string EXPONENTIAL_DIST_S{ "EXPONENTIAL" };

	static const int DISCRETE_DIST_T = 1;
	static const int NORMAL_DIST_T = 2;
	static const int UNIREAL_DIST_T = 3;
	static const int UNIINT_DIST_T = 4;
    static const int BETA_DIST_T = 5;
	static const int GAMMA_DIST_T = 6;
	static const int CAUCHY_DIST_T = 7;
	static const int EXPONENTIAL_DIST_T = 8;

	void* m_probDistrib; /* Alternatively could use boost:variant as a union of different distribution types, and a visitor syntax; To test which is faster, classic or variant */
	void* m_probDistrib1;
	void* m_probDistrib2;
	int m_type; 

public:

	/** 
	 *  Constructor of the class, version 1
	 *
	 * \param[in] prob_type a string containing the label of the distribution, must match the supported/predefined values
	 * \param[in] prob_params a string containing space,comma, or tab separated numerical values of the required parameters associated with the distribution type
	 */
	probability_distribution(const std::string& prob_type, std::string prob_params);

	//  
	//   Constructor of the class, version 2
	// 
    //  \param[in] prob_type a string containing the label of the distribution, must match the supported/predefined values
	//  \param[in] prob_params a vector of numerical values associated with the distribution type
	// 
	// probability_distribution(std::string prob_type, std::vector<double> prob_params);

	/** 
	 * Returns a numerical random variate extracted from the defined probability distribution
	 *
	 * \return the random number, returned as a double type 
	 */
	double Get() const;
	
	/**
	 * Returns an integer random variate extracted from the defined probability distribution
	 * Maybe marginally faster than get(), as it has less types to check
	 *
	 * \return the random number, returned as an integer; returns zero if the underlying prob is a continuous and not a discrete one
	 */
	int Geti() const;

	~probability_distribution();

};

void g_init_random_seed_fnc();


#endif // ALMASS_RANDOM_H
