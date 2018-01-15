
#include "Cache.h"

Cache::Cache(int _total_blocks, int _ram_blocks, const char *_swap_file) {
    swap_file = _swap_file;
    total_blocks = _total_blocks;
    ram_blocks = _ram_blocks;
    swap_blocks = _total_blocks - _ram_blocks;
    init_store(total_blocks, ram_blocks);
}

Cache::~Cache() {
    cleanup_store();
}

void Cache::init_store(int total_blocks, int ram_blocks) {
    // Wir erstellen unser Daten-Array
    cache_data = new string *[ram_blocks];
    // Erstellen nur die Datei und füllen Sie mit 0
    ofstream afile(swap_file);
    string init_string = "";
    while (init_string.size() < 1024) {
        init_string.append("0");
    }
    int counter = 0;
    while (afile.is_open() && afile.good() && (counter < swap_blocks)) {
        afile.write(init_string.c_str(), init_string.size());
        counter++;
    }
    afile.close();
}

void Cache::cleanup_store() {
    //remove(swap_file);
    lru_list.clear();
}

int Cache::writeCacheBlock(int blockID, const string& buffer) {
    if (blockID == -1) {
        //Es wird ein neuer Block angefordert
        blockID = getFreeBlockID();
        if (blockID == -1) {
            cout << "Kein freier Block mehr!" << endl;
            return -1;
        }
    } else if (!checkIfBlockIDIsFree(blockID)) {
        cout << "Dieser Block ist nicht frei!" << endl;
        return -1;
    }
    int position = getFreePosition();
    if (position == -1) {
        cout << "Fehler beim Berechnen einer Freien Position!" << endl;
        return -1;
    }
    if (lru_list.size() < ram_blocks && position < ram_blocks) {
        //Es ist noch Platz im RAM, Element wird einfach geschrieben
        cache_data[position] = new string(buffer);
        updateLRUList(blockID);
        updateMaps(blockID, position);
        return 0;
    } else if (lru_list.size() == ram_blocks && position >= ram_blocks) {
        //Es ist kein Platz mehr im RAM, es muss etwas ausgelagert werden
        int block_to_swap_id = lru_list.back();
        int block_to_swap_pos = getPosition(block_to_swap_id);
        //Wir müssen den Block mit der ID block_to_swap_id und der Position block_to_swap_pos auf die Festplatte auslagern
        writeBlock(position, *(cache_data[block_to_swap_pos]));
        lru_list.remove(block_to_swap_id);
        // Der Block steht jetzt in der Auslagerungsdatei
        updateMaps(block_to_swap_id, position);
        // Jetzt können wir die alte Position benutzen
        delete cache_data[block_to_swap_pos];
        cache_data[block_to_swap_pos] = new string(buffer);
        updateLRUList(blockID);
        updateMaps(blockID, block_to_swap_pos);
        return 0;
    } else {
        cout << "Die Liste ist zu Groß geworden -> Fehler" << endl;
        return -1;
    }
}

int Cache::readCacheBlock(int blockID, string& buffer) {
    if (checkIfBlockIDIsFree(blockID)) {
        cout << "Kann nicht von freiem Block lesen" << endl;
        return -1;
    }
    int position = getPosition(blockID);
    if (position == -1) {
        cout << "Fehler beim finden der Position" << endl;
        return -1;
    } else if (position < ram_blocks) {
        // Der block befindet sich im RAM
        buffer = *cache_data[position];
        updateLRUList(blockID);
        return 0;
    } else if (position >= ram_blocks) {
        // Der Block befindet sich in der Datei
        int block_to_swap_id = lru_list.back();
        int block_to_swap_pos = getPosition(block_to_swap_id);
        buffer = readBlock(position);
        writeBlock(position, *cache_data[block_to_swap_pos]);
        delete cache_data[block_to_swap_pos];
        cache_data[block_to_swap_pos] = new string(buffer);
        lru_list.remove(block_to_swap_id);
        updateLRUList(blockID);
        updateMaps(block_to_swap_id, position);
        updateMaps(blockID, block_to_swap_pos);
    }
}

int Cache::freeCacheBlock(int blockID) {
    if (checkIfBlockIDIsFree(blockID)) {
        cout << "Block nicht vorhanden kann nicht freigeben werden!" << endl;
        return -1;
    }
    int position = getPosition(blockID);
    if (position == -1) {
        cout << "Fehler beim Berechnen der Positon!" << endl;
        return -1;
    } else  if (position < ram_blocks) {
        // Block aus dem Ram freigeben
        delete cache_data[position];
        lru_list.remove(blockID);
        block_map.erase(blockID);
        pos_map.erase(position);
    } else if (position >= ram_blocks) {
        // Der Block ist in der Datei und kann einfach überschrieben werden
        writeBlock(position, "");
        block_map.erase(blockID);
        pos_map.erase(position);
    }
}

int Cache::getFreeBlockID() const {
    for (int i = 0; i < total_blocks; i++) {
        if (checkIfBlockIDIsFree(i))
            return i;
    }
    return -1;
}

bool Cache::checkIfBlockIDIsFree(int id) const {
    if (block_map.find(id) != block_map.end())
        return false;
    return true;
}

int Cache::getFreePosition() const {
    for (int i = 0; i < total_blocks; i++) {
        if (checkIfPositionIsFree(i))
            return i;
    }
    return -1;
}

bool Cache::checkIfPositionIsFree(int pos) const {
    if (pos_map.find(pos) != pos_map.end())
        return false;
    return true;
}

void Cache::updateLRUList(int blockID) {
    lru_list.remove(blockID);
    lru_list.push_front(blockID);
}

void Cache::updateMaps(int blockID, int position) {
    block_map[blockID] = position;
    pos_map[position] = blockID;
}

void Cache::printLRUList() const {
    cout << "[ ";
    for (auto& el : lru_list) {
        cout << el << " ";
    }
    cout << "]" << endl;
}

int Cache::getPosition(int blockID) {
    return block_map[blockID];
}

string Cache::readBlock(int position) {
    const int read_size = 1024;
    ifstream afile(swap_file, ios_base::in | ios_base::out);
    if (afile.is_open()) {
        afile.seekg((position - ram_blocks) * 1024);
        cout << afile.tellg() << endl;
        char buf[1024];
        afile.read(buf, 1024);
        string s(buf);
        s.append("0");
        cout << s.size() << " Size, " << afile.gcount() << " Bytes gelesen" << endl;
        return s;
    }
    string s = "";
    return s;
}

bool Cache::writeBlock(int write_position, string buffer) {
    while (buffer.size() < 1024)
        buffer.append("0");
    const char* data = buffer.c_str();
    ofstream afile(swap_file, ios_base::in | ios_base::out);
    if (afile.is_open()) {
        afile.seekp((write_position - ram_blocks) * 1024);
        long before = afile.tellp();
        //cout << before << endl;
        afile.write(data, 1023);
        afile.write("\0", 1);
        long after = afile.tellp();
        //cout << after << endl;
        cout << (after - before) << " Bytes ausgelagert" << endl;
        afile.close();
        return true;
    }
    return false;
}

