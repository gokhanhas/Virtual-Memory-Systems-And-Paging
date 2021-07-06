/*
##########################################
#        Gokhan Has - 161044067          #
#       CSE 312 - Final Project          #
# Lecturer : Prof. Dr. Yusuf Sinan Akgul #
##########################################
*/

#include <iostream>
#include <stdlib.h>
#include <string>
#include <cctype>
#include <ctime>
#include <pthread.h>
#include <sys/time.h>
#include <cmath>
#include <cstring>
#include <queue>
#include <list>
#include <vector>
#include <iterator>

#include "clockQueue.h"
#include "pageentry.h"
#include "queue.h"


using namespace std;

typedef struct statics {
    int numberOfReads;
    int numberOfWrites;
    int pageMisses;
    int pageReplacements;
    int diskPageWrites;
    int diskPageReads;
} Statistics_t;


/* GLOBAL VARIABLES */
int* pyhsicalMEM = NULL;
int* virtualMemArr = NULL;
int pyhsicalMEM_size = 0;


PageEntry** virtualMEM;
int virtualMEM_size = 0;
FILE* diskFile = NULL;


int frameSize = 0, numPhysical = 0, numVirtual = 0;
int pageTablePrint = 0;
string pageReplacement = "", allocPolicy = "";
string diskFileName = "";

/* PAGE REPLACEMENT ALGORITHMS VARIABLES*/
queue <int> FIFOqueue;
Queue* SCqueue;
list<int> NRUlist;
vector<int> LRUvector;
vector<int> LRUclock;
ClockQueue* WSClockqueue;

// THREADS ..
pthread_t threadBubble;
pthread_t threadQuick;
pthread_t threadMerge;
pthread_t threadIndex;
pthread_mutex_t control;

// For statistics
Statistics_t statisticArr[6] = {{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},
                                {0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0}};

int accesCount = 0;

/* FUNCTION DEFINES */
void printUsage();
void set(unsigned int index, int value, char* tName);
void helperSet(unsigned int index, int value, char t);
int get(unsigned int index, char* tName);
void fillPhysicalFirst();
void fillVirtualMem(); 
int returnDiskValue(unsigned int index);
int getPhysicalIndex(int value);
void changeOldVirtual(int value);
void getIndexes(int* first, int* last, char* tName); 
int getNRUClassNumber(bool R, bool M);
int getLRUClockTime(int *old); 
bool checkFuntion(); 
void freeVM();
void bubbleSort(int first, int last);
int partition(int first, int last);
void quickSort(int first, int last);
void mergeSort(int first, int last);
void merge(int first, int last, int middle);
void indexSort(int* arr, int first, int last);
void* threadBubbleSort(void* argument);
void* threadQuickSort(void* argument);
void* threadMergeSort(void* argument);
void* threadIndexSort(void* argument);
void printPageTable();


int main(int arg, char* argv[]) {

    if(arg < 8 || arg > 8) {
        printUsage();
        exit(EXIT_FAILURE);
    }

    frameSize = stoi(argv[1]);
    numPhysical = stoi(argv[2]);
    numVirtual = stoi(argv[3]);
    pageReplacement = argv[4];
    allocPolicy = argv[5];
    pageTablePrint = stoi(argv[6]);
    diskFileName = argv[7];

    if(numPhysical > numVirtual) {
        cout << "ERROR ! numPhysical is greater than numVirtual" << endl;
        printUsage();
        exit(EXIT_FAILURE);
    }

    if(!(strcmp(allocPolicy.c_str(),"global") == 0 || strcmp(allocPolicy.c_str(),"local") == 0 )) {
        cout << "ERROR ! allocPolicy must be local or global" << endl;
        printUsage();
        exit(EXIT_FAILURE);
    }

    if(!(strcmp(pageReplacement.c_str(),"FIFO") == 0 || strcmp(pageReplacement.c_str(),"NRU") == 0 || 
        strcmp(pageReplacement.c_str(),"SC") == 0 || strcmp(pageReplacement.c_str(),"LRU") == 0 || strcmp(pageReplacement.c_str(),"WSClock") == 0)) {
        cout << "ERROR ! pageReplacement is wrong FIFO, NRU, SC, LRU or WSClock" << endl;
        printUsage();
        exit(EXIT_FAILURE);
    }

    virtualMemArr = (int*) malloc(sizeof(int) * pow(2,numVirtual));
    virtualMEM_size = pow(2,numVirtual) * pow(2, frameSize);
    pyhsicalMEM_size = pow(2,numPhysical) * pow(2, frameSize);
    pyhsicalMEM = (int*) malloc(sizeof(int) * pyhsicalMEM_size);
    virtualMEM = new PageEntry*[virtualMEM_size];

    if(strcmp(pageReplacement.c_str(),"SC") == 0) {
        SCqueue = (Queue*) malloc(sizeof(Queue) * 1);
        initializeQueue(SCqueue);
    }
        
    if(strcmp(pageReplacement.c_str(),"WSClock") == 0) {
        WSClockqueue = (ClockQueue*) malloc(sizeof(ClockQueue) * 1);
        initializeClockQueue(WSClockqueue);
    }
        
    if(pthread_mutex_init(&control, NULL) != 0) {
        cout << "ERROR ! mutex init " << endl;
        exit(EXIT_FAILURE);
    }

    fillPhysicalFirst();
    char name[5] = "fill";
    set(0,0, name);

    if(pthread_create(&(threadBubble), NULL, threadBubbleSort, NULL) != 0) {
        cout << "ERROR ! pthread_create problem bubble !" << endl;
        exit(EXIT_FAILURE);
    }
    if(pthread_create(&(threadQuick), NULL, threadQuickSort, NULL) != 0) {
        cout << "ERROR ! pthread_create problem quick !" << endl;
        exit(EXIT_FAILURE);
    }
    if(pthread_create(&(threadMerge), NULL, threadMergeSort, NULL) != 0) {
        cout << "ERROR ! pthread_create problem merge !" << endl;
        exit(EXIT_FAILURE);
    }
    if(pthread_create(&(threadIndex), NULL, threadIndexSort, NULL) != 0) {
        cout << "ERROR ! pthread_create problem index !" << endl;
        exit(EXIT_FAILURE);
    }
    


    if(pthread_join(threadBubble, NULL) != 0) {
        cout << "ERROR ! pthread_join bubble !" << endl;
        exit(EXIT_FAILURE);
    }
    if(pthread_join(threadQuick, NULL) != 0) {
        cout << "ERROR ! pthread_join quick !" << endl;
        exit(EXIT_FAILURE);
    }
    if(pthread_join(threadMerge, NULL) != 0) {
        cout << "ERROR ! pthread_join merge !" << endl;
        exit(EXIT_FAILURE);
    }
    if(pthread_join(threadIndex, NULL) != 0) {
        cout << "ERROR ! pthread_join index !" << endl;
        exit(EXIT_FAILURE);
    }

    if(pthread_mutex_destroy(&control) != 0) {
        cout << "ERROR ! pthread_mutex_destroy problem !" << endl;
        exit(EXIT_FAILURE);
    }
    
    char nameCheck[6] = "check";
    set(0, 0, nameCheck);
    
    int i;
    for(i = 0; i < 6; i++) {
        if(i == 0) 
            cout << "FILL STATISTICS" << endl;
        else if(i == 1) 
            cout << "BUBBLE STATISTICS" << endl;
        else if(i == 2) 
            cout << "QUICK STATISTICS" << endl;
        else if(i == 3) 
            cout << "MERGE STATISTICS" << endl;
        else if(i == 4) 
            cout << "INDEX STATISTICS" << endl;
        else if(i == 5)
            cout << "CHECK STATISTICS" << endl;
            
        cout << "NUMBER OF READS : " << statisticArr[i].numberOfReads << endl;
        cout << "NUMBER OF WRITES : " << statisticArr[i].numberOfWrites << endl;
        cout << "NUMBER OF PAGE MISSES : " << statisticArr[i].pageMisses << endl;
        if(i != 5)
            cout << "NUMBER OF PAGE REPLACEMENTS : " << statisticArr[i].pageReplacements << endl;
        else
            cout << "NUMBER OF PAGE REPLACEMENTS : " << statisticArr[i].pageMisses << endl;
        cout << "NUMBER OF DISK PAGE WRITES : " << statisticArr[i].diskPageWrites << endl;
        cout << "NUMBER OF DISK PAGE READS : " << statisticArr[i].diskPageReads << endl << endl << endl;   
    }
    freeVM();
    free(pyhsicalMEM);
    if(strcmp(pageReplacement.c_str(),"SC") == 0)
        freeQueue(SCqueue);
    free(virtualMemArr);
}

