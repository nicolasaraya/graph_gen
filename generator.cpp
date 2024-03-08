#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <set>
#include <algorithm>
#include <random>
#include <omp.h>
#include <threads.h>
#include <mutex>
#include <unordered_map>
#include <stdint.h>

#include "Utils.hpp"

#define DBITS32
//#define WEIGHTED

using namespace std;

float porcentaje = 0.8;
unsigned edges = 10000000;
unsigned edgesBicl = edges * porcentaje; 
unsigned SxC_Biclique = 2500; 
unsigned size_s = sqrt(SxC_Biclique);
unsigned graphNodes = edges + edgesBicl;

float desvest = size_s * 0.2;

unsigned minWeight = 1; 
unsigned maxWeight = 10;

string name = "g_" + to_string(edges) + "_" + to_string(int(porcentaje*100)) + "_" + to_string(SxC_Biclique);

typedef struct {
    set<uint32_t> S;
    vector<pair<uint32_t,uint32_t>> C; 
} Biclique;

typedef vector<pair<uint32_t, uint32_t>> C_values;

struct CompactBicliqueWeighted {
    set<uint32_t> weights_values;
    vector<C_values> c_bicliques; 
    vector<pair<uint32_t, vector<uint32_t>>> linked_s; //S_value to C_values index   
};


void clear()
{
	system("@cls||clear");
}

vector<Biclique>* generateBicliques()
{
    std::cout << "generando bicliques" << endl;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<double> distribution(size_s, desvest);

    unordered_map<uint32_t, uint32_t> aux;
    
    auto bicliques = new vector<Biclique>();

    uint64_t countEdges = 0;
    uint64_t countBicliques = 0;
    int32_t size = 10;

    while (countEdges < edgesBicl) {
        do {
            size = static_cast<int>(distribution(gen));
        } while (size < 3); 
        

        Biclique b; 
        set<uint32_t>* S = &(b.S);
        vector<pair<uint32_t,uint32_t>>* C = &(b.C); 

        while (S->size() < size) {
            uint32_t index = rand()%graphNodes + 1; 
            S->insert(index);
        }

        for (auto i : *S) {
            uint32_t adj = i+1; 
            uint32_t weight = rand()%(maxWeight-minWeight) + minWeight;
            while(aux[adj] != 0) {
                adj++;
            }
            aux[adj] = 1; 
            C->push_back(make_pair(adj, weight));

        }
        //cout << "done C" << endl;
        

        bicliques->push_back(b);
        countEdges += b.C.size() * b.S.size();
        countBicliques++;
    }
    
    std::cout << "Edges in bicliques: " << countEdges << std::endl;
    std::cout << "Num bicliques: " << countBicliques << std::endl;


    //sleep(40);
    return bicliques; 

    

}

void generateGraph(vector<Biclique>* bicliques)
{
    map<uint32_t, set<pair<uint32_t,uint32_t>>> graph;
    map<uint32_t, set<pair<uint32_t,uint32_t>>> full_graph;

    uint64_t countEdges = 0;

    cout << "insert biclique in graph" << endl;
    for (auto i : *bicliques) {
        for (auto j : i.S) {
            for (auto k : i.C) {
                full_graph[j].insert(make_pair(k.first, k.second));
            }
        }
        countEdges += i.S.size() * i.C.size();
        //cout << "current edges: " << countEdges << endl; 
    }

    cout << "filling graph" << endl;

    while (countEdges < edges) {
        uint32_t index = rand()%graphNodes + 1;
        

        size_t cant = rand()%2 + 1; 
        countEdges += cant;

        while (cant > 0) {
            uint32_t adj = rand()%graphNodes + 1;
            uint32_t weight = rand()%(maxWeight-minWeight) + minWeight;

            set<pair<uint32_t,uint32_t>>::iterator it;
            it = (full_graph[index]).find(pair<uint32_t, uint32_t>(adj,weight));

            if (it != full_graph[index].end()) {
                continue; 
            } else {
                full_graph[index].insert(make_pair(adj,weight));
                graph[index].insert(make_pair(adj,weight));
                cant--;
            }
        }
         
    }

    string path = name + ".txt";

    ofstream file;
    file.open(path, ofstream::out | ofstream::trunc);
    file << full_graph.size() << std::endl;

    for (auto i : full_graph) {
        for (auto j : (i.second)) {
            file << i.first << " " << j.first ;
            #ifdef WEIGHTED
            file << " " << j.second;
            #endif
            file << std::endl;
        }
    }
    file.close();

    path = name + "_compressed.txt";

    file.open(path, ofstream::out | ofstream::trunc);

    file << graph.size() << std::endl;

    for (auto i : graph) {
        for (auto j : (i.second)) {
            file << i.first << " " << j.first ;
            #ifdef WEIGHTED
            file << " " << j.second;
            #endif
            file << std::endl;
        }
    }
    file.close();
    std::cout << "nodes: " << full_graph.size();
}

