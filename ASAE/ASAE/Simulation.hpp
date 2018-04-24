//
//  Simulation.hpp
//  ASAE
//
//  Created by Benjamin G Fields on 4/2/18.
//  Copyright © 2018 Benjamin G Fields. All rights reserved.
//
//  Description: defines the use and structure of the simulation class

#ifndef Simulation_hpp
#define Simulation_hpp

#include <stdio.h>
#include <queue>
#include <vector>
#include "Event.hpp"
#include "Process.hpp"
#include "xlsxwriter.h"
#include <iostream>

//Description: main class that runs the simulation
class Simulation{
private:
  int debug;
  int finished;
  int jobsComplete;
  int jobsInSystem;
  int createJobID;
  float simTime;
  float timeStep;
  lxw_workbook* workbook;
  lxw_worksheet* worksheet;
  std::priority_queue <Event, std::vector<Event>, Compare> eventQueue;
  Process* simProcesses;
  int numProcesses;
public:
  int startRecordRow;
  int FinishRecordRow;
  Simulation();
  ~Simulation();
  int constructModel(std::vector<processInfo> &processes);
  int getFeedBufferState(Process P);
  void run(int numJobs, int verbose);
  void init();
  void printModel();
  void processNextEvent();
  nextEventInfo processCurrentEvent(Event current, int Process);
  void checkIfFinished(int);
};

#endif /* Simulation_hpp */
