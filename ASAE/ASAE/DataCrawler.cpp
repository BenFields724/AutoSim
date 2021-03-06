//
//  DataCrawler.cpp
//  DataExtractor
//
//  Created by Benjamin G Fields on 5/15/18.
//  Copyright © 2018 Benjamin G Fields. All rights reserved.
//

#include "DataCrawler.hpp"

//Description: Constructor to open and read files
DataCrawler::DataCrawler(std::string startFileName, std::string finishFileName){
  myStartFile.open(startFileName.c_str());
  if(!myStartFile.is_open()){
    std::cout<<"ERROR: Unable to open start event log\n";
    throw std::runtime_error("ERROR: failed to open starts.txt file!");
  }
  std::cout<<"Successfully opened start file\n";
  myFinishFile.open(finishFileName.c_str());
  if(!myFinishFile.is_open()){
    std::cout<<"ERROR: Unable to open finish event log\n";
    throw std::runtime_error("ERROR: failed to open finish.txt file!");
  }
  std::cout<<"Successfully opened finish file\n";
  resultsFile.open("Results.txt");
  if(!resultsFile.is_open()){
    std::cout<<"ERROR: Unable to open Results.txt\n";
    throw std::runtime_error("ERROR: failed to open Results.txt file!");
  }
  std::cout<<"Reading in Files\n";
  myStarts = readInStartsFile();
  myFinishs = readInFinishFile();
  if(myStarts.data() == NULL || myFinishs.data() == NULL){
    throw std::runtime_error("ERROR: Failed to create stored starts and finishes!");
  }
  std::cout<<"Successfully read in start and finish files\n";
}

//Description:Destructor to close and present files
DataCrawler::~DataCrawler(){
  std::cout<<"Shutting down crawler\n";
  //clean up and present results
  myStartFile.close();
  myFinishFile.close();
  resultsFile.close();
  system("open -a TextEdit Results.txt");
}