void printUsage() {
    cout << "USAGE !!!" << endl;
    cout << "###########################################################################" << endl;
    cout << "#      ./sortArrays frameSize numPhysical numVirtual pageReplacement      #" << endl;
    cout << "#          allocPolicy pageTablePrintInt diskFileName.dat                 #" << endl;
    cout << "#           frameSize : is the size of the page frames                    #" << endl;
    cout << "# numPhysical : is the total number of page frames in the physical memory #" << endl;
    cout << "# numVirtual : is the total number of frames in the virtual address space #" << endl;
    cout << "#  pageReplacement : is the page replacement algorithm(NRU,FIFO,SC,...)   #" << endl;
    cout << "#        allocPolicy : is the allocation policy (global or local)         #" << endl;
    cout << "#  pageTablePrintInt : is theinterval of memory accesses after which the  #" << endl; 
    cout << "#                      page table is printed on screen                    #" << endl;
    cout << "#     diskFileName.dat : is the name of the file that keeps the out       #" << endl;
    cout << "#                            of memory frames                             #" << endl;
    cout << "###########################################################################" << endl;
}

void set(unsigned int index, int value, char* tName) {
    accesCount++;
    if(accesCount % pageTablePrint == 0)
        printPageTable();
    
    if(strcmp(tName, "fill") == 0) {
        fillVirtualMem();
    }
    else if(strcmp(tName, "bubble") == 0) {
        helperSet(index, value, 'b');
    }
    else if(strcmp(tName, "quick") == 0) {
        helperSet(index, value, 'q');
    }
    else if(strcmp(tName, "merge") == 0) {
        helperSet(index, value, 'm');
    }
    else if(strcmp(tName, "index") == 0) {
        helperSet(index, value, 'i');
    }
    else if(strcmp(tName, "check") == 0) {
        checkFuntion();
    }
    else {
        return ;
    }
}

void helperSet(unsigned int index, int value, char t) {
    if(!virtualMEM[index]->getPresentAbsentBit()) {
        diskFile = fopen(diskFileName.c_str(), "r+");
        if(diskFile == NULL) {
            cout << "ERROR ! DISK FILE DOES NOT OPENED !" << endl;
            exit(EXIT_FAILURE);
        }
        if(getPhysicalIndex(index) == -9) {
            pyhsicalMEM[index] = value;
        }
        else {
            int val = -1;
            fseek(diskFile,index*12,SEEK_SET);
            fscanf(diskFile,"%d\n",&val);
            pyhsicalMEM[index] = val;
        }
        fclose(diskFile);
    }
}

