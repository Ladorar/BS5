/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.cpp
 * Author: Nicholas
 *
 * Created on 15. Januar 2018, 01:51
 */

#include <cstdlib>
#include "Cache.h"


using namespace std;


int main(int argc, char** argv) {
    Cache c(10, 5, "swap.txt");
    string buf;
    c.writeCacheBlock(2, "Hallo welt");
    c.writeCacheBlock(-1, "Hallo welt");
    c.writeCacheBlock(-1, "Hallo welt");
    c.writeCacheBlock(-1, "Hallo welt");
    c.writeCacheBlock(-1, "Hallo welt");
    c.writeCacheBlock(-1, "Hallo welt");
    c.readCacheBlock(2, buf);
    c.printLRUList();
    c.freeCacheBlock(5);
    c.printLRUList();
    c.writeCacheBlock(5, "HIHO");
    c.readCacheBlock(5, buf);
    c.printLRUList();
    cout << buf << endl;
    return 0;
}