//Description: Main runner to extract all process information from event data
void DataCrawler::run(){
  std::cout<<"\n\n*********Starting DataCrawler**********\n";
  std::cout<<"Number of Starts: "<<myStarts.size()-1<<"\n";
  std::cout<<"Number of Finishes: "<<myFinishs.size()-1<<"\n";
  
  auto now = std::chrono::system_clock::now();
  std::time_t time = std::chrono::system_clock::to_time_t(now);
  resultsFile<<"Results for "<<std::ctime(&time)<<"\n";
  std::cout<<"Results for "<<std::ctime(&time)<<"\n";
  
  resultsFile<<"Number of Start Records: "<<myStarts.size()-1<<"\n";
  resultsFile<<"Number of Finish Records: "<<myFinishs.size()-1<<"\n\n";
  
  float endTime = myFinishs.back().time;
  std::cout<<"Simulation runtime: "<<endTime<<"\n";
  resultsFile<<"Simulation runtime: "<<endTime<<"\n";
  
  int numProcesses = getNumProcesses(myFinishs);
  //create the transition state matrix
  int** transitionStateMatrix = new int*[numProcesses];
  for(int i =0;i<numProcesses;++i){
    transitionStateMatrix[i] = new int[numProcesses];
    for(int j = 0; j<numProcesses;++j){
      transitionStateMatrix[i][j] = 0;
    }
  }
  //find count of transitions
  countTransitions(transitionStateMatrix, numProcesses,myFinishs);
  std::unordered_set<int> numberOfTerminalStates = getNumberOfTerminalStates(transitionStateMatrix,numProcesses);
  std::cout<<"\nThere are/is "<<numberOfTerminalStates.size()<<" Terminal State(s)\n";
  resultsFile<<"\nThere are/is "<<numberOfTerminalStates.size()<<" Terminal State(s)\n";
  int totalComplete = 0;
  for (auto itr = numberOfTerminalStates.begin(); itr != numberOfTerminalStates.end(); ++itr) {
    int numExit =numJobsComplete(myFinishs,*itr);
    totalComplete = totalComplete+numExit;
    std::cout<<"-Number of Jobs Exiting via PID "<<*itr<<": "<<numExit<<"\n";
    resultsFile<<"-Number of Jobs Exiting via PID "<<*itr<<": "<<numExit<<"\n";
    std::cout<<"-Average time to produce one job: "<<endTime/(float)numExit<<"\n\n";
    resultsFile<<"-Average time to produce one job: "<<endTime/(float)numExit<<"\n\n";
  }
  std::cout<<"Total Complete Jobs: "<<totalComplete<<"\n";
  resultsFile<<"Total Complete Jobs: "<<totalComplete<<"\n";
  std::cout<<"Overall average time per job: "<<endTime/(float)totalComplete<<"\n";
  resultsFile<<"Overall average time per job: "<<endTime/(float)totalComplete<<"\n";
  
  int maxJIS = getMaxJobsInSystem(myFinishs);
  std::cout<<"Max Number of Components in System: "<<maxJIS<<"\n";
  resultsFile<<"\nMax Number of Components in System: "<<maxJIS<<"\n";
  
  
  std::cout<<"There are "<<numProcesses<<" processes\n";
  resultsFile<<"Number of Processes: "<<numProcesses<<"\n";
  
  //create the transition time matrix
  float** transitionTimeMatrix = new float*[numProcesses];
  for(int i =0;i<numProcesses;++i){
    transitionTimeMatrix[i] = new float[numProcesses];
    for(int j = 0; j<numProcesses;++j){
      transitionTimeMatrix[i][j] = 0.0;
    }
  }
  
  std::cout<<"Transition State Matrix\n";
  resultsFile<<"\nTransition State Matrix\n";
  //printTransitionStateMatrix(transitionStateMatrix, numProcesses);
  
  printTransitionStateMatrix(transitionStateMatrix, numProcesses);
  
  getTransitionTimes(transitionTimeMatrix, numProcesses, myFinishs, myStarts);
  std::cout<<"Transition Times summed\n";
  resultsFile<<"\nTotal Transition Times Matrix\n";
  printTransitionTimeMatrix(transitionTimeMatrix, numProcesses);
  averageTransitionTimes(transitionTimeMatrix, numProcesses, transitionStateMatrix);
  std::cout<<"Average Transition Times averaged\n";
  resultsFile<<"\nAverage Transition Times Matrix\n";
  printTransitionTimeMatrix(transitionTimeMatrix, numProcesses);
  
  //create the buffer capacity matrix
  int** bufferCap = new int*[numProcesses];
  for(int i =0;i<numProcesses;++i){
    bufferCap[i] = new int[numProcesses];
    for(int j = 0; j<numProcesses;++j){
      bufferCap[i][j] = 0;
    }
  }
  
  getUtilizedBufferCapacity(bufferCap,transitionStateMatrix,numProcesses);
  std::cout<<"\nMax Utilized Buffer Capacity Matrix\n";
  resultsFile<<"\nMax Utilized Buffer Capacity Matrix\n";
  printTransitionStateMatrix(bufferCap, numProcesses);
  
  //create average process time array
  float* procTimes = new float[numProcesses];
  for(int i = 0;i<numProcesses;++i){
    procTimes[i]=0.0;
  }
  getAverageProcessTimes(procTimes, numProcesses);
  std::cout<<"\nAverage Process Times\n";
  resultsFile<<"\nAverage Process Times\n";
  for(int i = 0;i<numProcesses;++i){
    std::cout<<std::fixed<<std::setw(8)<<i<<" ";
    resultsFile<<std::fixed<<std::setw(8)<<i<<" ";
  }
  std::cout<<"\n";
  resultsFile<<"\n";
  for(int i = 0;i<numProcesses;++i){
    std::cout<<std::fixed<<std::setw(8)<<procTimes[i]<<"|";
    resultsFile<<std::fixed<<std::setw(8)<<procTimes[i]<<"|";
  }
  std::cout<<"\n";
  resultsFile<<"\n";
}