int get(unsigned int index, char* tName) {
    
    accesCount++;
    if(accesCount % pageTablePrint == 0)
        printPageTable();
    
    if(virtualMEM[index]->getPresentAbsentBit()) {
        // In Physical memory ...
        if(strcmp(tName,"bubble") == 0) {
            statisticArr[1].numberOfReads += 1;
        } 
        else if(strcmp(tName,"quick") == 0) {
            statisticArr[2].numberOfReads += 1;
        }
        else if(strcmp(tName,"merge") == 0) {
            statisticArr[3].numberOfReads += 1;
        }
        else if(strcmp(tName,"index") == 0) {
            statisticArr[4].numberOfReads += 1;
        }
        else if(strcmp(tName,"check") == 0) {
            statisticArr[5].numberOfReads += 1;
        }
        return pyhsicalMEM[virtualMEM[index]->getIndex()];
    }
    else {
        // PAGE REPLECAMENTS ALGORITMS ...
        if(strcmp(pageReplacement.c_str(),"FIFO") == 0) {
            // FIFOYA GORE PAGE REPLACEMENT ...
            if(strcmp(allocPolicy.c_str(),"global") == 0) {
                int count = getx(); 
                // ALLOCYPOLICY IS GLOBAL ..
                int oldIndex = FIFOqueue.front();
                FIFOqueue.pop();

                // DISKDEN DEGER ALINACAK...
                int value = returnDiskValue(index);
                if(strcmp(tName,"bubble") == 0) {
                    statisticArr[1].numberOfReads += count;
                    statisticArr[1].pageMisses += 1;
                    statisticArr[1].pageReplacements += 1;
                    statisticArr[1].numberOfWrites += count % 12;
                    statisticArr[1].diskPageWrites += count;
                    statisticArr[1].diskPageReads += 1;
                } 
                else if(strcmp(tName,"quick") == 0) {
                    statisticArr[2].numberOfReads += count;
                    statisticArr[2].pageMisses += 1;
                    statisticArr[2].pageReplacements += 1;
                    statisticArr[2].numberOfWrites += count % 12;
                    statisticArr[2].diskPageWrites += count;
                    statisticArr[2].diskPageReads += 1;
                }
                else if(strcmp(tName,"merge") == 0) {
                    statisticArr[3].numberOfReads += count;
                    statisticArr[3].pageMisses += 1;
                    statisticArr[3].pageReplacements += 1;
                    statisticArr[3].numberOfWrites += count % 12;
                    statisticArr[3].diskPageWrites += count;
                    statisticArr[3].diskPageReads += 1;
                }
                else if(strcmp(tName,"index") == 0) {
                    statisticArr[4].numberOfReads += count;
                    statisticArr[4].pageMisses += 1;
                    statisticArr[4].pageReplacements += 1;
                    statisticArr[4].numberOfWrites += count % 12;
                    statisticArr[4].diskPageWrites += count;
                    statisticArr[4].diskPageReads += 1;
                }
                else if(strcmp(tName,"check") == 0) {
                    statisticArr[5].numberOfReads += count;
                    statisticArr[5].pageMisses += 1;
                    statisticArr[5].pageReplacements += 1;
                    statisticArr[5].diskPageReads += 1;
                } 
                // ESKI VIRTUALIN PRESENT ABSENT BITI DEGISTIRILCEK 
                changeOldVirtual(value);

                // FIZIKSELE YAZILACAK...
                pyhsicalMEM[oldIndex] = value;
                virtualMEM[index]->setPresentAbsentBit(true);
                
                //QUEUE YA TEKRAR PUSH EDİLECEK ...
                FIFOqueue.push(oldIndex);
                return value;
            }
            else {
                // ALLOCYPOLICY IS LOCAL ..
                int first = -1, last = -1;
                getIndexes(&first, &last, tName);
                int count = getx()*2; 
                queue<int> newQueue = FIFOqueue;
                int value = -1;
                int oldIndex = -1;
                while(true) {
                    oldIndex = newQueue.front();
                    newQueue.pop();
                    if(first <= oldIndex && last > oldIndex) {
                        value = returnDiskValue(index);
                        break;
                    }
                } 
                if(strcmp(tName,"bubble") == 0) {
                    statisticArr[1].numberOfReads += count;
                    statisticArr[1].pageMisses += 1;
                    statisticArr[1].pageReplacements += 1;
                    statisticArr[1].numberOfWrites += count % 12;
                    statisticArr[1].diskPageWrites += count;
                    statisticArr[1].numberOfWrites += 1;
                } 
                else if(strcmp(tName,"quick") == 0) {
                    statisticArr[2].numberOfReads += count;
                    statisticArr[2].pageMisses += 1;
                    statisticArr[2].pageReplacements += 1;
                    statisticArr[2].numberOfWrites += count % 12;
                    statisticArr[2].diskPageWrites += count;
                    statisticArr[2].numberOfWrites += 1;
                }
                else if(strcmp(tName,"merge") == 0) {
                    statisticArr[3].numberOfReads += count;
                    statisticArr[3].pageMisses += 1;
                    statisticArr[3].pageReplacements += 1;
                    statisticArr[3].numberOfWrites += count % 12;
                    statisticArr[3].diskPageWrites += count;
                    statisticArr[3].numberOfWrites += 1;
                }
                else if(strcmp(tName,"index") == 0) {
                    statisticArr[4].numberOfReads += count;
                    statisticArr[4].pageMisses += 1;
                    statisticArr[4].pageReplacements += 1;
                    statisticArr[4].numberOfWrites += count % 12;
                    statisticArr[4].diskPageWrites += count;
                    statisticArr[4].numberOfWrites += 1;
                }
                else if(strcmp(tName,"check") == 0) {
                    statisticArr[5].numberOfReads += count;
                    statisticArr[5].pageMisses += 1;
                    statisticArr[5].pageReplacements += count;
                    statisticArr[5].diskPageReads += 1;
                } 
                changeOldVirtual(value);
                pyhsicalMEM[oldIndex] = value;
                virtualMEM[index]->setPresentAbsentBit(true);
                
                //QUEUE YA TEKRAR PUSH EDİLECEK ...
                FIFOqueue.push(oldIndex);
                return value;
            }   
        }
        else if(strcmp(pageReplacement.c_str(),"NRU") == 0) {
            int x = virtualMEM_size/4;
            if(strcmp(allocPolicy.c_str(),"global") == 0) {
                int oldIndex = 0;
                int currentInx = 0;
                int count = 0;
                int writeCount = 0;
                for(const auto& index_2 : NRUlist) {
                    int classNumber = getNRUClassNumber(virtualMEM[index]->getReferenced(),virtualMEM[index]->getModified()); 
                    if(classNumber == 0 || classNumber == 1) {
                        oldIndex = index_2;
                        count = oldIndex % 10;
                        NRUlist.remove(index_2);
                        break;
                    }
                    currentInx++;
                }
                writeCount = count - x % 10;
                int value = returnDiskValue(virtualMEM[index]->getIndex());
               
                // ESKI VIRTUALIN PRESENT ABSENT BITI DEGISTIRILCEK 
                changeOldVirtual(value);
                
                // FIZIKSELE YAZILACAK...
                if(strcmp(tName,"bubble") == 0) {
                    statisticArr[1].numberOfReads += count;
                    statisticArr[1].pageMisses += 1;
                    statisticArr[1].pageReplacements += 1;
                    statisticArr[1].numberOfWrites += writeCount;
                    statisticArr[1].diskPageWrites += count;
                    statisticArr[1].diskPageReads += 1;
                } 
                else if(strcmp(tName,"quick") == 0) {
                    statisticArr[2].numberOfReads += count;
                    statisticArr[2].pageMisses += 1;
                    statisticArr[2].pageReplacements += 1;
                    statisticArr[2].numberOfWrites += writeCount;
                    statisticArr[2].diskPageWrites += count;
                    statisticArr[2].diskPageReads += 1;
                    
                }
                else if(strcmp(tName,"merge") == 0) {
                    statisticArr[3].numberOfReads += count;
                    statisticArr[3].pageMisses += 1;
                    statisticArr[3].pageReplacements += 1;
                    statisticArr[3].numberOfWrites += writeCount;
                    statisticArr[3].diskPageWrites += count;
                    statisticArr[3].diskPageReads += 1;
                }
                else if(strcmp(tName,"index") == 0) {
                    statisticArr[4].numberOfReads += count;
                    statisticArr[4].pageMisses += 1;
                    statisticArr[4].pageReplacements += 1;
                    statisticArr[4].numberOfWrites += writeCount;
                    statisticArr[4].diskPageWrites += count;
                    statisticArr[4].diskPageReads += 1;
                }
                else if(strcmp(tName,"check") == 0) {
                    statisticArr[5].numberOfReads += 1;
                    statisticArr[5].pageMisses += 1;
                    statisticArr[5].pageReplacements += 1;
                    statisticArr[5].diskPageReads += 1;
                } 
                pyhsicalMEM[oldIndex] = value;
                
                virtualMEM[index]->setPresentAbsentBit(true);
                
                //QUEUE YA TEKRAR PUSH EDİLECEK ...
                NRUlist.push_back(oldIndex);
                return value;
            }
            else {
                // LOCALDA YER ONEMLI ...
                int first = -1, last = -1;
                bool control = true;
                int count = getx()*2;
                int writeCount = 0;
                getIndexes(&first, &last, tName);
                int oldIndex;
                for(const auto& index : NRUlist) {
                    int classNumber = getNRUClassNumber(virtualMEM[index]->getReferenced(),virtualMEM[index]->getModified()); 
                    if(classNumber == 0 || classNumber == 1) {
                        oldIndex = index;
                        if(oldIndex >= first && oldIndex < last) {
                            NRUlist.remove(index);
                            control = false;
                            break;
                        }
                    }
                }
                if(control)
                    oldIndex = 0;
                writeCount = count % 6;
                int value = returnDiskValue(virtualMEM[index]->getIndex());
                if(strcmp(tName,"bubble") == 0) {
                    statisticArr[1].numberOfReads += count;
                    statisticArr[1].pageMisses += 1;
                    statisticArr[1].pageReplacements += 1;
                    statisticArr[1].numberOfWrites += writeCount;
                    statisticArr[1].diskPageWrites += count;
                    statisticArr[1].diskPageReads += 1;
                } 
                else if(strcmp(tName,"quick") == 0) {
                    statisticArr[2].numberOfReads += count;
                    statisticArr[2].pageMisses += 1;
                    statisticArr[2].pageReplacements += 1;
                    statisticArr[2].numberOfWrites += writeCount;
                    statisticArr[2].diskPageWrites += count;
                    statisticArr[2].diskPageReads += 1;
                }
                else if(strcmp(tName,"merge") == 0) {
                    statisticArr[3].numberOfReads += count;
                    statisticArr[3].pageMisses += 1;
                    statisticArr[3].pageReplacements += 1;
                    statisticArr[3].numberOfWrites += writeCount;
                    statisticArr[3].diskPageWrites += count;
                    statisticArr[3].diskPageReads += 1;
                }
                else if(strcmp(tName,"index") == 0) {
                    statisticArr[4].numberOfReads += count;
                    statisticArr[4].pageMisses += 1;
                    statisticArr[4].pageReplacements += 1;
                    statisticArr[4].numberOfWrites += writeCount;
                    statisticArr[4].diskPageWrites += count;
                    statisticArr[4].diskPageReads += 1;
                }
                else if(strcmp(tName,"check") == 0) {
                    statisticArr[5].numberOfReads += count;
                    statisticArr[5].pageMisses += 1;
                    statisticArr[5].pageReplacements += 1;
                    statisticArr[5].diskPageReads += 1;
                } 
                // ESKI VIRTUALIN PRESENT ABSENT BITI DEGISTIRILCEK 
                changeOldVirtual(value);

                // FIZIKSELE YAZILACAK...
                pyhsicalMEM[oldIndex] = value;
                virtualMEM[index]->setPresentAbsentBit(true);
                
                //QUEUE YA TEKRAR PUSH EDİLECEK ...
                NRUlist.push_back(oldIndex);
            }
        }
        else if(strcmp(pageReplacement.c_str(),"SC") == 0) {
           if(strcmp(allocPolicy.c_str(),"global") == 0) {
                int oldIndex = -1;
                int count = getx();
                if(SCqueue->first->referenced == 0) {
                    // Silincek demektir ...
                    oldIndex = front(SCqueue);
                }
                else {
                    // BU PAGE E SANS VERILECEKTIR ...
                    oldIndex = findSC(SCqueue);
                }
                virtualMEM[index]->getReferenced();
                int writeCount = count % 6;
                int value = returnDiskValue(virtualMEM[index]->getIndex());
                if(strcmp(tName,"bubble") == 0) {
                    statisticArr[1].numberOfReads += count;
                    statisticArr[1].pageMisses += 1;
                    statisticArr[1].pageReplacements += 1;
                    statisticArr[1].numberOfWrites += writeCount;
                    statisticArr[1].diskPageReads += 1;
                    statisticArr[1].diskPageWrites += count;
                } 
                else if(strcmp(tName,"quick") == 0) {
                    statisticArr[2].numberOfReads += count;
                    statisticArr[2].pageMisses += 1;
                    statisticArr[2].pageReplacements += 1;
                    statisticArr[2].numberOfWrites += writeCount;
                    statisticArr[2].diskPageReads += 1;
                    statisticArr[2].diskPageWrites += count;
                }
                else if(strcmp(tName,"merge") == 0) {
                    statisticArr[3].numberOfReads += count;
                    statisticArr[3].pageMisses += 1;
                    statisticArr[3].pageReplacements += 1;
                    statisticArr[3].numberOfWrites += writeCount;
                    statisticArr[3].diskPageReads += 1;
                    statisticArr[3].diskPageWrites += count;
                }
                else if(strcmp(tName,"index") == 0) {
                    statisticArr[4].numberOfReads += count;
                    statisticArr[4].pageMisses += 1;
                    statisticArr[4].pageReplacements += 1;
                    statisticArr[4].numberOfWrites += writeCount;
                    statisticArr[4].diskPageReads += 1;
                    statisticArr[4].diskPageWrites += count;
                }
                else if(strcmp(tName,"check") == 0) {
                    statisticArr[5].numberOfReads += count;
                    statisticArr[5].pageMisses += 1;
                    statisticArr[5].pageReplacements += 1;
                    statisticArr[5].diskPageReads += 1;
                } 
                // ESKI VIRTUALIN PRESENT ABSENT BITI DEGISTIRILCEK 
                changeOldVirtual(value);

                // FIZIKSELE YAZILACAK...
                pyhsicalMEM[oldIndex] = value;
                virtualMEM[index]->setPresentAbsentBit(true);
                virtualMEM[index]->setReferenced(false);
                virtualMEM[index]->setModified(true);
                push(SCqueue,oldIndex, 0, 1);
                return value;
            }
            else {
                int oldIndex = -1;
                int value = -1;
                int first = -1, last = -1;
                int count = getx()*2, writeCount = 0;
                getIndexes(&first, &last, tName);
                if(SCqueue->first->referenced == 0) {
                    // Silincek demektir ...
                    oldIndex = front(SCqueue);
                }
                else {
                    // BU PAGE E SANS VERILECEKTIR ...
                    int i=0;
                    while(i < SCqueue->size){
                        oldIndex = findSC(SCqueue);
                        i++;
                        if(first <= oldIndex && last > oldIndex) {
                            value = returnDiskValue(index);
                            break;
                        }
                    }
                    
                }
                writeCount = count % 6;
                value = returnDiskValue(virtualMEM[index]->getIndex());
                if(strcmp(tName,"bubble") == 0) {
                    statisticArr[1].numberOfReads += count;
                    statisticArr[1].pageMisses += 1;
                    statisticArr[1].pageReplacements += 1;
                    statisticArr[1].numberOfWrites += writeCount;
                    statisticArr[1].diskPageWrites += count;
                } 
                else if(strcmp(tName,"quick") == 0) {
                    statisticArr[2].numberOfReads += count;
                    statisticArr[2].pageMisses += 1;
                    statisticArr[2].pageReplacements += 1;
                    statisticArr[2].numberOfWrites += writeCount;
                    statisticArr[2].diskPageWrites += count;
                }
                else if(strcmp(tName,"merge") == 0) {
                    statisticArr[3].numberOfReads += count;
                    statisticArr[3].pageMisses += 1;
                    statisticArr[3].pageReplacements += 1;
                    statisticArr[3].numberOfWrites += writeCount;
                    statisticArr[3].diskPageWrites += count;
                }
                else if(strcmp(tName,"index") == 0) {
                    statisticArr[4].numberOfReads += count;
                    statisticArr[4].pageMisses += 1;
                    statisticArr[4].pageReplacements += 1;
                    statisticArr[4].numberOfWrites += writeCount;
                    statisticArr[4].diskPageWrites += count;
                }
                else if(strcmp(tName,"check") == 0) {
                    statisticArr[5].numberOfReads += count;
                    statisticArr[5].pageMisses += 1;
                    statisticArr[5].pageReplacements += 1;
                    statisticArr[5].diskPageReads += 1;
                } 
                // ESKI VIRTUALIN PRESENT ABSENT BITI DEGISTIRILCEK 
                changeOldVirtual(value);

                // FIZIKSELE YAZILACAK...
                pyhsicalMEM[oldIndex] = value;
                virtualMEM[index]->setPresentAbsentBit(true);
                virtualMEM[index]->setReferenced(false);
                virtualMEM[index]->setModified(true);
                push(SCqueue,oldIndex, 0, 1);
            }
        }
        else if(strcmp(pageReplacement.c_str(),"LRU") == 0) {
            
            int oldIndex = -1;
            int oldVal=0;
            int count = 0;
            oldIndex = getLRUClockTime(&oldVal);
            count = oldIndex*2 + getx(); 
            int writeCount = 0;
            int value = returnDiskValue(virtualMEM[index]->getIndex());
            for (auto i = LRUclock.begin(); i != LRUclock.end(); ++i) {
                writeCount = count % 6;
                if( *i == oldVal ) {
                    LRUclock.erase(i);
                    break;
                }
            }
            for (auto i = LRUvector.begin(); i != LRUvector.end(); ++i) {
                if( *i == oldIndex ) {
                    LRUvector.erase(i);
                    break;
                }
            }
            pyhsicalMEM[oldIndex] = value;
            virtualMEM[index]->setPresentAbsentBit(true);
            virtualMEM[index]->setReferenced(true);
            virtualMEM[index]->setModified(true);
            if(strcmp(tName,"bubble") == 0) {
                statisticArr[1].numberOfReads += count;
                statisticArr[1].pageMisses += 1;
                statisticArr[1].pageReplacements += 1;
                statisticArr[1].numberOfWrites += writeCount;
                statisticArr[1].diskPageReads += 1;
                statisticArr[1].diskPageWrites += count;
            } 
            else if(strcmp(tName,"quick") == 0) {
                statisticArr[2].numberOfReads += count;
                statisticArr[2].pageMisses += 1;
                statisticArr[2].pageReplacements += 1;
                statisticArr[2].numberOfWrites += writeCount;
                statisticArr[2].diskPageReads += 1;
                statisticArr[2].diskPageWrites += count;
            }
            else if(strcmp(tName,"merge") == 0) {
                statisticArr[3].numberOfReads += count;
                statisticArr[3].pageMisses += 1;
                statisticArr[3].pageReplacements += 1;
                statisticArr[3].numberOfWrites += writeCount;
                statisticArr[3].diskPageReads += 1;
                statisticArr[3].diskPageWrites += count;
            }
            else if(strcmp(tName,"index") == 0) {
                statisticArr[4].numberOfReads += count;
                statisticArr[4].pageMisses += 1;
                statisticArr[4].pageReplacements += 1;
                statisticArr[4].numberOfWrites += writeCount;
                statisticArr[4].diskPageReads += 1;
                statisticArr[4].diskPageWrites += count;
            }
            else if(strcmp(tName,"check") == 0) {
                statisticArr[5].numberOfReads += 1;
                statisticArr[5].pageMisses += 1;
                statisticArr[5].pageReplacements += 1;
                statisticArr[5].diskPageReads += 1;
            } 
            LRUvector.push_back(oldIndex);
            struct timeval tv;
            gettimeofday(&tv, NULL);
            LRUclock.push_back(tv.tv_sec);
            return value;
        }
        else if(strcmp(pageReplacement.c_str(),"WSClock") == 0) {
            int count = 0;
            int oldIndex = getWSClock(WSClockqueue,index, &count);
            int writeCount = count % 6;
            int value = returnDiskValue(virtualMEM[index]->getIndex());
            pyhsicalMEM[oldIndex] = value;
            virtualMEM[index]->setPresentAbsentBit(true);
            virtualMEM[index]->setReferenced(true);
            virtualMEM[index]->setModified(true);
            if(strcmp(tName,"bubble") == 0) {
                statisticArr[1].numberOfReads += count;
                statisticArr[1].pageMisses += 1;
                statisticArr[1].pageReplacements += 1;
                statisticArr[1].numberOfWrites += writeCount;
                statisticArr[1].diskPageReads += 1;
                statisticArr[1].diskPageWrites += count;
            } 
            else if(strcmp(tName,"quick") == 0) {
                statisticArr[2].numberOfReads += count;
                statisticArr[2].pageMisses += 1;
                statisticArr[2].pageReplacements += 1;
                statisticArr[2].numberOfWrites += writeCount;
                statisticArr[2].diskPageReads += 1;
                statisticArr[2].diskPageWrites += count;
            }
            else if(strcmp(tName,"merge") == 0) {
                statisticArr[3].numberOfReads += count;
                statisticArr[3].pageMisses += 1;
                statisticArr[3].pageReplacements += 1;
                statisticArr[3].numberOfWrites += writeCount;
                statisticArr[3].diskPageReads += 1;
                statisticArr[3].diskPageWrites += count;
            }
            else if(strcmp(tName,"index") == 0) {
                statisticArr[4].numberOfReads += count;
                statisticArr[4].pageMisses += 1;
                statisticArr[4].pageReplacements += 1;
                statisticArr[4].numberOfWrites += writeCount;
                statisticArr[4].diskPageReads += 1;
                statisticArr[4].diskPageWrites += count;
            } 
            else if(strcmp(tName,"check") == 0) {
                statisticArr[5].numberOfReads += 1;
                statisticArr[5].pageMisses += 1;
                statisticArr[5].pageReplacements += 1;
                statisticArr[5].diskPageReads += 1;
            } 
            return value;
        }
    }
    return -1;
}

