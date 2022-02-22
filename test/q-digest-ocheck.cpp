#include "q-digest-mpc/q-digest-mpc.h"
#include <iostream>
#include <string>
#include <fstream>
#include <math.h>
#include "parse-utils.h"
#include <algorithm>

using namespace emp;
using namespace std;

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


int parse_nodes_ocheck(string path, QDigestNode *res, int alice_len, int party) {
    //ALICE input
    int k;
    if (party == ALICE) {
        ifstream ifs(path);
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
            }
            ifs.close();
        } else {
            cout << "FILE \"" << path << "\" NOT EXIST" << endl;
            return -1;
        }
        return 0;
    }
    // BOB INPUT
    if (party == BOB) {
        k = 0; 
        cout << "Parsing nodes: BOB" << endl;
        for (; k < alice_len; k++) {
            res[k] = QDigestNode(0, 0, false, false, ALICE);
        }
    }
    return 0;
}

/**
 * MPC OCheck
 * @param Q q-digest set
 * @param n times
 * @param q the length of Q
 * @param k compression parameter
 * @param d Universe index ranges 1-2^(d+1), leaf nodes ranges 2^d-2^(d+1)
 * @return Check result：Bit_true means check passes，otherwise, Bit_False
 */
Bit oCheck(QDigestNode *G, int n, int q, int k, int d) {
    if (q > 3 * k) {
        Bit flag0(false, PUBLIC);
        return flag0;
    }
    int sigma = n / k;
    /*recover the interleaf nodes*/
    
    int len_Q = 2 * q;
    QDigestNode *Q = new QDigestNode[len_Q + 1];
    for (int i = 0; i < q; ++i) {
        Q[i] = G[i];
        int index_H = i + q;
        Bit condition = G[i].isParent;
        auto node = QDigestNode(Integer(QDigestNode::id_len, 0, PUBLIC), Integer(32, 0, PUBLIC),
                                Bit(false, PUBLIC), Bit(false, PUBLIC));
        node.id = IfThenElse_Integer(condition, G[i].id >> 1, node.id);
        node.c = IfThenElse_Integer(condition, Integer(G[i].c), node.c);
        Q[index_H] = node;
    }

    
    Q[len_Q] = QDigestNode(Integer(QDigestNode::id_len, 0, PUBLIC), Integer(32, 0, PUBLIC),
                           Bit(false, PUBLIC), Bit(false, PUBLIC));
    //Sort
    sort(Q, len_Q);
    reverse(Q, len_Q);
/*    cout << "=================== output all start ======================" << endl;
    for (int i = 0; i < len_Q; ++i) {
        auto node1 = Q[i];
        cout << "Q[i]: " << node1.reveal<string>() << endl;
    }
    cout << "=================== output all end   ======================" << endl;*/
    //return
    Bit flag(true, PUBLIC);
    Bit flagFalse(false, PUBLIC);
    
    Integer expV(QDigestNode::id_len, (int64_t) pow(2, d), PUBLIC);
    Integer integerSigma(32, sigma, PUBLIC);
    Integer integerOne(32, 1, PUBLIC);
    for (int i = 0; i < len_Q; ++i) {
        Integer parentId = Q[i].id >> 1;
        Integer next_parentId = Q[i + 1].id >> 1;
        Bit father_condition = !Q[i].isParent & !parentId.equal(next_parentId);
        if (i == 0) {
            Bit basic_condition = integerSigma.geq(Q[i].c); 
            Bit oneValidLeaf = Q[i].id.geq(expV) & !basic_condition;
            Bit validRoot = Q[i].id.equal(integerOne) & basic_condition;
            Bit final_condition = father_condition & !(oneValidLeaf | validRoot);
            flag = IfThenElse_Bit(final_condition, flagFalse, flag);
            //test
/*            if (!flag.reveal(PUBLIC)) {
                auto node1 = Q[i];
                auto node2 = Q[i + 1];
                cout << "Q[i]: " << node1.reveal<string>() << endl;
                cout << "Q[i+1]: " << node2.reveal<string>() << endl;
                cout << "sigma: " << integerSigma.reveal<int64_t>() << endl;
                cout << "expV: " << expV.reveal<int64_t>() << endl;
                cout << "one validLeaf: " << oneValidLeaf.reveal(PUBLIC) << endl;
                cout << "validRoot: " << validRoot.reveal(PUBLIC) << endl;
                cout << "position1\n" << endl;
                break;
            }*/
        } else if (i == 1) {
            Integer prev_parentId = Q[i - 1].id >> 1;
            Integer sum = Q[i].c + Q[i - 1].c;
            Bit condition1 = parentId.equal(prev_parentId);
            Bit condition2 = integerSigma.geq(Q[i].c); // sigma >= Q[i].c
            Bit condition3 = Q[i].id.geq(expV);
            Bit condition4 = Q[i - 1].id.geq(expV);
            Bit condition5 = !integerSigma.geq(sum);    //sum>sigma
            Bit condition6 = integerSigma.geq(Q[i-1].c); 
            Bit oneValidLeaf = condition3 & !condition2 & !condition1;
            Bit twoValidLeaf =
                    condition3 & condition4 & condition5 & condition1 & !Q[i - 1].isParent;
            Bit twoValidInterNode =
                    !condition3 & !condition4 & condition1 & !Q[i - 1].isParent & condition2 & condition5 & condition6;
            Bit validRoot = Q[i].id.equal(integerOne) & condition2;

            Bit final_condition = father_condition & !(oneValidLeaf | twoValidLeaf | twoValidInterNode | validRoot);
            flag = IfThenElse_Bit(final_condition, flagFalse, flag);
        } else {
            Integer prev_parentId = Q[i - 1].id >> 1;
            Integer prev_prev_parentId = Q[i - 2].id >> 1; 
            Integer sum = Q[i].c + Q[i - 1].c;
            Bit condition1 = parentId.equal(prev_parentId);
            Bit condition2 = integerSigma.geq(Q[i].c); // sigma >= Q[i].c
            Bit condition3 = Q[i].id.geq(expV);
            Bit condition4 = Q[i - 1].id.geq(expV);
            Bit condition5 = !integerSigma.geq(sum);    //sum>sigma
            Bit condition6 = !prev_parentId.equal(prev_prev_parentId); 
            Bit condition7 = integerSigma.geq(Q[i-1].c); 
            Bit oneValidLeaf = condition3 & !condition2 & !condition1;
            Bit twoValidLeaf =
                    condition3 & condition4 & condition5 & condition1 & condition6 & !Q[i - 1].isParent; 
            Bit twoValidInterNode =
                    !condition3 & !condition4 & condition1 & condition6 & !Q[i - 1].isParent & condition2 & condition5 & condition7; 
            Bit validRoot = Q[i].id.equal(integerOne) & condition2;

            Bit final_condition = father_condition & !(oneValidLeaf | twoValidLeaf | twoValidInterNode | validRoot);
            flag = IfThenElse_Bit(final_condition, flagFalse, flag);
        }
    }
    return flag;
}