float DataCrawler::findStartTime(std::string id){
  for(int i=1;i<(int)myStarts.size();++i){
    if(myStarts[i].jobID.compare(id)==0){
      return myStarts[i].time;
    }
  }
  return 0.0;
}

processData DataCrawler::countNumberOfTimesPIDFinishes(int id){
  processData ans;
  ans.totaltime = 0.0;
  ans.sum = 0;
  for(int i = 1;i<(int)myFinishs.size();++i){
    std::string jid = myFinishs[i].jobID;
    if(getProcessID(jid)==id){
      float start = findStartTime(jid);
      //std::cout<<"JID "<<jid<<"starttime "<<start<<"\n";
      float duration = myFinishs[i].time - start;
      //std::cout<<"Duration of process "<<id<<" using jid "<<jid<<" is "<<duration<<"\n";
      ans.totaltime+= duration;
      ans.sum++;
    }
  }
  return ans;
}

void DataCrawler::getAverageProcessTimes(float*mytimes, int size){
  for(int i = 0;i<size;++i){
    processData info = countNumberOfTimesPIDFinishes(i);
    mytimes[i] = info.totaltime/(float)info.sum;
  }
}

//Description: Helper function used to parse a jobID and get the job number
int DataCrawler::getJobNum(std::string line){
  int ans;
  int posOfColon = getPosOfColon(line);
  int length = posOfColon -1;
  ans = std::atoi(line.substr(1,length).c_str());
  return ans;
}

//Description: Helper function to retreive the list of proper jobNumbers related to a downstream node connection
std::vector<int> DataCrawler::getProperStartConnections(int downstream,int upstream){
  std::vector<int> ans;
  for(int i=1;i<(int)myFinishs.size();++i){
    if(getProcessID(myFinishs[i].jobID)==downstream){
      //std::cout<<"Found process id "<<downstream<<" in jobid "<<myFinishs[i].jobID<<"\n";
      std::vector<std::string> depend = getUpConnections(myFinishs[i].jobID);
      for(int j = 0;j<depend.size();++j){
        //std::cout<<depend[j]<<"\n";
        if(depend[j]=="x"){
          continue;
        }
        int pid = getProcessID(depend[j]);
        if(pid == upstream){
          int jobNum = getJobNum(depend[j]);
          ans.push_back(jobNum);
        }
      }
    }
  }
  return ans;
}

//Description: Return how many exits to the system there are
std::unordered_set<int> DataCrawler::getNumberOfTerminalStates(int** freqMat, int size){
  std::unordered_set<int> ans;
  for(int i = 0;i<size;++i){
    int sum = 0;
    for(int j = 0;j<size;++j){
      sum += freqMat[i][j];
    }
    if(sum==0){
      ans.insert(i);
    }
  }
  return ans;
}