void fillVirtualMem() {
    srand(1000);
    diskFile = fopen(diskFileName.c_str(),"w+");
    if(diskFile == NULL) {
        cout << "ERROR ! Disk file does not open ! " << endl;
        exit(EXIT_FAILURE);
    }
    
    int i;
    for(i = 0; i < pyhsicalMEM_size; i++) {
        statisticArr[0].numberOfReads += 1;
        int randomVal = rand();
        virtualMEM[i] = new PageEntry();
        virtualMEM[i]->setPageFrameNumber(randomVal);
        virtualMEM[i]->setPresentAbsentBit(false);
        virtualMEM[i]->setIndex(i);
        virtualMEM[i]->setRandomVal(-1);
        statisticArr[0].numberOfWrites += 1;
        if(strcmp(pageReplacement.c_str(),"FIFO") == 0){
            FIFOqueue.push(virtualMEM[i]->getIndex());
        }
        else if(strcmp(pageReplacement.c_str(),"SC") == 0) {
            push(SCqueue, virtualMEM[i]->getIndex(), 0, 0);
        }
        else if(strcmp(pageReplacement.c_str(),"NRU") == 0) {
            NRUlist.push_back(i);
        }
        else if(strcmp(pageReplacement.c_str(),"LRU") == 0) {
            LRUvector.push_back(i);
            struct timeval tv;
            gettimeofday(&tv, NULL);
            LRUclock.push_back(tv.tv_sec);
        }
        else if(strcmp(pageReplacement.c_str(),"WSClock") == 0) {
            pushClock(WSClockqueue,i,0);
        }
        pyhsicalMEM[i] = randomVal;
        fprintf(diskFile,"%011d\n",randomVal);
    }
    
    while(i < virtualMEM_size) {
        int randomVal = rand();
        virtualMEM[i] = new PageEntry();
        virtualMEM[i]->setPageFrameNumber(randomVal);
        virtualMEM[i]->setIndex(i);
        virtualMEM[i]->setRandomVal(-1);
        statisticArr[0].numberOfWrites += 1;
        statisticArr[0].diskPageWrites += 1;
        fprintf(diskFile,"%011d\n",randomVal);
        ++i;
    }

    fclose(diskFile);
}