CompactBicliqueWeighted* generateCompactStructure(vector<Biclique>* bicliques)
{   
    auto compBicl = new CompactBicliqueWeighted();

    for (auto biclique = bicliques->begin(); biclique != bicliques->end(); biclique++) {
        vector<uint32_t> S; 
        vector<pair<uint32_t,uint32_t>> C; 

        uint32_t S_size = S.size();
        uint32_t C_size = C.size(); 

        C_values bicl_C_values;

        for (auto it : biclique->S) {
            S.push_back(it);
        }
        for (auto it : biclique->C) {
            C.push_back(make_pair(it.first, it.second));
        }

        sort(S.begin(), S.end());
        sort(C.begin(), C.end(), [](auto& a, auto& b){return a.first < b.first;});

        for(auto it = C.begin(); it != C.end(); it++){
            compBicl->weights_values.insert((*it).second);
            bicl_C_values.push_back(make_pair((*it).first, (*it).second)); 
        }
        

        compBicl->c_bicliques.push_back(bicl_C_values);

        for(auto it = S.begin(); it != S.end(); it++){
            uint32_t id = (*it);

            vector<pair<uint32_t, vector<uint32_t>>>::iterator f = std::find_if( compBicl->linked_s.begin(), compBicl->linked_s.end(),
                [id](const std::pair<uint32_t, vector<uint32_t>>& element){ return element.first == id;} );

            if (f == compBicl->linked_s.end()) {
                pair<uint32_t, vector<uint32_t>> p; 
                p.first = id; 
                p.second.push_back(compBicl->c_bicliques.size()-1);
                compBicl->linked_s.push_back(p);
            } else {
                (*f).second.push_back(compBicl->c_bicliques.size()-1);
            }
        }
    }
    return compBicl; 
}

void writeCompactStructure(CompactBicliqueWeighted* compBicl)
{   
    ofstream file;
    string new_path = name + "_compact_biclique.txt";
    file.open(new_path, fstream::trunc);
    assert(file.is_open());


    std::sort(compBicl->linked_s.begin(), compBicl->linked_s.end(), [](pair<uint32_t, vector<uint32_t>> a, pair<uint32_t, vector<uint32_t>> b){ return a.first < b.first; });

    /*file << "W: ";
    for(auto i : compBicl->weights_values) {
        file << i << " ";
    }
    file << endl;*/

    //file << "Bicliques: " << endl;
    for (size_t i = 0; i < compBicl->c_bicliques.size(); i++) {
        file <<  i << ": ";
        for(auto j : compBicl->c_bicliques.at(i)) {
            file << "(" << j.first << "," << j.second << ")" << " ";
        } 
        file << endl;
    }
    file << "S: " << endl;
    for (auto i : compBicl->linked_s) {
        file << i.first << ": ";
        for (auto j : i.second) {
            file << j << " ";
        }
        if (i != *(compBicl->linked_s.end())) file << endl; 
    }
    
    file.close();
    return; 
}


void writeCompactStructureBin(CompactBicliqueWeighted* compBicl)
{   
    typedef bool binVar; 
    bool on = true;
    bool off = false;

    string S_path = name + "_S.bin";
    string SS_path = name + "_SS.bin";
    string C_path = name + "_C.bin";
    string CC_path = name + "_CC.bin";

    ofstream S, SS, C, CC;

    S.open(S_path, std::ios::out | std::ios::binary | std::ios::trunc);
    SS.open(SS_path, std::ios::out | std::ios::binary | std::ios::trunc);
    C.open(C_path, std::ios::out | std::ios::binary | std::ios::trunc);
    CC.open(CC_path, std::ios::out | std::ios::binary | std::ios::trunc);

    assert(S.is_open());
    assert(SS.is_open());
    assert(C.is_open());
    assert(CC.is_open());

    std::sort(compBicl->linked_s.begin(), compBicl->linked_s.end(), [](pair<uint32_t, vector<uint32_t>> a, pair<uint32_t, vector<uint32_t>> b){ return a.first < b.first; });

    //file << "Bicliques: " << endl;
    for (size_t i = 0; i < compBicl->c_bicliques.size(); i++) {
        //file << "B[" << i << "]: ";
        CC.write((char*)&on, sizeof(bool));
        for(auto j : compBicl->c_bicliques.at(i)) {
            //file << "(" << j.first << "," << j.second << ")" << " ";
            C.write((char*)&(j.first), sizeof(uint32_t));
            C.write((char*)&(j.second), sizeof(uint32_t));
            CC.write((char*)&off, sizeof(bool));
        } 
        //file << endl;
    }
    //file << "S: " << endl;
    for (auto i : compBicl->linked_s) {
        //file << i.first << ": ";
        S.write((char*)&(i.first), sizeof(uint32_t));
        SS.write((char*)&on, sizeof(bool));
        //cout << i.first <<  ",1: " ;
        for (auto j : i.second) {
            S.write((char*)&j, sizeof(uint32_t));
            SS.write((char*)&off, sizeof(bool));
            //cout << j << ",0" << "  ";
        }
        //cout << endl;
    }
    S.close();
    SS.close();
    C.close();
    CC.close();
    
    return; 
}