//Description: For the position check what was the max capacity used for the buffer
void DataCrawler::getCapacityForPos(int** buffMat,int upstream, int downstream){
  // the source represents the finishs and the destination if the start
  //start with gap to see if there is a utilized buffer
  std::vector<int> properStartConnections = getProperStartConnections(downstream, upstream);
  int startNum = 3;
  int bufferFound = 0;
  int finishNum = 1;
  int done = 0;
  int inBuffer = 0;
  int max = 0;
  int lastCheckedStartIndex = 0;
  int lastCheckedFinishIndex = 0;
  float startTime=0.0;
  float finishTime=0.0;
  while(!done){
    //start cheking for buffer case
    //find the relevent startjob with if of [startNum:destination-...
    int found1 = 1;
    for(int i = lastCheckedStartIndex;i<myStarts.size();++i){
      //if found then set found1 to 0 and break
      int jobNum = getJobNum(myStarts[i].jobID);
      int pid = getProcessID(myStarts[i].jobID);
      if(jobNum == properStartConnections[startNum-1] && pid == upstream){
        found1 = 0;
        lastCheckedStartIndex = i;
        startTime = myStarts[i].time;
      }
      if(found1==0)break;
    }
    int found2 = 1;
    for(int i = lastCheckedFinishIndex;i<myFinishs.size();++i){
      //if found then set found2 to 0 and break
      int jobNum = getJobNum(myFinishs[i].jobID);
      int pid = getProcessID(myFinishs[i].jobID);
      if(jobNum == finishNum && pid == downstream){
        found2 = 0;
        lastCheckedFinishIndex = i;
        finishTime = myFinishs[i].time;
      }
      if(found2==0)break;
    }
    if(found1==1||found2==1){
      done = 1;
      continue;
    }
    //use the times to check if there is a gap
    if(startTime < finishTime){
      //there is a buffer
      //need to add one
      inBuffer = inBuffer+1;
      if(inBuffer>max){
        max = inBuffer;
      }
      startNum++;
      bufferFound=1;
      continue;
    }
    else if(startTime >= finishTime && (startNum-finishNum)>2){
      inBuffer = inBuffer-1;
      finishNum++;
      continue;
    }
    else if(startTime >= finishTime && (startNum-finishNum)==2){
      startNum++;
      finishNum++;
      continue;
    }
  }
  //set the max seen
  buffMat[upstream][downstream] = max;
}

//Description: look through data to see what capacity of buffers was used
void DataCrawler::getUtilizedBufferCapacity(int** bufMat,int** freqMat,int size){
  //for every position with a valid transition check if there is a utilized buffer
  for(int i = 0;i<size;++i){
    for(int j = 0;j<size;++j){
      //check everyposition valid transition
      if(freqMat[i][j]){
        //valid transition and need to check
        int upstream = i;
        int dowstream = j;
        getCapacityForPos(bufMat, upstream, dowstream);
      }
    }
  }
}

//Description: Helper function to get the closing bracket of an initial opening bracket
int DataCrawler::getClosingBracket(int pos, std::string ID){
  int ans = -1;
  int ignore = 0;
  for(int i = pos;i<ID.length();++i){
    if(ID.substr(i,1)=="]" && ignore==0){
      ans = i;
      break;
    }
    else if(ID.substr(i,1)=="]" && ignore>0){
      ignore--;
    }
    else if(ID.substr(i,1)=="["){
      ignore++;
    }
  }
  return ans;
}

//Description: helper function used to find the next '[' character from a given position
int DataCrawler::posOfNextBracket(std::string line, int start){
  int ans = -1;
  for(int i = start;i<(int)line.length();++i){
    if(line[i] == '['){
      ans = i;
      break;
    }
  }
  return ans;
}

//Description: find the position of the first ':' in the string
int DataCrawler::getPosOfColon(std::string line){
  int pos = -1;
  for(int i = 1;i<line.length();++i){
    if(line[i] == ':'){
      pos = i;
      break;
    }
  }
  return pos;
}

//Description: get the position of the first '-' in the string
int DataCrawler::getPosOfDash(std::string line, int start){
  int pos = -1;
  for(int i = start;i<line.length();++i){
    if(line[i] == '-'){
      pos = i;
      break;
    }
  }
  return pos;
}

//Description: extract the dependency string from the job id string
std::string DataCrawler::getDependancy(std::string line){
  std::string ans;
  int start = getPosOfColon(line);
  int end = getPosOfDash(line, start);
  int begin = end+2;
  int length = (int)line.length()-2-begin;
  ans = line.substr(begin,length);
  return ans;
}

//Description: get all dependencies within dependency string
std::vector<std::string> DataCrawler::getUpConnections(std::string line){
  std::vector<std::string> depend;
  std::string dependStr = getDependancy(line);
  if(dependStr=="x"){
    depend.push_back("x");
    return depend;
  }
  int index = 0;
  while(index != dependStr.length()-1){
    index = posOfNextBracket(dependStr, index);
    int closingPos = getClosingBracket(index+1, dependStr);
    std::string val = dependStr.substr(index,closingPos-index+1);
    depend.push_back(val);
    index = closingPos;
  }
  return depend;
}

