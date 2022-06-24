#include <string.h>
#include <iostream>
#include <string>
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
using namespace std;

typedef pair<int,int> NetValue;     // left, right
typedef pair<int,int> VertexPair;   // vertex id, gain value
class Vertex
{
public:
    int id;
    int size;
    int gain;
    bool lock;
    set<int> adj_cell;
    set<int> adj_net;
};

class Graph
{
public:
    int area_constraint;
    int n, m;               // n: number of cells; m: number of nets
    int S_max;              // max size cell size
    int A;                  // A group total size
    float r;                // fraction
    float lower_bound;
    float upper_bound;
    float mid;
    int total_area;                         // total area of cells
    bool is2way;
    int groupNum;

    map<int, Vertex > cell;                 // cell
    map<int, vector<int> > net;             // net, connected cells
    map<int, NetValue > netDistribution;    // net F, T value

    vector<bool> bitvector;
    vector<int> kwayvector;

    // only contain unlock cell
    map<int, int> gain_vec;         // unlock cell id - gain
    map<int, set<int> > bucket;  // gain - unlock cell id
    vector<bool> locked;

    int choosen_cell;                   // id for the choosen cell to swap
    int gain_max;                       // max gain value
    int step;
    int partial_sum;
    int partial_sum_max;
    int ans;                            // step number for the best ans

    map<int, vector<int> > log_table;       // [step] [choosen cell, gain_max] [A, partial sum]
    map<int, vector<bool>> log_pattern;         // [step] pattern

    Graph(int a, int n_) 
    {
        area_constraint = a;
        total_area = 0;
        n = n_;
        S_max = 0;
        A = 0;
        r = 0.55;
        step = 0;
        partial_sum = 0;
        step = 0;
        groupNum = 0;
    }

    void initBitVector()
    {
        // make a partition based on area constraint
        for(int i = 0; i < n; i ++) {
            // if( (A + cell[i].size) <= area_constraint) {
            if( (A + cell[i].size) <= total_area*r) {
                // cell i can be in group 0
                bitvector.push_back(0);
                A += cell[i].size;
            }
            else bitvector.push_back(1);

            locked.push_back(0);
        }
        
        // create net distribution, may be further optimized
        // for each net
        for(int i = 0; i < m; i ++) {
            int f = 0;
            int t = 0;
            // for each cell this net connects to
            for(auto j: net[i]) {   
                if(bitvector[j] == 0) f ++;
                else t ++;
            }
            netDistribution[i] = make_pair(f,t);
        }
        log_pattern[0] = bitvector;
        lower_bound = total_area*r - S_max;
        upper_bound = total_area*r + S_max;
        mid = (lower_bound + upper_bound) / 2;
    }

    void initGain()
    {
        // init gain value
        map<int, int> vec;
        
        for(int i = 0; i < n; i ++) {
            for(auto j: cell[i].adj_net) {
                // first = group 0
                if(bitvector[i] == 0) {
                    if(netDistribution[j].first == 1) vec[i] ++;
                    if(netDistribution[j].second == 0) vec[i] --;
                }
                else {
                    if(netDistribution[j].second == 1) vec[i] ++;
                    if(netDistribution[j].first == 0) vec[i] --;
                }
            }
            bucket[vec[i]].insert(i);
            gain_max = max(gain_max, vec[i]);
        }
        gain_vec = vec;
    }

    void pick()
    {
        // pick a valid cell with max gain value
        bool found = false; // probably dont need this
        // perform size criterion on candidates
        // go through buckets, the backward bucket has larger gain value
        for (auto iter = bucket.rbegin(); iter != bucket.rend() && !found ; ++iter) {
            float distance = INT16_MAX;
            // find a valid swap in this gain value
            for(auto c: bucket[iter->first]) {
                // c for cell id
                // balance?
                
                int A_ = A;     // new A
                if(bitvector[c] == 0) A_ = A - cell[c].size;    // move from left to right
                else A_ = A + cell[c].size;                     // move from right to left
                

                if( A_ < lower_bound || A_ > upper_bound ) continue;
                if( area_constraint > 1600000 && A_ > total_area*r) continue;
                // smaller distance from mid?
                if( abs(A_-mid) < distance ) {
                    choosen_cell = c;
                    gain_max = gain_vec[c];
                    distance = abs(A_-mid);
                }
            }
            if(distance != INT16_MAX) {
                found = true;
                return;
            }
        }
    }
    
    void removeFromBucket(int bucket_id, int cell_id)
    {
        bucket[bucket_id].erase(cell_id);
        // if bucket empty, erase it
        if(bucket[bucket_id].size() == 0)
            bucket.erase(bucket_id);
    }

    void partialSumComp()
    {
        // new partial sum: partial_sum
        if(partial_sum > partial_sum_max) {
            partial_sum_max = partial_sum;
            ans = step;
        }
        else if(partial_sum >= partial_sum_max) {
            // compare balance
            // old best: ans, partial_sum_max
            // new best: step, partial_sum
            float distance1 = abs(log_table[ans][2] - total_area/2 );
            float distance2 = abs(log_table[step][2] - total_area/2 );
            if(distance2 < distance1) {
                // update ans
                partial_sum_max = partial_sum;
                ans = step;
            }
        }
    }

    int calculateCost_2way() 
    {
        int ret = 0;
        vector<bool> ans_pattern = log_pattern[ans]; // this is the final partition
        for(auto i: net) {
            // net i
            set<int> span;
            for(auto j: i.second) {
                // net i spans to group of cell j
                span.insert(ans_pattern[j]);
            }
            if(span.size() != 0)
                ret += pow((span.size()-1),2);
        }
        return ret;
    }

