//
//  DataCrawler.hpp
//  DataExtractor
//
//  Created by Benjamin G Fields on 5/15/18.
//  Copyright © 2018 Benjamin G Fields. All rights reserved.
//

#ifndef DataCrawler_hpp
#define DataCrawler_hpp

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stdexcept>
#include <unordered_set>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <algorithm>

typedef struct{
  std::string jobID;
  float time;
  int jobsInSystem;
  std::vector<std::string> connections;
}Start;

typedef struct{
  std::string jobID;
  float time;
  int jobsInSystem;
}Finish;

typedef struct{
  float startTime;
  int Pid;
}startInfo;

typedef struct {
  int sum;
  float totaltime;
}processData;

class DataCrawler{
private:
  std::ifstream myStartFile;
  std::ifstream myFinishFile;
  std::ofstream resultsFile;
  std::vector<Start> myStarts;
  std::vector<Finish> myFinishs;
public:
  DataCrawler(std::string startFileName, std::string finishFileName);
  ~DataCrawler();
  std::vector<Start> readInStartsFile();
  std::vector<Finish> readInFinishFile();
  int getClosingBracket(int pos, std::string ID);
  int posOfNextBracket(std::string line, int start);
  int getPosOfColon(std::string line);
  int getPosOfDash(std::string line, int start);
  std::string getDependancy(std::string line);
  std::vector<std::string> getUpConnections(std::string line);
  int numJobsComplete(std::vector<Finish> &myFin,int pid);
  int getNumProcesses(std::vector<Finish> &myFin);
  void printTransitionStateMatrix(int** mat, int size);
  void printTransitionTimeMatrix(float** mat, int size);
  int getProcessID(std::string line);
  std::vector<int> getPIDS(std::string line);
  void countTransitions(int **mat, int size, std::vector<Finish> &myfin);
  startInfo getStart(std::string id,std::vector<Start> &myStart);
  void getTransitionTimes(float** mat, int size, std::vector<Finish> &myFin, std::vector<Start> &myStart);
  void averageTransitionTimes(float** mat, int size,int** matFreq);
  int getMaxJobsInSystem(std::vector<Finish> &myFin);
  void getUtilizedBufferCapacity(int** bufMat,int** freqMat,int size);
  void getCapacityForPos(int** buffMat,int source, int destination);
  int getJobNum(std::string line);
  std::unordered_set<int> getNumberOfTerminalStates(int**freqMat,int size);
  std::vector<int> getProperStartConnections(int downstream,int upstream);
  void getAverageProcessTimes(float*mytimes, int size);
  float findStartTime(std::string id);
  processData countNumberOfTimesPIDFinishes(int id);
  void run();
};

#endif /* DataCrawler_hpp */