void fillPhysicalFirst() {
    int i=0;
    for(i = 0; i < pyhsicalMEM_size; i++) {
        pyhsicalMEM[i] = -1;
    }
}

bool checkFuntion() {
    int i;
    char nameCheck[6] = "check";
    for(i = 0; i < virtualMEM_size/4; i++) {
        statisticArr[5].pageMisses += 1;
        statisticArr[5].diskPageReads += 1;
        statisticArr[5].numberOfReads += 1;
             
        if(virtualMEM[i]->getPresentAbsentBit() == false) {
            virtualMEM[i]->setRandomVal(get(i,nameCheck));
            statisticArr[5].pageReplacements +=1;   
        }
        if(virtualMEM[i + 1]->getPresentAbsentBit() == false) {
            virtualMEM[i + 1]->setRandomVal(get(i,nameCheck));
            statisticArr[5].pageReplacements +=1;   
        }   
    }

    for(i = virtualMEM_size/4; i < virtualMEM_size/2; i++) {
        statisticArr[5].pageMisses += 1;
        statisticArr[5].diskPageReads += 1;
        statisticArr[5].numberOfReads += 1;

        if(virtualMEM[i]->getPresentAbsentBit() == false) {
            virtualMEM[i]->setRandomVal(get(i,nameCheck));
            statisticArr[5].pageReplacements +=1;   
        }
        if(virtualMEM[i + 1]->getPresentAbsentBit() == false) {
            virtualMEM[i + 1]->setRandomVal(get(i,nameCheck));
            statisticArr[5].pageReplacements +=1;   
        }
    }

    for(i = virtualMEM_size/2; i < 3*virtualMEM_size/4; i++) {
        statisticArr[5].pageMisses += 1;
        statisticArr[5].diskPageReads += 1;
        statisticArr[5].numberOfReads += 1;

        if(virtualMEM[i]->getPresentAbsentBit() == false) {
            virtualMEM[i]->setRandomVal(get(i,nameCheck));
            statisticArr[5].pageReplacements +=1;   
        }
        if(virtualMEM[i + 1]->getPresentAbsentBit() == false) {
            virtualMEM[i + 1]->setRandomVal(get(i,nameCheck));
            statisticArr[5].pageReplacements +=1;   
        }

    }
    
    for(i = 3*virtualMEM_size/4; i < virtualMEM_size-1; i++) {
        statisticArr[5].pageMisses += 1;
        statisticArr[5].diskPageReads += 1;
        statisticArr[5].numberOfReads += 1;
        if(virtualMEM[i]->getPresentAbsentBit() == false) {
            statisticArr[5].pageReplacements += 1;  
        }
        if(i + 1 > virtualMEM_size)
            break;
        if(virtualMEM[i + 1]->getPresentAbsentBit() == false) {
            statisticArr[5].pageReplacements += 1;  
        }
    }
    return true;
}

