//
//  Process.cpp
//  ASAE
//
//  Created by Benjamin G Fields on 4/2/18.
//  Copyright © 2018 Benjamin G Fields. All rights reserved.
//
//  Description: Implementation of the process object

#include "Process.hpp"

typedef struct{
  float percent;
  int index;
}selectionChance;

//Description:defines what type of distribution a process adheres to (triangular,normal, uniform)
void Process::setDistType(int type){
  this->distType = type;
}

//Description: standard function to set the process ID
void Process::setProcessID(int id){
  this->processID = id;
}

//Description: standard function to return the type of process indicating where in the line it is
int Process::getProcessType(){
  return this->processType;
}

//Description:prints the parameters of the current process
void Process::printProcessInfo(){
  std::cout<<"\tProcess ID: "<<this->processID<<"\n";
  std::cout<<"\tDist Type: "<<this->distType<<"\n";
  std::cout<<"\tPos Type: "<<this->processType<<"\n";
  std::cout<<"\tUpstream Dependencies: "<<this->upStreamDependencies.size()<<"\n";
  for(int i = 0;i<this->upStreamDependencies.size();++i){
    std::cout<<"\t\tProcessID: "<<upStreamDependencies[i].processID<<"\n";
    std::cout<<"\t\tBufferIndex: "<<upStreamDependencies[i].bufferIndex<<"\n";
  }
  std::cout<<"\tDownStream Dependencies: "<<this->downStreamDependencies.size()<<"\n";
  for(int i = 0;i<this->downStreamDependencies.size();++i){
    std::cout<<"\t\tProcessID: "<<downStreamDependencies[i].processID<<"\n";
    std::cout<<"\t\tPercentage: "<<downStreamDependencies[i].percentage<<"\n";
    std::cout<<"\t\tBuffer Capacity: "<<downStreamDependencies[i].capacity<<"\n";
  }
}

//Description:create the dependencies that will be upstream from a process. limited to 0-9
void Process::setUpstreamDependencies(std::string line){
  std::cout<<"\tSetting up upstream dependencies with string "<<line<<"\n";
  int num = std::atoi(line.substr(0,1).c_str());
  std::cout<<"There are "<<num<<" upstream dependencies\n";
  int start = 3;
  for(int i = 0;i<num;i++){
    //create each dependency
    upStreamConnection conn;
    conn.processID = std::atoi(line.substr(start,1).c_str());
    conn.bufferIndex = std::atoi(line.substr(start+2,1).c_str());
    upStreamDependencies.push_back(conn);
    start = start + 6;
  }
}

//Description:create the downstream dependencies to control flow. limited to 0-9
void Process::setDownstreamDependencies(std::string line){
  std::cout<<"\tSetting up downstream dependencies with string "<<line<<"\n";
  int num = std::atoi(line.substr(0,1).c_str());
  std::cout<<"There are "<<num<<" downstream dependencies\n";
  int start = 2;
  float total = 0.0;
  for(int i = 0;i<num;i++){
    //create each dependency
    downStreamConnection conn;
    conn.processID = std::atoi(line.substr(start,1).c_str());
    conn.percentage = std::atof(line.substr(start+2,4).c_str());
    total = total + conn.percentage;
    conn.capacity = std::atoi(line.substr(start+7,1).c_str());
    downStreamDependencies.push_back(conn);
    Buffer buff;
    buff.capacity = conn.capacity;
    process_Buffers.push_back(buff);
    start = start +9;
  }
  
  if(abs(total-1.00) > 0.0001 && num != 0){
    throw std::runtime_error("\nDOWNSTREAM CONNECTION ERROR: Downstream branching percentages must equal 1.00\n");
  }
}

//Description:return a random time for a triangular dist
float getTrianglarDistribution(float a, float b, float c)
{
  //std::cout << "Generating a random number in the triangular distribution with low " << a << " and high " << b << " and mean of " << c << std::endl;
  float randnum = (float)rand() / (float)RAND_MAX;
  //std::cout << "Rand num: " << randnum << std::endl;
  float fc = (c - a) / (b - a);
  float val;
  if (randnum < fc && fc > 0.0)
  {
    val = a + sqrt(randnum*(b - a) / (c - a));
  }
  else
  {
    val = b - sqrt((1.0 - randnum)*(b - a) / (c - a));
  }
  //std::cout << "Value is " << val << std::endl;
  return val;
  
}

