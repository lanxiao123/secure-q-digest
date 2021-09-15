#include "q-digest-mpc/q-digest-mpc.h"
#include <iostream>
#include <string>
#include <fstream>
#include <math.h>
#include <algorithm>

using namespace emp;
using namespace std;

int main(int argc, char **argv) {

    int port, party;
    parse_party_and_port(argv, &party, &port);
    if (party != ALICE && party != BOB) {
        cout << "Please specify party to ALICE (1) or BOB (2)" << endl;
        return -1;
    }

    NetIO *io = new NetIO(party == ALICE ? nullptr : "127.0.0.1", port);
    setup_semi_honest(io, party);

    Bit bitTrue(true);
    Bit bitFalse(false);

    Integer a(32, 11, ALICE); //executed by ALICE
    Integer b(32, 22, BOB); //executed by BOB

    cout << "=================================" << endl;
    cout << "b.select" << endl;
    
    Integer res1 = b.select(bitTrue, a);
    Integer res2 = b.select(bitFalse, a);
    cout << res1.reveal<int32_t>(PUBLIC) << endl;
    cout << res2.reveal<int32_t>(PUBLIC) << endl;
    cout << "a.select" << endl;
    Integer res3 = a.select(bitTrue, b);
    Integer res4 = a.select(bitFalse, b);
    cout << res3.reveal<int32_t>(PUBLIC) << endl;
    cout << res4.reveal<int32_t>(PUBLIC) << endl;

    cout << "=================================" << endl;

    QDigestNode node1 = QDigestNode(1, 11, false, false, ALICE);
    QDigestNode node2 = QDigestNode(2, 22, false, false, BOB);
    cout << "original" << endl;
    cout << node1.reveal<string>() << endl;
    cout << node2.reveal<string>() << endl;

    cout << "=================================" << endl;
    cout << "node1.select" << endl;
    
    QDigestNode resNode1 = node1.select(bitTrue, node2);
    QDigestNode resNode2 = node1.select(bitFalse, node2);
    cout << resNode1.reveal<string>() << endl;
    cout << resNode2.reveal<string>() << endl;
    cout << "node1.select" << endl;
    QDigestNode resNode3 = node2.select(bitTrue, node1);
    QDigestNode resNode4 = node2.select(bitFalse, node1);
    cout << resNode3.reveal<string>() << endl;
    cout << resNode4.reveal<string>() << endl;


    cout << "=================================" << endl;
    cout << "use IfThenElse" << endl;
    QDigestNode resNode5 = IfThenElse_QNode(bitTrue, node1, node2);
    QDigestNode resNode6 = IfThenElse_QNode(bitFalse, node1, node2);

    cout << resNode5.reveal<string>() << endl;
    cout << resNode6.reveal<string>() << endl;

    cout << "=================================" << endl;

    Integer test1(32, 1, PUBLIC);
    Integer test11(32, 1, PUBLIC);
    Integer test2(32, 2, PUBLIC);
    Integer test3(32, 3, PUBLIC);
    /*
    1geq1: 1
    2geq1: 1
    1geq2: 0
    1equal1: 1
    2equal1: 0
     */
    cout << "1geq1: " << (test1.geq(test1)).reveal(PUBLIC) << endl;
    cout << "2geq1: " << (test2.geq(test1)).reveal(PUBLIC) << endl;
    cout << "1geq2: " << (test1.geq(test2)).reveal(PUBLIC) << endl;
    cout << "1equal1: " << (test1.equal(test1)).reveal(PUBLIC) << endl;
    cout << "2equal1: " << (test2.equal(test1)).reveal(PUBLIC) << endl;
    cout << "1equal2: " << (test2.equal(test1)).reveal(PUBLIC) << endl;


    cout << "================= split line ================" << endl;

    cout << "QDigestNode::id_len " << QDigestNode::id_len << endl;

    QDigestNode::id_len = 64;
    cout << "QDigestNode::id_len " << QDigestNode::id_len << endl;

    int64_t num = 0x3ffffffff;
    Integer integer_num(35, num, PUBLIC);
    cout << num << ", " << integer_num.reveal<int64_t>(PUBLIC) << endl;
    cout << num << ", " << integer_num.reveal<int32_t>(PUBLIC) << endl;

    delete io;
}