void saveBicliques(vector<Biclique>* bicliques) 
{

    auto compBicl = generateCompactStructure(bicliques);
    //writeCompactStructure(compBicl);
    //writeCompactStructureBin(compBicl);
    delete compBicl;

    ofstream file;
    string new_path = name + "_bicliques.txt";
    file.open(new_path, fstream::trunc);
    assert(file.is_open());

    for (auto i : *bicliques) {
        auto S = i.S;
        auto C = i.C;

        for (auto j : S) {
            file << j << " ";
        }
        file << ";"; 
        for (auto j : C) {
            file << " ";
            #ifdef WEIGHTED
            file << "(" ;
            #endif
            file << j.first;
            #ifdef WEIGHTED
            file << "," << j.second << ")";
            #endif
        }
        file << std::endl;
    }
}



void convertToWeighted(const string path, const int weight){
    ifstream file;
    file.open(path);

    string outPath = path;
    while(outPath.back() != '.') outPath.pop_back();
    outPath.pop_back();
    ofstream outFile;
    outFile.open(outPath + "_weighted.txt", std::ofstream::out | std::ofstream::trunc);

    assert(file.is_open());
    assert(outFile.is_open());


    string line; 

    while(getline(file, line)){
        auto adjacents = splitString(line, " ");
        if(adjacents.empty()) continue; 

        long long int id = atoi(adjacents.at(0).c_str());

        for(int i = 1; i < adjacents.size(); i++){
            outFile << id << " " << adjacents[i] << " " << weight << endl;
        }

    }
    file.close();
    outFile.close();
}
/*
void buildNetflixTxt(const string path){
    vector<Node*> matrix;
    cout << "format: " << "id rank ady" << endl;
	ifstream file;
	file.open(path);
	assert(file.is_open());
	string line; 


    cout << "Reading file" << endl;
	long long int count = 0;
	while(getline(file, line)){
		if(line.front() < '0' and line.front() > '9') continue;
		auto content = splitString(line, " ");
		uint32_t id = atoi(content[0].c_str()); //id

		while(matrix.size() <= id){
			matrix.push_back(new Node(matrix.size(), true));
		}
		uint32_t weight = atoi(content[1].c_str()); //ratings
		uint32_t adj = atoi(content[2].c_str()); //movie
		(matrix.at(id))->addAdjacent(adj, weight);
		if(count++ == 6000000) break;
	}
	
    cout << "Writing File" << endl; 


    string outPath = path;
    while(outPath.back() != '.'){
        outPath.pop_back();
    }
    outPath.pop_back();
    outPath += "normalized.txt"; 
    ofstream outFile; 
    outFile.open(outPath, std::ofstream::out | std::ofstream::trunc);
    outFile << "% " << path << endl; 
    for(auto i : matrix){
        for(auto j = i->wAdjacentsBegin(); j != i->wAdjacentsEnd(); j++){
            outFile << i->getId() << " " << (*j).first << " " << (*j).second << endl; 
        }
    }
}
*/



int main(int argc, char const *argv[])
{
    //srand(time(NULL));
    cout << "size uint32_t: " << sizeof(uint32_t) << endl;
    if (argc == 3) {
        convertToWeighted(argv[1], atoi(argv[2]));
        return 0;
    } else if (argc == 2) {
        //buildNetflixTxt(argv[1]);
        return 0;
    } else if (argc == 1) {
        auto b = generateBicliques();
        generateGraph(b);
        saveBicliques(b);
        delete b;
        return 0;
    } else if (argc == 4) { 
        graphNodes = edges + edgesBicl;
        edges = atoi(argv[1]);
        porcentaje = float(atoi(argv[2]))/100;
        SxC_Biclique = atoi(argv[3]);
        size_s = sqrt(SxC_Biclique);
        desvest = size_s * 0.2;
        edgesBicl = edges * porcentaje; 
        name = "g_" + to_string(edges) + "_" + to_string(int(porcentaje*100)) + "_" + to_string(SxC_Biclique);

        std::cout << "graph nodes: " << graphNodes << std::endl;
        std::cout << "edges: " << edges << std::endl;
        std::cout << "compression rate: " << porcentaje << std::endl;
        std::cout << "average edges per biclique: " << SxC_Biclique << std::endl;

        auto b = generateBicliques();
        generateGraph(b);
        saveBicliques(b);
        delete b;
        return 0;

    }



    return 0;
}
