/*
Sawyer McLane
CS3800
Homework 2
April 6th 2016
*/
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <vector>
#include <string>
#include <math.h>

using namespace std;

const int AVAILABLE_FRAMES = 512, NUM_PROGRAMS = 10;

//Global Vars
int pageOffset = 0;
long pageFaults = 0;



struct pageTable
{
	public:
		int m_programNumber;
		int m_programSize;
		int m_pagesNeeded;
		int m_lowerPageNum;
		int m_upperPageNum;
		int m_firstRAMindex;

		//metdata
		int m_fifoIndex;
	pageTable(int programNumber, int programSize, int pageSize)
	{
		m_programNumber = programNumber;
		m_programSize = programSize;
		m_pagesNeeded = m_programSize / pageSize;
		m_lowerPageNum = pageOffset + 1;
		pageOffset += m_pagesNeeded;
		m_upperPageNum = pageOffset;
		m_fifoIndex = m_firstRAMindex; //init fifoindex to first entry in memory space
	}
	pageTable() {}
};

int main(int argc, char** argv)
{
	/*
	arg0: Program Name
	arg1: programlist file
	arg2: programtrace file
	arg3: page size (positive power of 2)
	arg4: page replacement algorithm (clock, fifo or lru)
	arg5: demand/prepaging (0 = demand,1 = prepaging)
	*/
	bool prepaging = atoi(argv[5]);
	if (argc != 6) {
		cerr << "Invalid number of parameters.";
		return 0;
	}
	string algorithm = argv[4];
	if (algorithm != "clock" && algorithm != "fifo" && algorithm != "lru")
	{
		cerr << "Invalid algorithm. Please choose clock, fifo, or lru.";
		return 0;
	}
	int programSizes[NUM_PROGRAMS];
	ifstream in;
	in.open(argv[1]);
	//make programSize array
	for(int i = 0; i < NUM_PROGRAMS; i++) {
		int temp;
		in >> temp >> temp;
		programSizes[i] = temp;
	}
	in.close();
	pageTable programlist[NUM_PROGRAMS];
	int pageSize = atoi(argv[3]);
	for(int i = 0; i < NUM_PROGRAMS; i++)
	{
		programlist[i] = pageTable(i, programSizes[i], pageSize);
	}
	/*
	for(int i = 0; i < NUM_PROGRAMS; i++)
	{
		cout << programlist[i].m_programNumber << endl;
		cout << programlist[i].m_pagesNeeded << endl;
		cout << programlist[i].m_programSize << endl;
		cout << programlist[i].m_lowerPageNum << endl;
		cout << programlist[i].m_upperPageNum << endl;
		cout << endl;
	}
	*/
	int RAMSize = AVAILABLE_FRAMES / pageSize;
	int RAM[RAMSize];
	int lruArray[RAMSize];
	int lruTimestamp = 0;
	int clockArray[RAMSize];
	int clockIndex;
	int pageAlloc = RAMSize / NUM_PROGRAMS; //how many page locations each program gets split evenly
	for(int i = 0; i < RAMSize; i++) 
	{ 
		RAM[i] = 0;
	}
	int ramIndex = 0;
	for(int i = 0; i < NUM_PROGRAMS; i++)
	{
		for(int j = programlist[i].m_lowerPageNum; j < programlist[i].m_lowerPageNum + pageAlloc; j++)
		{
			//move the first $pageAlloc pages into RAM
			if (j == programlist[i].m_lowerPageNum) //if this is the first entry, set the fifo index
			{
				programlist[i].m_fifoIndex = ramIndex;
				programlist[i].m_firstRAMindex = ramIndex;
			}
			RAM[ramIndex] = j;
			lruArray[ramIndex] = 0;
			clockArray[ramIndex] = 0;
			ramIndex++;
		}
	}
	/*
	for(int i = 0; i < RAMSize; i++){
		cout << RAM[i] << endl;
	}
	*/
	in.open(argv[2]);
	int programid, pagerequest;
	while(in >> programid)
	{
		bool found = false;
		in >> pagerequest;
		pagerequest = floor(pagerequest / pageSize); //convert requested unit to requested page
		int lowerbound = pageAlloc * programid;
		int uppperbound = lowerbound + pageAlloc;
		for(int i = lowerbound; i < uppperbound; i++){
			//looks through programs memory space for requested page
			if (RAM[i] == pagerequest)
			{
				found = true;
				lruArray[i] = 0; //reset lru because it has been recently used
				clockArray[i] = 1; //set useBit to 1 because recently used.
			}
			else
			{
				lruArray[i]++; //increment the timestamp if the value isn't found
			}
		}
		if(!found)
		{
			pageFaults++;
			if (algorithm == "fifo")
			{
				RAM[programlist[programid].m_fifoIndex] = pagerequest;
				programlist[programid].m_fifoIndex++;
				if(programlist[programid].m_fifoIndex > uppperbound)
				{
					programlist[programid].m_fifoIndex = lowerbound;
				}
				if (prepaging)
				{
					RAM[programlist[programid].m_fifoIndex] = pagerequest;
					programlist[programid].m_fifoIndex++;
					if(programlist[programid].m_fifoIndex > uppperbound)
					{
						programlist[programid].m_fifoIndex = lowerbound;
					}
				}
			}
			else if (algorithm == "lru")
			{
				int leastRecentlyUsed = lruArray[lowerbound];
				int lruIndex = lowerbound;
				for(int i = lowerbound; i < uppperbound; i++)
				{
					if (lruArray[i] > leastRecentlyUsed)
					{
						leastRecentlyUsed = lruArray[i];
						lruIndex = i;
					}
				}
				RAM[lruIndex] = pagerequest;
				lruArray[lruIndex] = 0;
				if (prepaging) {
					RAM[lruIndex+1] = pagerequest + 1;
					lruArray[lruIndex+1] = 0;
				}
			}
			else if (algorithm == "clock")
			{
				clockIndex = lowerbound;
				while (clockArray[clockIndex] == 1)
				{
					clockArray[clockIndex] = 0;
					clockIndex++;
					if (clockIndex > uppperbound)
					{
						clockIndex = lowerbound;
					}
				}
				RAM[clockIndex] = pagerequest;
				clockArray[clockIndex] = 1;
				if (prepaging)
				{
					RAM[clockIndex + 1] = pagerequest + 1;
					clockArray[clockIndex + 1] = 1;
				}
			}
		}
	}
	cout << "Page Size: " << pageSize << endl;
	cout << "Replacement Policy: " << algorithm << endl;
	if (prepaging) {cout << "Prepaging" << endl;} 
	else { cout << "Demand Paging" << endl;}
	cout << "Page Faults: " << pageFaults;
	return 0;
}