//Description: read in the start events into memory for storage
std::vector<Start> DataCrawler::readInStartsFile(){
  std::vector<Start> readStarts;
  std::string line;
  int lineNum = 0;
  while(std::getline(myStartFile,line)){
    Start aStart;
    std::stringstream ss(line);
    std::string token;
    std::getline(ss,token, ',');
    aStart.jobID = token;
    if(lineNum != 0){
      aStart.connections = getUpConnections(token);
    }
    std::getline(ss,token, ',');
    aStart.time = std::atof(token.c_str());
    std::getline(ss,token, ',');
    aStart.jobsInSystem = std::atoi(token.c_str());
    readStarts.push_back(aStart);
    lineNum++;
  }
  return readStarts;
}

//Description: read in finish events into memory for storage and use
std::vector<Finish> DataCrawler::readInFinishFile(){
  std::vector<Finish> readFinish;
  std::string line;
  int lineNum = 0;
  while(std::getline(myFinishFile,line)){
    Finish aFinish;
    std::stringstream ss(line);
    std::string token;
    std::getline(ss,token, ',');
    aFinish.jobID = token;
    std::getline(ss,token, ',');
    aFinish.time = std::atof(token.c_str());
    std::getline(ss,token, ',');
    aFinish.jobsInSystem = std::atoi(token.c_str());
    readFinish.push_back(aFinish);
    lineNum++;
  }
  return readFinish;
}

//Description: return the number of jobs completed in simulation run from data
int DataCrawler::numJobsComplete(std::vector<Finish> &myFin,int pid){
  int max=0;
  for(int i=0;i<(int)myFin.size();++i){
    int Checkpid = getProcessID(myFin[i].jobID);
    if(Checkpid==pid){
      int num = getJobNum(myFin[i].jobID);
      if(num>max){
        max=num;
      }
    }
  }
  return max;
}

//Description: Return the number of unique processes in the system
int DataCrawler::getNumProcesses(std::vector<Finish> &myFin){
  std::unordered_set<int> myPIDs;
  for(int i = 1;i<(int)myFin.size();++i){
    std::string line = myFin[i].jobID;
    int start = getPosOfColon(line);
    int end = getPosOfDash(line, start);
    int length = end - start -1;
    int pid = std::atoi(line.substr(start+1,length).c_str());
    myPIDs.insert(pid);
  }
  return (int)myPIDs.size();
}

//Description:Print the transition frequency matrix to results and console
void DataCrawler::printTransitionStateMatrix(int** mat, int size){
  std::cout<<std::setw(3)<<"PID ";
  resultsFile<<std::setw(3)<<"PID ";
  for(int i = 0;i<size;++i){
    std::cout<<std::setw(3)<<i<<" ";
    resultsFile<<std::setw(3)<<i<<" ";
  }
  std::cout<<"\n";
  resultsFile<<"\n";
  for(int i = 0;i<size;++i){
    std::cout<<std::setw(3)<<i<<"|";
    resultsFile<<std::setw(3)<<i<<"|";
    for(int j = 0;j<size;++j){
      std::cout<<std::setw(3)<<mat[i][j]<<"|";
      resultsFile<<std::setw(3)<<mat[i][j]<<"|";
    }
    std::cout<<"\n";
    resultsFile<<"\n";
  }
}

//Description:Print the transition time matrix to results and console
void DataCrawler::printTransitionTimeMatrix(float** mat, int size){
  std::cout<<std::setw(9)<<std::right<<" PID";
  resultsFile<<std::setw(9)<<std::right<<" PID";
  for(int i = 0;i<size;++i){
    std::cout<<std::fixed<<std::setw(10)<<std::right<<i;
    resultsFile<<std::fixed<<std::setw(10)<<std::right<<i;
  }
  std::cout<<"\n";
  resultsFile<<"\n";
  for(int i = 0;i<size;++i){
    std::cout<<std::setw(9)<<std::right<<i<<"|";
    resultsFile<<std::setw(9)<<std::right<<i<<"|";
    for(int j = 0;j<size;++j){
      std::cout<<std::setfill(' ')<<std::fixed<<std::right<<std::setw(9)<<std::setprecision(3)<<mat[i][j]<<"|";
      resultsFile<<std::setfill(' ')<<std::fixed<<std::right<<std::setw(9)<<std::setprecision(3)<<mat[i][j]<<"|";
    }
    std::cout<<"\n";
    resultsFile<<"\n";
  }
}

