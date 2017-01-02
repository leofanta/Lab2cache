//
//  main.cpp
//  Lab2cache
//
//  Created by Jen Liu on 11/26/16.
//  Copyright © 2016 Jen Liu. All rights reserved.
//

/*
 Cache Simulator
 Level one L1 and level two L2 cache parameters are read from file (block size, line per set and set per cache).
 The 32 bit address is divided into tag bits (t), set index bits (s) and block offset bits (b)
 s = log2(#sets)   b = log2(block size)  t=32-s-b
 */
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <iomanip>
#include <stdlib.h>
#include <cmath>
#include <bitset>

using namespace std;
//access state:
#define NA 0 // no action
#define RH 1 // read hit
#define RM 2 // read miss
#define WH 3 // Write hit
#define WM 4 // write miss



struct config{
    int L1blocksize;
    int L1setsize;
    int L1size;
    int L2blocksize;
    int L2setsize;
    int L2size;
};

// you can define the cache class here, or design your own data structure for L1 and L2 cache
 class cache {
     
 public:
     
     cache(unsigned int blockSize, unsigned int setSize, unsigned int cacheSize)
     {
         //divide to tag bits and index bits.
         this->blockLength = log2(blockSize);
         this->indexLength = 10 + log2(cacheSize) - log2(setSize) - log2(blockSize);
         this->tagLength = 32 - blockLength - indexLength;
         this->setSize = setSize;
         
         //initialize the sets
         sets.resize(int(pow(2.0, indexLength)));
         for (int i = 0; i < sets.size(); i++) {
             sets[i].vblock.resize(setSize);
             sets[i].evictCounter = 0;
             sets[i].emptyWay = 0;
         }
     }
     

     struct block{
         unsigned long tag;
         bool valid;
     };

     struct set{
         vector<block> vblock;
         unsigned int emptyWay;
         unsigned int evictCounter;
     };
     
     /*BELOW ARE READ AND WRITE RELETED OPERATION FUNCTIONS
      */
     

     bool readAccess(bitset<32> addr){
         unsigned long tagNo = addr.to_ulong();
         tagNo = tagNo >> (blockLength + indexLength);
         
         unsigned long number = findSet(addr);
         set *present;
         present = &sets[number];
         //find the block
         for (unsigned long i = 0; i < present->vblock.size(); i ++ ) {
             if (present->vblock[i].valid == true) {
                 if (present->vblock[i].tag == tagNo) {
                     return true;
                 }
             }
         }
         return false;
     }
     

     void writeAccess(bitset<32> addr){
         
         unsigned long tagNo = addr.to_ulong();
         tagNo = tagNo >> (blockLength + indexLength);

         unsigned long number = findSet(addr);
         set *present;
         present = &(sets[number]);
         //update/evict block
         if (present->emptyWay == setSize) {
             present->vblock[present->evictCounter].tag = tagNo;
             present->evictCounter ++;
             if (present->evictCounter == setSize) {
                 present->evictCounter = 0;
             }
         }else{
             present->vblock[present->emptyWay].tag = tagNo;
             present->vblock[present->emptyWay].valid = true;
             present->emptyWay ++;
         }
     }
     
     //find the set and return it
     unsigned long findSet(bitset<32> addr){
         unsigned long setNo = addr.to_ulong();
         setNo = setNo % int(pow(2.0, blockLength+indexLength));
         setNo = setNo >> (blockLength);
         
         return setNo;
     }
     
 private:
     unsigned int setSize;
     unsigned int blockLength;
     unsigned int indexLength;
     unsigned int tagLength;
     vector<set> sets;
 
 };



int main(int argc, char* argv[]){
    
    config cacheconfig; //read from config.txt
    ifstream cache_params;
    string dummyLine;
    cache_params.open(argv[1]);//should be argv[1]
    while(!cache_params.eof())  // read config file
    {
        cache_params>>dummyLine;
        cache_params>>cacheconfig.L1blocksize;
        cache_params>>cacheconfig.L1setsize;
        cache_params>>cacheconfig.L1size;
        cache_params>>dummyLine;
        cache_params>>cacheconfig.L2blocksize;
        cache_params>>cacheconfig.L2setsize;
        cache_params>>cacheconfig.L2size;
    }
    
    
    
    // Implement by you:
    // initialize the hirearch cache system with those configs
    // probably you may define a Cache class for L1 and L2, or any data structure you like
    cache L1(cacheconfig.L1blocksize, cacheconfig.L1setsize, cacheconfig.L1size);
    cache L2(cacheconfig.L2blocksize, cacheconfig.L2setsize, cacheconfig.L2size);
    
    
    int L1AcceState = 0; // L1 access state variable, can be one of NA, RH, RM, WH, WM;
    int L2AcceState = 0; // L2 access state variable, can be one of NA, RH, RM, WH, WM;
    
    
    ifstream traces;
    ofstream tracesout;
    string outname;
    outname = string(argv[2]) + ".out";//should be argv[2]
    
    traces.open(argv[2]);//should be argv[2]
    tracesout.open(outname.c_str());
    
    string line;
    string accesstype;  // the Read/Write access type from the memory trace;
    string xaddr;       // the address from the memory trace store in hex;
    unsigned int addr;  // the address from the memory trace store in unsigned int;
    bitset<32> accessaddr; // the address from the memory trace store in the bitset;
    
    if (traces.is_open()&&tracesout.is_open()){
        while (getline (traces,line)){   // read mem access file and access Cache
            
            istringstream iss(line);
            if (!(iss >> accesstype >> xaddr)) {break;}
            stringstream saddr(xaddr);
            saddr >> std::hex >> addr;
            accessaddr = bitset<32> (addr);
            
            
            // access the L1 and L2 Cache according to the trace;
            if (accesstype.compare("R")==0)
                
            {
                //Implement by you:

                if (L1.readAccess(addr)) {// L1 read hit
                    L1AcceState = RH;
                    L2AcceState = NA;

                }else if (L2.readAccess(addr)) {// L1 read miss
                    L1AcceState = RM;// L2 read hit
                    L2AcceState = RH;
                    L1.writeAccess(addr);
                    
                    
                }else{// L2 read miss
                    L1AcceState = RM;
                    L2AcceState = RM;
                    L2.writeAccess(addr);
                    L1.writeAccess(addr);
                }
            
                
                
            }
            else
            {
                //Implement by you:
                
                if (L1.readAccess(addr)) {// L1 write hit
                    L1AcceState = WH;
                    L2AcceState = NA;
                    
                } else if(L2.readAccess(addr)){// L1 write miss
                    L1AcceState = WM;// L2 write hit
                    L2AcceState = WH;
                    
                }else{// L2 write miss
                    L1AcceState = WM;
                    L2AcceState = WM;
                }
                
                
                
            }
            
            
            
            tracesout<< L1AcceState << " " << L2AcceState << endl;  // Output hit/miss results for L1 and L2 to the output file;
            
            
        }
        traces.close();
        tracesout.close();
    }
    else cout<< "Unable to open trace or traceout file ";
    
    
    
    
    
    
    
    return 0;
}
