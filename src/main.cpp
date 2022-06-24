#include <iostream>
#include <vector>
#include <iomanip>
#include <algorithm>
#include <iterator>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <math.h>
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>
#include <queue>
#include <fstream>
#include "graph.h"
using namespace std;

int main(int argc, char* argv[])
{

    ifstream cin(argv[1]);
    ofstream cout(argv[2]);
    int area_constraint, n, m; // area constraint per group
    string keyword;

    cin >> area_constraint >> keyword >> n;
    Graph ckt(area_constraint, n);
    // cell info
    int cell_num, size;
    for(int i = 0; i < n; i ++) {
        cin >> cell_num >> size;
        ckt.cell[i].id = i;
        ckt.cell[i].size = size;
        ckt.total_area += size;

        ckt.S_max = max(ckt.S_max, size);
    }
    // net info
    cin >> keyword >> m;
    ckt.m = m;
    for(int i = 0; i < m; i ++) {
        // net i
        int k, temp;
        cin >> k;       // net i contains k cells
        while(k --) {
            cin >> temp;        // cell id

            ckt.net[i].push_back(temp);
            ckt.cell[temp].adj_net.insert(i);   // add this net to this cell
        }
    }
    
    if(!ckt.isKway()) {
        // init bitvector
        ckt.initBitVector(); // easier for init gain
        // init gain
        ckt.initGain();

        while( (ckt.partial_sum > 0 && ckt.step != 0) || ckt.step == 0) {
            ckt.step ++;
            ckt.pick();
            ckt.log_table[ckt.step].push_back(ckt.choosen_cell);
            ckt.log_table[ckt.step].push_back(ckt.gain_max);

            ckt.partial_sum += ckt.gain_max;
            ckt.update();
            ckt.log_table[ckt.step].push_back(ckt.A);
            ckt.log_table[ckt.step].push_back(ckt.partial_sum);

            if(ckt.step == 1) {
                ckt.partial_sum_max = ckt.partial_sum;
                ckt.ans = ckt.step;
            }
            else ckt.partialSumComp();
        }
        // output
        cout << ckt.calculateCost_2way() << endl;
        cout << ckt.groupNum << endl;
        for(auto i: ckt.log_pattern[ckt.ans])
            cout << i << endl;
    }
    else {
        ckt.kway_initBitVector();
        // output
        cout << ckt.calculateCost_kway() << endl;
        cout << ckt.groupNum << endl;
        for(auto i: ckt.kwayvector)
            cout << i << endl;
    }
    return 0;
}