//Description: Return the pid from a given jid string
int DataCrawler::getProcessID(std::string line){
  int start = getPosOfColon(line);
  int end = getPosOfDash(line, start);
  int length = end - start -1;
  int pid = std::atoi(line.substr(start+1,length).c_str());
  return pid;
}

//Description: get all the process IDs in a provided string
std::vector<int> DataCrawler::getPIDS(std::string line){
  std::vector<int> ans;
  int index = 0;
  while(index != line.length()-1){
    index = posOfNextBracket(line, index);
    int closingPos = getClosingBracket(index+1, line);
    int pid = getProcessID(line.substr(index+1,closingPos-index-1));
    ans.push_back(pid);
    index = closingPos;
  }
  return ans;
}

//Description: sum all transition counts in matrix
void DataCrawler::countTransitions(int **mat, int size, std::vector<Finish> &myfin){
  for(int i = 1;i<myfin.size();++i){
    std::string jid = myfin[i].jobID;
    int pid = getProcessID(jid);
    //need to get upstream transitions
    std::string depend = getDependancy(jid);
    if(depend == "x")continue;
    //need to parse dependency to get the PID of each upstram task if not x
    std::vector<int> upstream = getPIDS(depend);
    for(int j = 0;j<upstream.size();++j){
      int process = upstream[j];
      mat[process][pid] = mat[process][pid]+1;
    }
  }
}

//Description: get the next start event info for a given finish job id
startInfo DataCrawler::getStart(std::string id,std::vector<Start> &myStart){
  startInfo info;
  for(int i=1;i<(int)myStart.size();++i){
    int depNum = (int)myStart[i].connections.size();
    for(int j = 0;j<depNum;++j){
      if(myStart[i].connections[j].compare(id)==0){
        //they are equal and return info
        info.startTime = myStart[i].time;
        info.Pid = getProcessID(myStart[i].jobID);
        return info;
      }
    }
  }
  info.startTime = -999.999;
  info.Pid = -99;
  return info;
}

//Description: Sum the transition times into the transition time matrix
void DataCrawler::getTransitionTimes(float** mat, int size, std::vector<Finish> &myFin, std::vector<Start> &myStart){
  //for every finish except last position check when it begins next and is the major dependency of the start
  for(int i = 1;i<(int)myFin.size()-1;++i){
    int pid = getProcessID(myFin[i].jobID);
    float finishTime = myFin[i].time;
    startInfo info = getStart(myFin[i].jobID,myStart);
    if(info.Pid <0)continue;
    float elapsedTime = info.startTime - finishTime;
    mat[pid][info.Pid] =  mat[pid][info.Pid] + elapsedTime;
  }
}

//Description: Use frequencies and summed amounts to provide an average transition time matrix
void DataCrawler::averageTransitionTimes(float** mat, int size,int** matFreq){
  for(int i = 0;i<size;++i){
    for(int j = 0;j<size;++j){
      if(matFreq[i][j]==0)continue;
      mat[i][j] = mat[i][j]/(float)matFreq[i][j];
    }
  }
}

//Description: Return the max seen number of jobs in system from finish events
int DataCrawler::getMaxJobsInSystem(std::vector<Finish> &myFin){
  int max = myFin[0].jobsInSystem;
  for(int i = 1;i<(int)myFin.size();++i){
    if(myFin[i].jobsInSystem > max){
      max = myFin[i].jobsInSystem;
    }
  }
  return max;
}