    int calculateCost_2way_init() 
    {
        int ret = 0;
        for(auto i: net) {
            // net i
            set<int> span;
            for(auto j: i.second) {
                // net i spans to group of cell j
                span.insert(bitvector[j]);
            }
            ret += pow((span.size()-1),2);
        }
        return ret;
    }

    void checker()
    {
        vector<bool> vec = bitvector;
        if(is2way) {
            int A = 0;
            for(int i = 0; i < n; i ++) {
                if(vec[i] == 0) A += cell[i].size;
            }
            cout << "left group: " << A << endl;
            cout << "right group:" << total_area - A << endl;
            if(A <= area_constraint && (total_area - A) <= area_constraint)
                cout << "--- valid partition\n";
        }
        else {
            cout << "ac: " << area_constraint << endl;
            map<int, int> v;
            for(int i = 0; i < n; i ++) {
                v[kwayvector[i]] += cell[i].size;
            }
            for(auto i: v) {
                if(i.second > area_constraint) {
                    cout << "kway invalid group: " << i.first << endl;
                    return;
                }
            }
            cout << "kway all valid : )\n";
        }
    }

    void update()
    {
        /*
            // only contain unlock cell
            map<int, int> gain_vec;         // unlock cell id - gain
            map<int, vector<int> > bucket;  // gain - unlock cell id
        */
        vector<bool> update_or_not = locked;    // only unlock and un-updated_cell should be remove from bucket
        set<int> updated_cell;                       // save cell id that updated_cell in this iteration of update
        bool F = bitvector[choosen_cell];
        bool T = !F;
        locked[choosen_cell] = 1;
        
        // cell from left to right
        for(auto net_num: cell[choosen_cell].adj_net) {
            ////// before move : T=0, g+; T=1, T-g-
            // for F=0, T=right=second
            int Fn, Tn;
            if(F == 0) Tn = netDistribution[net_num].second;
            else Tn = netDistribution[net_num].first;

            if(Tn == 0) {
                for(auto cell_num: net[net_num]) {
                    if(cell_num == choosen_cell || locked[cell_num]) continue;
                    // get it update
                    if(!update_or_not[cell_num]) {
                        removeFromBucket(gain_vec[cell_num] , cell_num);
                        update_or_not[cell_num] = 1;
                        updated_cell.insert(cell_num);
                    }
                    gain_vec[cell_num] ++;
                }
            }
            else if(Tn == 1) {
                for(auto cell_num: net[net_num]) {
                    if(cell_num == choosen_cell || locked[cell_num]) continue;
                    if(bitvector[cell_num] == T) {
                        if(!update_or_not[cell_num]) {
                            removeFromBucket(gain_vec[cell_num] , cell_num);
                            update_or_not[cell_num] = 1;
                            updated_cell.insert(cell_num);
                        }
                        gain_vec[cell_num] --; // !!!
                    }
                }
            }
            ////// change net distribution to reflect the move
            if(F == 0) {
                netDistribution[net_num].first --;
                netDistribution[net_num].second ++;
            }
            else {
                netDistribution[net_num].second --;
                netDistribution[net_num].first ++;
            }

            if(F == 0) Fn = netDistribution[net_num].first;
            else Fn = netDistribution[net_num].second;

            ////// after move : F=0, g-; F=1, F-g+
            // for F=0, F=left=first
            if(Fn == 0) {
                for(auto cell_num: net[net_num]) {
                    if(cell_num == choosen_cell || locked[cell_num]) continue;
                    if(!update_or_not[cell_num]) {
                        removeFromBucket(gain_vec[cell_num] , cell_num);
                        update_or_not[cell_num] = 1;
                        updated_cell.insert(cell_num);
                    }
                    gain_vec[cell_num] --;
                }
            }
            else if(Fn == 1) {
                for(auto cell_num: net[net_num]) {
                    if(cell_num == choosen_cell || locked[cell_num]) continue;
                    if(bitvector[cell_num] == F) {
                        if(!update_or_not[cell_num]) {
                            removeFromBucket(gain_vec[cell_num] , cell_num);
                            update_or_not[cell_num] = 1;
                            updated_cell.insert(cell_num);
                        }
                        gain_vec[cell_num] ++; // !!!
                    }
                }
            }
        }
        // move this cell
        bitvector[choosen_cell] = !bitvector[choosen_cell];
        if(F == 0) A -= cell[choosen_cell].size;
        else A += cell[choosen_cell].size;
        
        // remove the choosen cell from candidate list
        gain_vec.erase(choosen_cell);
        bucket[gain_max].erase(choosen_cell);

        // update bucket
        for(auto i: updated_cell) {
            bucket[ gain_vec[i] ].insert(i);    // gain value - cell id
        }
        log_pattern[step] = bitvector;
    }

    bool isKway()
    {
        if(area_constraint < total_area*0.5) {
            is2way = false;
            return true;
        }
        else {
            is2way = true;
            groupNum = 2;
            return false;
        }
    }

    void kway_initBitVector()
    {
        
        int cur = 0;        // current group id
        int curSize = 0;    // current group size
        // make a valid kway partition
        for(int i = 0; i < n; i ++) {
            if( (curSize + cell[i].size) <= area_constraint) {
                kwayvector.push_back(cur);
                curSize += cell[i].size;
            }
            else {
                // change to a new group
                cur ++;
                curSize = 0;

                kwayvector.push_back(cur);
                curSize += cell[i].size;
            }
        }
        groupNum = cur+1;
    }

    int calculateCost_kway() 
    {
        
        int ret = 0;
        for(auto i: net) {
            // net i
            set<int> span;
            for(auto j: i.second) {
                // net i spans to group of cell j
                span.insert(kwayvector[j]);
            }
            if(span.size() != 0) {
                ret += pow((span.size()-1),2);
            }
        }
        
        // checker();
        return ret;
    }

};