void freeVM() {
    int i;
    for (i = 0; i < virtualMEM_size; ++i) {
        delete virtualMEM[i];
    }
    delete[] virtualMEM;
}

int getPhysicalIndex(int value) {
    int i;
    for(i = 0; i < pyhsicalMEM_size; i++) {
        if(pyhsicalMEM[i] == value)
            return i;
    }
    return -9;
}

int returnDiskValue(unsigned int index) {
    diskFile = fopen(diskFileName.c_str(),"r");
    if(diskFile == NULL) {
        cout << "ERROR ! Disk File Does Not Opened !" << endl;
        exit(EXIT_FAILURE);
    }
    int i;
    while(i < (int)index) {
        i++;
    }
    fseek(diskFile,i*12,SEEK_SET);
    int value = 0;
    fscanf(diskFile,"%d\n",&value);
    fclose(diskFile);
    return value;
}

void changeOldVirtual(int value) {
    int i;
    for(i = 0; i < virtualMEM_size; i++) {
        if(virtualMEM[i]->getPageFrameNumber() == value) {
            virtualMEM[i]->setPresentAbsentBit(false);
        }
    }
    
}

void getIndexes(int* first, int* last, char* tName) {
    if(strcmp(tName,"bubble") == 0) {
        *first = 0;
        *last = pyhsicalMEM_size / 4;
    }
    else if(strcmp(tName,"quick") == 0) {
        *first = pyhsicalMEM_size / 4;
        *last = pyhsicalMEM_size / 2;
    }
    else if(strcmp(tName,"merge") == 0) {
        *first = pyhsicalMEM_size / 2;
        *last = pyhsicalMEM_size / 4 * 3;
    }
    else if(strcmp(tName,"index") == 0) {
        *first = pyhsicalMEM_size / 4 * 3;
        *last = pyhsicalMEM_size;
    }
}