int main(int argc, char **argv) {
    if (argc < 3) {
        cout << "Please input at least 4 parameters, for example:" << endl;
        cout << "./bin/example party port local_merge_path" << endl;
        return -1;
    }

    int port, party;
    parse_party_and_port(argv, &party, &port);
    if (party != ALICE && party != BOB) {
        cout << "Please specify party to ALICE (1) or BOB (2)" << endl;
        return -1;
    }

    NetIO *io = new NetIO(party == ALICE ? nullptr : "127.0.0.1", port);
    setup_semi_honest(io, party);

    //alice input
    int64_t alice_n;
    int64_t alice_len, alice_k, alice_d;
    Integer alice_len_Integer;
    Integer alice_n_Integer;
    Integer alice_k_Integer;
    Integer alice_d_Integer;
    if (party == ALICE) {
        bool res = parse_parameter(&alice_n, &alice_len, &alice_k, &alice_d, argv[3]);
        if (res) {
            alice_len_Integer = Integer(32, alice_len, ALICE);
            alice_n_Integer = Integer(32, alice_n, ALICE);
            alice_k_Integer = Integer(32, alice_k, ALICE);
            alice_d_Integer = Integer(32, alice_d, ALICE);
            cout << "Finish Parsing Input Parameters: " << party << endl;
        } else {
            cout << "ERROR parsing parameters" << endl;
            return -2;
        }
    }
    if (party == BOB) {
        alice_len_Integer = Integer(32, 0, ALICE);
        alice_n_Integer = Integer(32, 0, ALICE);
        alice_k_Integer = Integer(32, 0, ALICE);
        alice_d_Integer = Integer(32, 0, ALICE);
        cout << "Finish Parsing Input Parameters: " << party << endl;
    }
    int size = alice_len_Integer.reveal<int>();
//    cout << "PARTY: " << party << "; " << "SIZE: " << size << endl;
    int64_t ocheck_n = alice_n_Integer.reveal<int64_t>();
    int64_t ocheck_len = size;
    int64_t ocheck_k = alice_k_Integer.reveal<int64_t>();
    int64_t ocheck_d = alice_d_Integer.reveal<int64_t>();

    cout << "n len k d:\n" << ocheck_n << " " << ocheck_len << " " << ocheck_k << " " << ocheck_d << endl;

    QDigestNode::id_len = ocheck_d >= 32 ? 64 : 32;
    
    QDigestNode *res = new QDigestNode[size];

    parse_nodes_ocheck(argv[3], res, ocheck_len, party);

    cout << "Ready to OCheck" << endl;
    auto start = clock_start();
    Bit check_result = oCheck(res, ocheck_n, ocheck_len, ocheck_k, ocheck_d);
    auto merge_time = time_from(start);
    cout << "ocheck time: " << merge_time << " us" << endl;
    cout << "Alice input OCheck Result: " << check_result.reveal(PUBLIC) << endl;

    if (party == ALICE) {
        HalfGateGen <NetIO> *circ = (HalfGateGen <NetIO> *) CircuitExecution::circ_exec;
        cout << "number of gates: " << circ->mitccrh.gid << endl;
    }

    delete io;
}
