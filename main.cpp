#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <map>
#include <vector>
#include <sstream>
#include <string>
#include <sys/types.h>
#include <math.h>
#include <sys/wait.h>
#include <signal.h>
#include <semaphore.h>
#include "LinkedList.h"
using namespace std;

void printList(int array[], int size)
{
  for(int b = 0; b < size; b++)
    {
      cout << array[b] << " ";
    }
    cout << endl;
}

struct Process{
  int pid;
  int frames;
};

int main(int argc, char* argv[]) {
  //Create and get all variables to hold inputs
  int totalPageFrames, pageSize, framesPerAlgo, lookAhead, minPool, maxPool, numProcesses;
  ifstream fin(argv[1]);
  fin >> totalPageFrames;
  fin >> pageSize;
  fin >> framesPerAlgo;
  fin >> lookAhead;
  fin >> minPool;
  fin >> maxPool;
  fin >> numProcesses;

  //Get number of page frames for each process
  Process processesInfo[numProcesses];
  for(int a = 0; a < numProcesses; a++)
    {
      fin >> processesInfo[a].pid >> processesInfo[a].frames;
    }
  //Get our input list
  LinkedList list;
  string input;
  while(getline(fin, input))
    list.add(input, pageSize);

  list.head = list.head->next;

  //Initialize semaphores that control how many processes to run based on free pool size
  int processesToRun = floor((totalPageFrames-minPool)/framesPerAlgo);
  sem_t sem;
  sem_init(&sem, 1, processesToRun);

  //Create child processes and do tasks
  int pids[numProcesses];
  for(int a = 0; a < numProcesses; a++)
    {
      pids[a] = fork();
      //Error with forks
      if(pids[a] == -1)
        {
          cout << "Error creating forks" << endl;
          return 1;
        }
      //Child process
      if(pids[a] == 0)
      {
        //Wait for semaphore to be available
        sem_wait(&sem);

        int processID = processesInfo[a].pid;
        int faultsArr[6]; //Holds total faults for each algo and prints at the end
        int pageFaults = 0; //Holds faults for current algo

        //Variables to aid in the algorithms
        Node* current = list.head;
        bool inArray = false;
        int pageFrames[framesPerAlgo];
        for(int b = 0; b < framesPerAlgo; b++)
        {
          pageFrames[b] = -10;
        }
        
        //Start LIFO algorithm
        while(current != nullptr)
          {
            inArray = false;
            //Checks to see if the input is for its own process
            if(current->process == processID)
            {
              //Break if process is finished
              if(current->address == -1)
                break;
              //Find what frame the current address is in
              int frame = current->address;
              //Check if page frame is in array
              for(int b = 0; b < framesPerAlgo; b++)
                {
                  if(pageFrames[b] == frame)
                  {
                    inArray = true;
                    break;
                  }
                }
              //Not in page frames, increment page faults and put page in array
              if(inArray == false)
              {
                pageFaults++;
                for(int b = 0; b < framesPerAlgo; b++)
                  {
                    //Put page in array if there is an empty spot
                    if(pageFrames[b] == -10)
                    {
                      pageFrames[b] = frame;
                      break;
                    }
                    //If no empty spots then swap the last element added to array
                    else if(b == framesPerAlgo - 1)
                    {
                      pageFrames[b] = frame;
                    }
                  }
              }
              //printList(pageFrames, framesPerAlgo);
            }
            current = current->next;
          }
        faultsArr[0] = pageFaults;
        
        //Reset array, page faults num, and current pointer to start another algorithm
        for(int b = 0; b < framesPerAlgo; b++)
        {
          pageFrames[b] = -10;
        }
        pageFaults = 0;
        current = list.head;

        //Start MRU algorithm
        int mostRecent = -1; //Keep track of the index most recently used in the array 
        while(current != nullptr)
          {
            inArray = false;
            //Checks to see if the input is for its own process
            if(current->process == processID)
            {
              //Break if process is finished
              if(current->address == -1)
                break;
              //Find what frame the current address is in
              int frame = current->address;
              //Check if page frame is in array
              for(int b = 0; b < framesPerAlgo; b++)
                {
                  if(pageFrames[b] == frame)
                  {
                    inArray = true;
                    mostRecent = b;
                    break;
                  }
                }
              //Not in page frames, increment page faults and put page in array
              if(inArray == false)
              {
                pageFaults++;
                for(int b = 0; b < framesPerAlgo; b++)
                  {
                    //Put page in array if there is an empty spot
                    if(pageFrames[b] == -10)
                    {
                      pageFrames[b] = frame;
                      mostRecent = b;
                      break;
                    }
                    //If no empty spots then replace the index most recently used
                    else if(b == framesPerAlgo - 1)
                    {
                      pageFrames[mostRecent] = frame;
                    }
                  }
              }
              //printList(pageFrames, framesPerAlgo);
            }
            current = current->next;
          }
        faultsArr[1] = pageFaults;

        //Reset array, page faults num, and current pointer to start another algorithm
        for(int b = 0; b < framesPerAlgo; b++)
        {
          pageFrames[b] = -10;
        }
        pageFaults = 0;
        current = list.head;

        //Start LFU algorithm
        map<int,int> map; //Stores all page frames used and number of times used
        while(current != nullptr)
        {
          inArray = false;
          //Checks to see if the input is for its own process
          if(current->process == processID)
          {
            //Break if process is finished
            if(current->address == -1)
              break;
            //Find what frame the current address is in
            int frame = current->address;
            //Check if page frame is in array
            for(int b = 0; b < framesPerAlgo; b++)
              {
                if(pageFrames[b] == frame)
                {
                  inArray = true;
                  map.at(frame)++; //Increment the number of times used for that page frame
                  break;
                }
              }
            //If not in page frames, increment page faults and put page in array
            if(inArray == false)
            {
              pageFaults++;
              for(int b = 0; b < framesPerAlgo; b++)
                {
                  //Put page in array if there is an empty spot
                  if(pageFrames[b] == -10)
                  {
                    pageFrames[b] = frame;
                    map.emplace(frame, 1);
                    break;
                  }
                  //If no empty spots then replace the index least frequently used
                  else if(b == framesPerAlgo - 1)
                  {
                    int leastUsed = 0;
                    //Finds the least frequently used index
                    for(int c = 1; c < framesPerAlgo; c++)
                      {
                        if(map.at(pageFrames[leastUsed]) > map.at(pageFrames[c]))
                          leastUsed = c;
                      }
                    //If new frame has been used increment the counter, if not enter it into the map with count 1
                    if(map.count(frame))
                      map.at(frame)++;
                    else
                      map.emplace(frame, 1);
                    pageFrames[leastUsed] = frame;
                  }
                }
            }
            //printList(pageFrames, framesPerAlgo);
          }
          current = current->next;
        }
        faultsArr[2] = pageFaults;

        //Reset array, page faults num, and current pointer to start another algorithm
        for(int b = 0; b < framesPerAlgo; b++)
        {
          pageFrames[b] = -10;
        }
        pageFaults = 0;
        current = list.head;
        
        //Start WS Algorithm
        int count = 0; //Checks how many frames are processed, once past delta run the window sliding algorithm
        vector<int> vec; //Holds the previous entries
        while(current != nullptr)
        {
          inArray = false;
          //Checks to see if the input is for its own process
          if(current->process == processID)
          {
            //Break if process is finished
            if(current->address == -1)
              break;
            //Find what frame the current address is in
            int frame = current->address;
            vec.push_back(frame);
            //Check if page frame is in array
            if(count < lookAhead)
            {
              for(int b = 0; b < framesPerAlgo; b++)
                {
                  if(pageFrames[b] == frame)
                  {
                    inArray = true;
                    break;
                  }
                }
              //If not in page frames, increment page faults and put page in array
              if(inArray == false)
              {
                pageFaults++;
                for(int b = 0; b < framesPerAlgo; b++)
                {
                  //Put page in array if there is an empty spot
                  if(pageFrames[b] == -10)
                  {
                    pageFrames[b] = frame;
                    break;
                  }
                }
              }
            }
            else //We know count > lookahead so we can run the window sliding algorithm without out of bnds error
            {
              //Check if page frame is in array
              for(int b = 0; b < framesPerAlgo; b++)
              {
                if(pageFrames[b] == frame)
                {
                  inArray = true;
                  break;
                }
              }
              //If not in page frames, increment page faults and put page in array
              if(inArray == false)
              {
                pageFaults++;
                std::map<int, int> myMap;
                //Slide the window for WS over
                for(int d = 0; d < lookAhead; d++)
                  {
                    if(myMap.count(vec.at((vec.size() - d) - 1)))
                      pageFrames[d] = -10;
                    else
                    {
                      myMap.emplace(vec.at((vec.size() - d) - 1), 1);
                      pageFrames[d] = vec.at((vec.size() - d) - 1);
                    }
                  }
              }
              //Here we know new page frame is in the array, so don't increment page faults, but still run the window sliding algorithm
              else if(inArray == true)
              {
                std::map<int, int> myMap;
                for(int d = 0; d < lookAhead; d++)
                  {
                    if(myMap.count(vec.at((vec.size() - d) - 1)))
                      pageFrames[d] = -10;
                    else
                    {
                      myMap.emplace(vec.at((vec.size() - d) - 1), 1);
                      pageFrames[d] = vec.at((vec.size() - d) - 1);
                    }
                  }
              }
            }
            //printList(pageFrames, framesPerAlgo);
            count++;
          }
          current = current->next;
        }
        faultsArr[3] = pageFaults;

        //Reset array, page faults num, and current pointer to start another algorithm
        for(int b = 0; b < framesPerAlgo; b++)
        {
          pageFrames[b] = -10;
        }
        pageFaults = 0;
        current = list.head;

        //Start OPT-X algorithm
        while(current != nullptr)
          {
            inArray = false;
            //Checks to see if the input is for its own process
            if(current->process == processID)
            {
              //Break if process is finished
              if(current->address == -1)
                break;
              //Find what frame the current address is in
              int frame = current->address;
              //Check if page frame is in array
              for(int b = 0; b < framesPerAlgo; b++)
                {
                  if(pageFrames[b] == frame)
                  {
                    inArray = true;
                    break;
                  }
                }
              //Not in page frames, increment page faults and put page in array
              if(inArray == false)
              {
                pageFaults++;
                for(int b = 0; b < framesPerAlgo; b++)
                  {
                    //Put page in array if there is an empty spot
                    if(pageFrames[b] == -10)
                    {
                      pageFrames[b] = frame;
                      break;
                    }
                    //If no empty spots then find the optimal swap
                    else if(b == framesPerAlgo - 1)
                    {
                      std::map<int,int> map2;
                      Node* current2 = current;
                      //Finds which element comes when by looking ahead by the given amount of look ahead
                      for(int c = 0; c < lookAhead; c++)
                        {
                          current2 = current2->next;
                          while(current2->process != processesInfo[a].pid)
                            {
                              current2 = current2->next;
                            }
                          if(current2->address == -1)
                            break;
                          map2.emplace(current2->address, c);
                        }
                      int farthest = -1;
                      //Checks to see which element in the page frames shows up the furthest
                      for(int c = 0; c < framesPerAlgo; c++)
                        {
                          if(map2.count(pageFrames[c]) == 0)
                          {
                            farthest = c;
                            break;
                          }
                          else
                          {
                            if(farthest == -1)
                              farthest = c;
                            else if(map2.at(pageFrames[farthest]) < map2.at(pageFrames[c]))
                              farthest = c;
                          }
                        }
                      pageFrames[farthest] = frame;
                    }
                  }
              }
              //printList(pageFrames, framesPerAlgo);
            }
            current = current->next;
          }
        faultsArr[4] = pageFaults;

        //Reset array, page faults num, and current pointer to start another algorithm
        for(int b = 0; b < framesPerAlgo; b++)
        {
          pageFrames[b] = -10;
        }
        pageFaults = 0;
        current = list.head;

        //Start LRU-X algorithm
        int count2 = 1; //Keeps track of how many frames processed
        int leastRecent[framesPerAlgo]; //Keep track of the elements and their last use times 
        while(current != nullptr)
          {
            inArray = false;
            //Checks to see if the input is for its own process
            if(current->process == processID)
            {
              //Break if process is finished
              if(current->address == -1)
                break;
              //Find what frame the current address is in
              int frame = current->address;
              //Check if page frame is in array, if it is update the time used
              for(int b = 0; b < framesPerAlgo; b++)
                {
                  if(pageFrames[b] == frame)
                  {
                    inArray = true;
                    leastRecent[b] = count2;
                    break;
                  }
                }
              //If not in page frames, increment page faults and put page in array
              if(inArray == false)
              {
                pageFaults++;
                for(int b = 0; b < framesPerAlgo; b++)
                  {
                    //Put page in array if there is an empty spot, and update time used
                    if(pageFrames[b] == -10)
                    {
                      pageFrames[b] = frame;
                      leastRecent[b] = count2;
                      break;
                    }
                    //If no empty spots then replace the index least recently used, and update new frame time used
                    else if(b == framesPerAlgo - 1)
                    {
                      int leastRecentlyUsed = 0;
                      for(int c = 1; c < framesPerAlgo; c++)
                        {
                          if(leastRecent[c] < leastRecent[leastRecentlyUsed])
                            leastRecentlyUsed = c;
                        }
                      pageFrames[leastRecentlyUsed] = frame;
                      leastRecent[leastRecentlyUsed] = count2;
                    }
                  }
              }
              //printList(pageFrames, framesPerAlgo);
              count2++;
            }
            current = current->next;
          }
        faultsArr[5] = pageFaults;

        
        cout << "===== Process " << processesInfo[a].pid << " ===== " << endl
        << "Page Faults for LIFO Process " << processesInfo[a].pid << ": " << faultsArr[0] << endl 
        << "Page Faults for MRU Process " << processesInfo[a].pid << ": " << faultsArr[1] << endl 
        << "Page Faults for LFU Process " << processesInfo[a].pid << ": " << faultsArr[2] << endl
        << "Page Faults for WS Process " << processesInfo[a].pid << ": " << faultsArr[3] << endl
        << "- Working Set Minimum = 1 : Working Set Maximum = " << framesPerAlgo << endl
        << "Page Faults for OPT-X Process " << processesInfo[a].pid << ": " << faultsArr[4] << endl
        << "Page Faults for LRU-X Process " << processesInfo[a].pid << ": " << faultsArr[5] << endl << endl;
        //Increment the semaphore so another process can run
        sem_post(&sem);
        exit(0);
      }
      //Parent process, wait for all childs to finish
      else
        for (int a = 0; a < numProcesses; a++) 
        {
          waitpid(pids[a], NULL, 0);
        }
    }
  
  //Free resources
  fin.close();
  sem_destroy(&sem);
  return 0;
}