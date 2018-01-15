

#ifndef CACHE_H
#define CACHE_H

#include <vector>
#include <list>
#include <unordered_map>
#include <fstream>
#include <string>
#include <set>
#include <iostream>

using namespace std;

class Cache {
public:
    Cache(int _total_blocks, int _ram_blocks, const char *_swap_file);
    virtual ~Cache();

    int writeCacheBlock(int blockID, const string& buffer);
    int readCacheBlock(int blockID, string& buffer);
    int freeCacheBlock(int blockID);
    
    void printLRUList() const;
private:
    void init_store(int total_blocks, int ram_blocks);
    void cleanup_store();

    //Hilfsfunktionen
    int getFreeBlockID() const;
    bool checkIfBlockIDIsFree(int id) const;
    int getFreePosition() const;
    bool checkIfPositionIsFree(int pos) const;
    void updateLRUList(int blockID);
    void updateMaps(int blockID, int position);
    
    string readBlock(int position);
    bool writeBlock(int write_position, string buffer);
    
    int getPosition(int blockID);

    int total_blocks;
    int ram_blocks;
    int swap_blocks;
    const char *swap_file;

    list<int> lru_list;
    unordered_map<int, int> block_map;
    unordered_map<int, int> pos_map;
    string **cache_data;
    
};

#endif /* CACHE_H */

