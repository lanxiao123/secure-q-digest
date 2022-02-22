#include "q-digest-mpc/q-digest-mpc.h"
#include <iostream>
#include <string>
#include <fstream>
#include <math.h>
#include "parse-utils.h"
#include <algorithm>

using namespace emp;
using namespace std;

int parse_nodes(string path, QDigestNode *res, int total_len, int alice_len, int party) {
    int k;
    ifstream ifs(path);
    //ALICE input
    if (party == ALICE) {
        cout << "Parsing nodes: ALICE" << endl;
        k = 0;
        if (ifs) {
            //read line
            string line;
            getline(ifs, line);
            while (getline(ifs, line)) {
                string temp[4];
                int s = 0;
                for (char i : line) { 
                    if (i == '(' || i == ')') {
                        continue;
                    }
                    if (i == ',') {
                        s++;
                        continue;
                    }
                    temp[s] += i;
                }
                res[k] = QDigestNode((int64_t) stoll(temp[0]), stoi(temp[1]), stoi(temp[2]) == 1, stoi(temp[3]) == 1,
                                     ALICE);
                k++;
            }//k should be alice_len
            for (; k < total_len; k++) {
                res[k] = QDigestNode(0, 0, false, false, BOB);
            }
        } else {
            cout << "FILE \"" << path << "\" NOT EXIST" << endl;
            return -1;
        }
        return 0;
    }
    // BOB INPUT
    if (party == BOB) {
        k = 0; 
        if (ifs) {
            cout << "Parsing nodes: BOB" << endl;
            for (; k < alice_len; k++) {
                res[k] = QDigestNode(0, 0, false, false, ALICE);
            }
            //read line
            string line;
            getline(ifs, line);
            while (getline(ifs, line)) {
                string temp[4];
                int s = 0;
                for (char i : line) { 
                    if (i == '(' || i == ')') {
                        continue;
                    }
                    if (i == ',') {
                        s++;
                        continue;
                    }
                    temp[s] += i;
                }
                res[k] = QDigestNode((int64_t) stoll(temp[0]), stoi(temp[1]), stoi(temp[2]) == 1, stoi(temp[3]) == 1,
                                     BOB);
                k++;
            }
        } else {
            cout << "FILE \"" << path << "\" NOT EXIST" << endl;
            return -1;
        }
        return 0;
    }
}

template<typename T>
void swap_haha(T &a, T &b) {
    T t = a;
    a = b;
    b = t;
}


void reverse(QDigestNode *G, int length) {
//    Bit b(true, PUBLIC);
    for (int i = 0; i < length / 2; ++i) {
        swap_haha<QDigestNode>(G[i], G[length - 1 - i]);
    }
}

void printOMergeResultToFile(QDigestNode *G, int length, string path) {
    ofstream out(path);
    if (out.is_open()) {
        for (int i = 0; i < length - 1; ++i) {
            if (!(G[i].isDummy.reveal(PUBLIC)) && G[i].id.reveal<int32_t>(PUBLIC) != 0) {
                out << G[i].reveal<string>() << "\n";
            }
        }

        if (!(G[length - 1].isDummy.reveal(PUBLIC)) && G[length - 1].id.reveal<int32_t>(PUBLIC) != 0) {
            out << G[length - 1].reveal<string>();
        }
        out.close();
    }
}