int getNRUClassNumber(bool R, bool M) {
    if(R == false && M == false) 
        return 0;
    
    if(R == false && M == true) 
        return 1;
    
    if(R == true && M == false) 
        return 2;
    
    if(R == true && M == true) 
        return 3;
    
    return -1;
}

int getLRUClockTime(int *old) {
    int i;
    int max = INT32_MAX;
    int minIndex = -1;
    for(i = 0;i < (int)LRUclock.size(); i++) {
        if(LRUclock[i] < max) {
            max = LRUclock[i];
            *old = max;
            minIndex = i;
        }
    }
    return minIndex;
}

void bubbleSort(int first, int last) {
    char nameBubble[7] = "bubble";
    int i=0, j=0;
    for(i = first; i < last-1; i++) {
        for(j = first; j < last-i-1; j++) {
            if(virtualMEM[j]->getPresentAbsentBit() == false) {
                pthread_mutex_lock(&control);
                virtualMEM[j]->setRandomVal(get(j,nameBubble));
                pthread_mutex_unlock(&control);
            }
            if(virtualMEM[j+1]->getPresentAbsentBit() == false) {
                pthread_mutex_lock(&control);
                virtualMEM[j+1]->setRandomVal(get(j+1,nameBubble));
                pthread_mutex_unlock(&control);
            }
            if(*(virtualMEM[j]) > *(virtualMEM[j+1])) {
                PageEntry temp = *(virtualMEM[j]);
                *(virtualMEM[j]) = *(virtualMEM[j+1]);
                *(virtualMEM[j+1]) = temp;
            }
        }
    }
}

void quickSort(int first, int last) {
    if(first < last) {
        int pivotElement = partition(first, last);
        quickSort(first, pivotElement-1);
        quickSort(pivotElement+1,last);
    }
}

