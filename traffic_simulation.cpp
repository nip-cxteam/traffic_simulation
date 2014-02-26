#include <iostream>
#include <stdio.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include <vector>
#include <algorithm> //random_shuffle, distance, max_element
#include <omp.h>

#include <fstream>
#include <bzlib.h>

#include "hdf_save.h"
#include "simulation.h"

using namespace std;



void BZIP(char* _filename){
	char message[100];
	sprintf(message, "bzip2 -6 %s", _filename);
	system(message);
}

string start(float density, float car_ratio, time_t seed){
	gsl_rng_set(generator, seed);
	time_t start = clock();
	vector<vector<vector<int> > > DATA;
	char _filename[30];
	sprintf(_filename, "CarRatio.%.2f_Density.%.2f.h5", car_ratio, density);
	for (int trial = 1; trial < TRIALS + 1; trial++){
		Simulation *traffic = new Simulation;
		traffic->evolve(density, car_ratio);
		DATA = traffic->vehicle_data;
		delete traffic;
		hd5data(DATA, density, car_ratio, trial, _filename, seed);
	}
	BZIP(_filename);
	char message[30];
	sprintf(message, "done in %.3f seconds.", (clock() - start)*1.0 / (CLOCKS_PER_SEC));
	return message;
}

void printstat(vector<string> status, vector<float> densities, float car_ratio){
#if (_WIN32)
	char clear[6] = "CLS";
#else
	char clear[8] = "clear";
#endif
	system(clear);
	for (int i = 0; i < 19; i++){
		cout << "Density::" << densities[i] << "\t\tCarRatio::" << car_ratio << "\t\tStatus:"<< status[i] << endl;
	}
}

static void show_usage(string name)
{
	std::cerr << "Usage: " << name << " -c CAR_RATIO [<option(s)> VALUES]\n"
		<< "Options:\n"
		<< "\t-h,--help\t\tShow this help message\n"
		<< "\t-c,--carratio\t\tSpecify the car ratio\n"
		<< "\t-T,--trials \t\tSpecify the number of trials (Default: " << TRIALS << ")\n"
		<< "\t-R,--roadlength \tSpecify the length of the road (Default: " << ROADLENGTH << ")\n"
		<< "\t-t,--timesteps \t\tSpecify the number of timesteps (Default: " << TIMESTEPS << ")\n"
		<< "\t-r,--reallanes \t\tSpecify the number of real lanes (Default: " << REAL_LANES << ")\n"
		<< "\t-v,--virtuallanes \tSpecify the number of virtual lanes (Default: " << VIRTUAL_LANES << ")\n"
		<< "\t-L,--lanechange \tToggle lane changing (Default: " << LANE_CHANGE << ")\n"
		<< "\t,--loadseed \tSet seed state (Default: " << LOAD_SEED << ")\n"
		<< endl;
}

int parser(int argc, char* argv[]){
	if (argc == 1) {
		cout << "Using default values." << endl;
		return 0;
	}
	for (int i = 1; i < argc; ++i) {
		std::string arg = argv[i];
		if ((arg == "-h") || (arg == "--help")) {
			show_usage(argv[0]);
			return 1;
		}
		else if ((arg == "-c") || (arg == "--carratio")) {
			if (i + 1 < argc) { // Make sure we aren't at the end of argv!
				i++;
				car_ratio = atof(argv[i]); // Increment 'i' so we don't get the argument as the next argv[i].
			}
			else {
				std::cerr << "--carratio option requires one argument." << std::endl;
				return 1;
			}
		}
		else if ((arg == "-T") || (arg == "--trials")) {
			if (i + 1 < argc) { // Make sure we aren't at the end of argv!
				i++;
				TRIALS = atoi(argv[i]); // Increment 'i' so we don't get the argument as the next argv[i].
			}
			else {
				std::cerr << "--trials option requires one argument." << std::endl;
				return 1;
			}
		}
		else if ((arg == "-t") || (arg == "--timesteps")) {
			if (i + 1 < argc) { // Make sure we aren't at the end of argv!
				i++;
				TIMESTEPS = atoi(argv[i]); // Increment 'i' so we don't get the argument as the next argv[i].
			}
			else {
				std::cerr << "--timesteps option requires one argument." << std::endl;
				return 1;
			}
		}
		else if ((arg == "-r") || (arg == "--reallanes")) {
			if (i + 1 < argc) { // Make sure we aren't at the end of argv!
				i++;
				REAL_LANES = atoi(argv[i]); // Increment 'i' so we don't get the argument as the next argv[i].
				LANES = REAL_LANES + VIRTUAL_LANES;
			}
			else {
				std::cerr << "--reallanes option requires one argument." << std::endl;
				return 1;
			}
		}
		else if ((arg == "-v") || (arg == "--virtuallanes")) {
			if (i + 1 < argc) { // Make sure we aren't at the end of argv!
				i++;
				VIRTUAL_LANES = atoi(argv[i]); // Increment 'i' so we don't get the argument as the next argv[i].
				LANES = REAL_LANES + VIRTUAL_LANES;
			}
			else {
				std::cerr << "--virtuallanes option requires one argument." << std::endl;
				return 1;
			}
		}
		else if ((arg == "-R") || (arg == "--roadlength")) {
			if (i + 1 < argc) { // Make sure we aren't at the end of argv!
				i++;
				ROADLENGTH = atoi(argv[i]); // Increment 'i' so we don't get the argument as the next argv[i].
			}
			else {
				std::cerr << "--roadlength option requires one argument." << std::endl;
				return 1;
			}
		}
		else if ((arg == "-L") || (arg == "--lanechange")) {
			if (i + 1 < argc) { // Make sure we aren't at the end of argv!
				i++;
				LANE_CHANGE = atoi(argv[i]); // Increment 'i' so we don't get the argument as the next argv[i].
			}
			else {
				std::cerr << "--lanechange option requires one argument (0 or 1)." << std::endl;
				return 1;
			}
		}
		else if (arg == "--loadseed") {
			if (i + 1 < argc) { // Make sure we aren't at the end of argv!
				i++;
				LOAD_SEED = atoi(argv[i]); // Increment 'i' so we don't get the argument as the next argv[i].
			}
			else {
				std::cerr << "--loadseed option requires one argument (0 or 1)." << std::endl;
				return 1;
			}
		}
	}
	return 0;
}

int main(int argc, char* argv[]){
	int status = parser(argc, argv);
	if (status == 1) return 1;

	vector<float> densities(19);
	vector<string> runmsg(19, "Not Done");
	double first = 0.05;
	for (int i = 0; i < 19; i++){
		densities[i] = first;
		first += 0.05;
	}
		omp_set_num_threads(2);
		#pragma omp parallel for
		for (unsigned i = 0; i < densities.size(); i++){
			if (LOAD_SEED == false) seed = time(NULL) * 123456789;
			runmsg[i] = start(densities[i], car_ratio, seed);
			printstat(runmsg, densities, car_ratio);
		}
	return 0;
}