void oMerge(QDigestNode *G, int64_t length, int64_t d, int64_t sigma) {
    sort(G, length);
    reverse(G, length);

    
    for (int64_t i = 0; i < length - 1; ++i) {
        Bit condition = (G[i].id.equal(G[i + 1].id)) & (G[i].isParent == G[i + 1].isParent);
        Integer new_c = G[i].c + G[i + 1].c;
        G[i].c = IfThenElse_Integer(condition, new_c, G[i].c);
        G[i + 1].isDummy = IfThenElse_Bit(condition, Bit(true, PUBLIC), G[i + 1].isDummy);
    }

    sort(G, length);
    reverse(G, length);

    /* start merge q-digest */
    //define null node
    G[length] = QDigestNode(Integer(QDigestNode::id_len, 0, PUBLIC), Integer(32, 0, PUBLIC),
                            Bit(false, PUBLIC), Bit(false, PUBLIC));
    Integer integerSigma(32, sigma, PUBLIC);
    for (int64_t j = d; j >= 1; --j) { 
        Integer low(QDigestNode::id_len, exp2(j), PUBLIC);
        Integer high(QDigestNode::id_len, exp2(j + 1), PUBLIC);
        for (int i = 0; i < length; ++i) { 
      
            Integer parentId = G[i].id >> 1;
            Integer next_parentId = G[i + 1].id >> 1;
            Bit basic_condition =
                    !G[i].isDummy & G[i].id.geq(low) & !G[i].id.geq(high) & (!parentId.equal(next_parentId));
            if (i == 0) {
                Bit condition1 = basic_condition & integerSigma.geq(G[i].c);
                G[i].id = IfThenElse_Integer(condition1, parentId, G[i].id);
                G[i].isParent = IfThenElse_Bit(condition1, Bit(false, PUBLIC), G[i].isParent);
            } else if (i == 1) {
                Integer prev_parentId = G[i - 1].id >> 1;
                Integer sum = G[i].c + G[i - 1].c;
                Bit condition1 = !parentId.equal(prev_parentId);
                Bit condition2 = integerSigma.geq(G[i].c);
                Bit condition3 = integerSigma.geq(sum);
                
                Bit one_node_merge = basic_condition & condition1 & condition2;
                Bit one_node_recover = basic_condition & condition1 & !condition2 & G[i].isParent;
                Bit two_node_merge = basic_condition & !condition1 & condition3;
                Bit two_node_recover = basic_condition & !condition1 & !condition3 & G[i].isParent;
                Bit recover_parent = one_node_recover | two_node_recover;
                //one node merge
                G[i].id = IfThenElse_Integer(one_node_merge, parentId, G[i].id);
                G[i].isParent = IfThenElse_Bit(one_node_merge, Bit(false, PUBLIC), G[i].isParent);
                //two node merge
                G[i].id = IfThenElse_Integer(two_node_merge, parentId, G[i].id);
                G[i].c = IfThenElse_Integer(two_node_merge, sum, G[i].c);
                G[i].isParent = IfThenElse_Bit(two_node_merge, Bit(false, PUBLIC), G[i].isParent);
                G[i - 1].isDummy = IfThenElse_Bit(two_node_merge, Bit(true, PUBLIC), G[i - 1].isDummy);
                //recoverParent
                G[i].id = IfThenElse_Integer(recover_parent, parentId, G[i].id);
                G[i].isParent = IfThenElse_Bit(recover_parent, Bit(false, PUBLIC), G[i].isParent);
            } else {
                Integer prev_parentId = G[i - 1].id >> 1;
                Integer prev_prev_parentId = G[i - 2].id >> 1;
                Integer sumTwo = G[i].c + G[i - 1].c;
                Integer sumThree = G[i].c + G[i - 1].c + G[i - 2].c;
                Bit condition1 = !parentId.equal(prev_parentId);
                Bit condition2 = integerSigma.geq(G[i].c);
                Bit condition3 = !prev_parentId.equal(prev_prev_parentId);
                Bit condition4 = integerSigma.geq(sumTwo);
                Bit condition5 = integerSigma.geq(sumThree);
                
                Bit one_node_merge = basic_condition & condition1 & condition2;
                Bit one_node_recover = basic_condition & condition1 & !condition2 & G[i].isParent;
                Bit two_node_merge = basic_condition & !condition1 & condition3 & condition4;
                Bit two_node_recover = basic_condition & !condition1 & condition3 & !condition4 & G[i].isParent;
                Bit three_node_merge = basic_condition & !condition1 & !condition3 & condition5;
                Bit three_node_recover = basic_condition & !condition1 & !condition3 & !condition5 & G[i].isParent;
                Bit recover_parent = one_node_recover | two_node_recover | three_node_recover;
                //one node merge
                G[i].id = IfThenElse_Integer(one_node_merge, parentId, G[i].id);
                G[i].isParent = IfThenElse_Bit(one_node_merge, Bit(false, PUBLIC), G[i].isParent);
                //two node merge
                G[i].id = IfThenElse_Integer(two_node_merge, parentId, G[i].id);
                G[i].c = IfThenElse_Integer(two_node_merge, sumTwo, G[i].c);
                G[i].isParent = IfThenElse_Bit(two_node_merge, Bit(false, PUBLIC), G[i].isParent);
                G[i - 1].isDummy = IfThenElse_Bit(two_node_merge, Bit(true, PUBLIC), G[i - 1].isDummy);
                //three node merge
                G[i].id = IfThenElse_Integer(three_node_merge, parentId, G[i].id);
                G[i].c = IfThenElse_Integer(three_node_merge, sumThree, G[i].c);
                G[i].isParent = IfThenElse_Bit(three_node_merge, Bit(false, PUBLIC), G[i].isParent);
                G[i - 1].isDummy = IfThenElse_Bit(three_node_merge, Bit(true, PUBLIC), G[i - 1].isDummy);
                G[i - 2].isDummy = IfThenElse_Bit(three_node_merge, Bit(true, PUBLIC), G[i - 2].isDummy);
                //recoverParent
                G[i].id = IfThenElse_Integer(recover_parent, parentId, G[i].id);
                G[i].isParent = IfThenElse_Bit(recover_parent, Bit(false, PUBLIC), G[i].isParent);
            }
        }
        //Sort
        sort(G, length);
        reverse(G, length);
    }
}