//Description:bassed on the type of distribution get a random time
float Process::getProcessingTimeFromDist(){
  if (distType == TRIANGULAR) {
    return getTrianglarDistribution(minimum, upper, average);
  }
  else if(distType == NORMAL){
    //TODO
  }
  else if(distType == UNIFORM){
    //TODO
  }
  return 0.0;
}

//Description:set the type of process to indicate position in line
void Process::setProcessType(int type){
  this->processType = type;
  std::cout<<"\tSetting process type to "<<type<<"\n";
}

//Description:standard setter for buffer capacity
void Process::setBufferCapacity(int val,int ind){
  std::cout<<"\tSetting buffer capacity of "<<val<<"\n";
  this->process_Buffers[ind].capacity = val;
}


struct greater_than_key
{
  inline bool operator() (const selectionChance& struct1, const selectionChance& struct2)
  {
    return (struct1.percent > struct2.percent);
  }
};

int Process::getBufferIndexToPush(){
  std::cout<<"Choosing buffer to push\n";
  int numdep = (int)downStreamDependencies.size();
  std::vector<selectionChance> options;
  for(int i = 0;i<numdep;i++){
    selectionChance option;
    option.percent = downStreamDependencies[i].percentage;
    option.index = i;
    options.push_back(option);
  }
  std::sort(options.begin(), options.end(),greater_than_key());
  std::cout<<"Sorted percentages: ";
  for(int i = 0; i<numdep;i++){
    std::cout<<"("<<options[i].percent<<","<<options[i].index<<")|";
  }
  std::cout<<"\n";
  
  int ans = 0;
  int v2 = rand() % 100 + 1;
  float percentR = (float)v2/100.0;
  std::cout<<"Rand percentage: "<<percentR<<"\n";
  float totalPercentage = 0.0;
  for(int i = 0;i<numdep;i++){
    totalPercentage = totalPercentage+options[i].percent;
    if(i == 0 && percentR <= totalPercentage){
      //select first
      ans = i;
      break;
    }
    else if(i == numdep-1){
      ans = i;
      break;
    }
    else{
      if(percentR<=totalPercentage){
        ans = i;
        break;
      }
    }
  }
  std::cout<<"Selected index "<<ans<<" with probability of "<<options[ans].percent<<"\n";
  return ans;
}

//Description: returns index of the process buffer downstream to place into
int Process::getNumDownStreamDependencies(){
  return (int)downStreamDependencies.size();
}

//Description: returns the amount of buffers feeding a process
int Process::getNumUpStreamDependencies(){
  return (int)upStreamDependencies.size();
}

//Description: takes an event and places in the process buffer
void Process::placeEventInBuffer(Event E,int ind){
  process_Buffers[ind].placeInBuffer(E);
}

//Description: takes an event fromt he process buffer
Event Process::getEventFromBuffer(int ind){
  return process_Buffers[ind].GetNext();
}

//Description: add one to the jobs complete parameter
void Process::AddOneJob(){
  jobNum++;
}

//Description: get the job number for the process
std::string Process::getJobNum(){
  return std::to_string(jobNum);
}

//Description: return the state of the process buffer (full,empty, space left)
int Process::BufferState(int i){
  return process_Buffers[i].getState();
}

//Description: set the timing parameters for distribution
void Process::setProcessParameters(std::string info){
  if (this->distType == TRIANGULAR) {
    //setParameters in process for triangular
    info.append(">");
    std::cout<<"Setting up triangular with string "<<info<< "\n";
    int done = 0;
    int index = 3;
    int front = 2;
    int num = 1;
    int length = 1;
    while(!done){
      if(info[index]=='>'){
        done =1;
        std::string max = info.substr(front,length);
        std::cout<<"\tMax:"<<max<<"\n";
        upper = atof(max.c_str());
        continue;
      }
      if(info[index]==':'){
        if(num ==1){
          std::string min = info.substr(front,length);
          std::cout<<"\tMinimum:"<<min<<"\n";
          minimum = atof(min.c_str());
          num++;
          length = 0;
          front = index+1;
          index++;
        }
        else if(num ==2){
          std::string avg = info.substr(front,length);
          std::cout<<"\tAverage:"<<avg<<"\n";
          average = atof(avg.c_str());
          num++;
          length = 0;
          front = index+1;
          index++;
        }
      }
      else{
        length++;
        index++;
      }
    }
  }
  else if(this->distType==NORMAL){
    //set for normal
    //TODO
  }
  else if(this->distType==UNIFORM){
    //set for uniform
    //TODO
  }
}