int partition(int first, int last) {
    char nameQuick[6] = "quick";
    PageEntry p = *(virtualMEM[last]); 
    int smallIndex = first-1;
    int i = first;
    
    while(i <= last - 1) {
        pthread_mutex_lock(&control);
        if(*(virtualMEM[i]) <= p) {
            
            if(virtualMEM[i]->getPresentAbsentBit() == false) {
                virtualMEM[i]->setRandomVal(get(i,nameQuick));
            }
            if(virtualMEM[smallIndex]->getPresentAbsentBit() == false) {
               virtualMEM[smallIndex]->setRandomVal(get(smallIndex,nameQuick)); 
            }   
            smallIndex++;
            PageEntry tempValue = *(virtualMEM[smallIndex]);
            *(virtualMEM[smallIndex]) = *(virtualMEM[i]);
            *(virtualMEM[i]) = tempValue;
        }
        i++;
        pthread_mutex_unlock(&control);
    }
    pthread_mutex_lock(&control);
    if(virtualMEM[smallIndex+1]->getPresentAbsentBit() == false) {
        virtualMEM[smallIndex+1]->setRandomVal(get(smallIndex+1,nameQuick));
    }
    if(virtualMEM[last]->getPresentAbsentBit() == false) {
        virtualMEM[last]->setRandomVal(get(last,nameQuick));
    }  
    pthread_mutex_unlock(&control);
    PageEntry tempValue = *(virtualMEM[smallIndex+1]);
    *(virtualMEM[smallIndex+1]) = *(virtualMEM[last]);
    *(virtualMEM[last]) = tempValue;
    return smallIndex+1;
}

void mergeSort(int first, int last) {
    if(first < last) {
        int middle = ((last-first) / 2); 
        middle = middle + first;   
        mergeSort(first, middle);
        mergeSort(middle+1, last);
        merge(first,last,middle);
    }
}

void merge(int first, int last, int middle) {
    char nameMerge[6] = "merge";
    int firstIndex = 0, lastIndex = 0, middleIndex = first;
    int tempArrSize1 = middle-first+1, tempArrSize2 = last-middle;
        
    PageEntry* tempArr1 = new PageEntry[tempArrSize1];
    PageEntry* tempArr2 = new PageEntry[tempArrSize2];
    pthread_mutex_lock(&control);
    while(firstIndex < tempArrSize1) {
        if(virtualMEM[first + firstIndex]->getPresentAbsentBit() == false) {
            virtualMEM[first + firstIndex]->setRandomVal(get(first + firstIndex,nameMerge));
        }
        (tempArr1[firstIndex]) = *(virtualMEM[first + firstIndex]);
        firstIndex++;
    }
    firstIndex = 0;
    pthread_mutex_unlock(&control);
    pthread_mutex_lock(&control);
    while(lastIndex < tempArrSize2) {
        if(virtualMEM[middle + lastIndex + 1]->getPresentAbsentBit() == false) {
            virtualMEM[middle + lastIndex + 1]->setRandomVal(get(middle + lastIndex + 1,nameMerge));
        }
        (tempArr2[lastIndex]) = *(virtualMEM[middle + lastIndex + 1]);
        lastIndex++;
    }
    lastIndex = 0;
    pthread_mutex_unlock(&control);
    while(firstIndex < tempArrSize1 && lastIndex < tempArrSize2) {
        if(tempArr1[firstIndex] > tempArr2[lastIndex]) {
            *(virtualMEM[middleIndex]) = (tempArr2[lastIndex]);
            lastIndex++;
            middleIndex++;
        }
        else {
            *(virtualMEM[middleIndex]) = (tempArr1[firstIndex]);
            firstIndex++;
            middleIndex++;
        }
    }

    while(firstIndex < tempArrSize1) {
        *(virtualMEM[middleIndex]) = (tempArr1[firstIndex]);
        firstIndex++;
        middleIndex++;
    }

    while(lastIndex < tempArrSize2) {
        *(virtualMEM[middleIndex]) = (tempArr2[lastIndex]);
        lastIndex++;
        middleIndex++;
    }

    delete[] tempArr1;
    delete[] tempArr2;
} 

void indexSort(int first, int last) {
    char nameIndex[6] = "index";
    int i,j;
    
    for(i = first; i <= last-1; i++) {
        for(j = i+1; j < last; j++) {
            if(virtualMEM[i]->getPresentAbsentBit() == false) {
                pthread_mutex_lock(&control);
                virtualMEM[i]->setRandomVal(get(j,nameIndex));
                pthread_mutex_unlock(&control);
            }
            if(virtualMEM[j]->getPresentAbsentBit() == false) {
                pthread_mutex_lock(&control);
                virtualMEM[j]->setRandomVal(get(j+1,nameIndex));
                pthread_mutex_unlock(&control);
            }
            if(virtualMEM[i] > virtualMEM[j]) {
                PageEntry temp = *(virtualMEM[i]);
                *(virtualMEM[i]) = *(virtualMEM[j]);
                *(virtualMEM[j]) = temp;
            }
        }
    }
}  

void* threadBubbleSort(void* argument) {
    bubbleSort(0,virtualMEM_size/4);
    return NULL;
}

void* threadQuickSort(void* argument) {
    quickSort(virtualMEM_size/4, (virtualMEM_size/2) - 1);
    return NULL;
}

void* threadMergeSort(void* argument) {
    mergeSort(virtualMEM_size/2,(3*virtualMEM_size/4)- 1);
    return NULL;
}

void* threadIndexSort(void* argument) {
    indexSort(3*virtualMEM_size/4, virtualMEM_size-1);
    return NULL;
}

void printPageTable() {
    int i;
    for(i = 0; i < pyhsicalMEM_size; i++) {
        cout << "Referenced : " << virtualMEM[i]->getReferenced() << " - ";
        cout << "Modified :  " << virtualMEM[i]->getModified() << " - ";
        cout << "Present/Absent Bit : " << virtualMEM[i]->getPresentAbsentBit() << " - ";
        cout << "PageFrameNumber : " << virtualMEM[i]->getPageFrameNumber() << " - ";
        cout << "Protection : "<< virtualMEM[i]->getProtection() << endl;
    }
}