int main(int argc, char **argv) {
    if (argc < 4) {
        cout << "Please input at least 4 parameters, for example:" << endl;
        cout << "./bin/example party port local_merge_path" << endl;
        return -1;
    }

    auto start = clock_start();

    int port, party;
    parse_party_and_port(argv, &party, &port);
    if (party != ALICE && party != BOB) {
        cout << "Please specify party to ALICE (1) or BOB (2)" << endl;
        return -1;
    }

    NetIO *io = new NetIO(party == ALICE ? nullptr : "127.0.0.1", port);
    setup_semi_honest(io, party);

    /*parse parameters*/
    //ALICE
    int64_t alice_n;
    int64_t alice_len, alice_k, alice_d;
    Integer alice_len_Integer;
    Integer alice_n_Integer;
    Integer alice_k_Integer;
    Integer alice_d_Integer;
    Integer bob_len_Integer;
    Integer bob_n_Integer;
    Integer bob_k_Integer;
    Integer bob_d_Integer;
    if (party == ALICE) {
        bool res = parse_parameter(&alice_n, &alice_len, &alice_k, &alice_d, argv[3]);
        if (res) {
            alice_len_Integer = Integer(32, alice_len, ALICE);
            alice_n_Integer = Integer(32, alice_n, ALICE);
            alice_k_Integer = Integer(32, alice_k, ALICE);
            alice_d_Integer = Integer(32, alice_d, ALICE);
            bob_len_Integer = Integer(32, 0, BOB); 
            bob_n_Integer = Integer(32, 0, BOB);
            bob_k_Integer = Integer(32, 0, BOB);
            bob_d_Integer = Integer(32, 0, BOB);
            QDigestNode::id_len = alice_d >= 32 ? 64 : 32;
            cout << "Finish Parsing Input Parameters: " << party << endl;
            cout << "n len k d:\n" << alice_n << " " << alice_len << " " << alice_k << " " << alice_d << endl;
        } else {
            cout << "ERROR parsing parameters" << endl;
            return -2;
        }
    }
    //BOB
    int64_t bob_n;
    int64_t bob_len, bob_k, bob_d;
    if (party == BOB) {
        bool res = parse_parameter(&bob_n, &bob_len, &bob_k, &bob_d, argv[3]);
        if (res) {
            alice_len_Integer = Integer(32, 0, ALICE);
            alice_n_Integer = Integer(32, 0, ALICE);
            alice_k_Integer = Integer(32, 0, ALICE);
            alice_d_Integer = Integer(32, 0, ALICE);
            bob_len_Integer = Integer(32, bob_len, BOB);
            bob_n_Integer = Integer(32, bob_n, BOB);
            bob_k_Integer = Integer(32, bob_k, BOB);
            bob_d_Integer = Integer(32, bob_d, BOB);
            QDigestNode::id_len = bob_d >= 32 ? 64 : 32;
            cout << "Finish Parsing Input Parameters: " << party << endl;
            cout << "n len k d:\n" << bob_n << " " << bob_len << " " << bob_k << " " << bob_d << endl;
        } else {
            cout << "ERROR parsing parameters" << endl;
            return -2;
        }
    }

    int size = (bob_len_Integer + alice_len_Integer).reveal<int>(); 
//    cout << "SIZE: " << size << endl;

    QDigestNode *res = new QDigestNode[size + 1];

    int alice_num = alice_len_Integer.reveal<int>();
    cout << "Parsing nodes" << endl;
    parse_nodes(argv[3], res, size, alice_num, party);

    res[size] = QDigestNode(0, 0, false, false, PUBLIC);

    int64_t merge_k = alice_k_Integer.reveal<int64_t>();
    int64_t merge_d = alice_d_Integer.reveal<int64_t>();
    int64_t merge_k1 = bob_k_Integer.reveal<int64_t>();
    int64_t merge_d1 = bob_d_Integer.reveal<int64_t>();
    if (merge_k != merge_k1 || merge_d != merge_d1) {
        cout << "ERROR!!! Input parameters are not matched!!" << endl;
        return -1;
    }

    int merge_n = (alice_n_Integer + bob_n_Integer).reveal<int>();
    int64_t sigma = int64_t (merge_n / merge_k);

    cout << "Ready to merge" << endl;

    oMerge(res, size, merge_d, sigma);
    auto merge_time = time_from(start);
    cout << "o_merge time: " << merge_time << " us" << endl;

/*    for (int i = 0; i < size; ++i) {
        if (!(res[i].isDummy.reveal(PUBLIC))) {
            cout << res[i].reveal<string>() << " ";
        }
    }*/
    cout << endl;
    if (argc == 5) {
        printOMergeResultToFile(res, size, argv[4]);
    }

    if (party == ALICE) {
        HalfGateGen <NetIO> *circ = (HalfGateGen <NetIO> *) CircuitExecution::circ_exec;
        cout << "number of gates: " << circ->mitccrh.gid << endl;
        ofstream stat_result;
        stat_result.open("stat_result.txt",ofstream::app);
        stat_result<<argv[3]<<" "<<"merge time is "<<merge_time<<"; gates are "<<circ->mitccrh.gid<<endl;
        stat_result.close();
    }

    delete io;
}
