//
//

#ifndef Q_DIGEST_NODE_H
#define Q_DIGEST_NODE_H

#include "emp-tool/circuits/number.h"
#include "emp-tool/circuits/comparable.h"
#include "emp-tool/circuits/swappable.h"
#include "emp-tool/circuits/integer.h"
#include "emp-tool/circuits/bit.h"
#include <vector>
#include <bitset>
#include <algorithm>
#include <math.h>
#include <execinfo.h>
#include <string>

using std::string;
using std::min;
namespace emp {
    class QDigestNode : public Swappable<QDigestNode>, public Comparable<QDigestNode> {

    public:
        static int id_len;
    public:
        Integer id; 
        Integer c; 
        Bit isDummy;
        Bit isParent;

        QDigestNode();

        ~QDigestNode();

        QDigestNode(int64_t id, int c, bool is_Dummy, bool is_Parent, int party = PUBLIC);

        QDigestNode(const Integer &id, const Integer &c, const Bit &isDummy, const Bit &isParent);

        template<typename O>
        O reveal(int party = PUBLIC) const;

        //Comparable
        Bit geq(const QDigestNode &rhs) const;

        Bit equal(const QDigestNode &rhs) const;

        //Swappable
        QDigestNode select(const Bit &sel, const QDigestNode &rhs) const;

        QDigestNode operator^(const QDigestNode &rhs) const;

        int size() const;

        QDigestNode operator+(const QDigestNode &rhs) const;

    };

    int QDigestNode::id_len = 32;

    QDigestNode IfThenElse_QNode(Bit &condition, const QDigestNode &res1, const QDigestNode &res2) {
        return res2.select(condition, res1);
    }

    Bit IfThenElse_Bit(Bit &condition, const Bit &res1, const Bit &res2) {
        return res2.select(condition, res1);
    }

    Integer IfThenElse_Integer(Bit &condition, const Integer &res1, const Integer &res2) {
        return res2.select(condition, res1);
    }

    //Swappable
    QDigestNode QDigestNode::select(const Bit &sel, const QDigestNode &rhs) const {
        QDigestNode res(*this);
        res.id = this->id.select(sel, rhs.id);
        res.c = this->c.select(sel, rhs.c);
        res.isDummy = this->isDummy.select(sel, rhs.isDummy);
        res.isParent = this->isParent.select(sel, rhs.isParent);
        return res;
    }

    QDigestNode QDigestNode::operator^(const QDigestNode &rhs) const {
        QDigestNode res(*this);
        res.id = this->id ^ rhs.id;
        res.c = this->c ^ rhs.c;
        res.isDummy = this->isDummy ^ rhs.isDummy;
        res.isParent = this->isParent ^ rhs.isParent;
        return res;
    }

   

    /*this is used for OCheck（id, -isParent）*/
    Bit QDigestNode::geq(const QDigestNode &rhs) const {

        Bit bitTrue(true);
        Bit bitFalse(false);

        Bit res = bitTrue;

        Bit cond1 = this->id == rhs.id;
        Bit cond2 = !cond1;

        res = res.select(cond1, this->isParent);

        res = res.select(cond2, rhs.id >= this->id);

        return res;
    }

   


    Bit QDigestNode::equal(const QDigestNode &rhs) const {
        return this->id.equal(rhs.id) & this->c.equal(rhs.c) & (this->isDummy & rhs.isDummy) &
               (this->isParent & rhs.isParent);
    }

    inline int QDigestNode::size() const {
        return id_len + 34;
    }

/*
    template<>
    inline string QDigestNode::reveal<string>(int party) const {
        string res = "(";
        res.append(this->id.reveal<string>(party));
        res.append(",");
        res.append(this->c.reveal<string>(party));
        res.append(",");
        res.append(this->isDummy.reveal<string>(party));
        res.append(",");
        res.append(this->isParent.reveal<string>(party));
        res.append(")");
        return res;
    }
*/
    template<>
    inline string QDigestNode::reveal<string>(int party) const {
        string res = "(";
        if (id_len > 32) {
            res.append(std::to_string(this->id.reveal<int64_t>()));
        } else {
            res.append(std::to_string(this->id.reveal<int32_t>()));
        }
        res.append(",");
        res.append(std::to_string(this->c.reveal<int32_t>()));
        res.append(",");
        res.append(this->isDummy.reveal(PUBLIC) ? "1" : "0");
        res.append(",");
        res.append(this->isParent.reveal(PUBLIC) ? "1" : "0");
        res.append(")");
        return res;
    }

    QDigestNode QDigestNode::operator+(const QDigestNode &rhs) const {
        return QDigestNode(this->id + rhs.id, this->c + rhs.c, this->isDummy | rhs.isDummy,
                           this->isParent | rhs.isParent);
    }

    QDigestNode::QDigestNode(int64_t _id, int _c, bool _is_Dummy, bool _is_Parent, int party) {
        id = Integer(id_len, _id, party);
        c = Integer(32, _c, party);
        isDummy = Bit(_is_Dummy, party);
        isParent = Bit(_is_Parent, party);
    }

    QDigestNode::QDigestNode(const Integer &_id, const Integer &_c, const Bit &_isDummy, const Bit &_isParent) :
            id(_id),
            c(_c),
            isDummy(_isDummy),
            isParent(_isParent) {}

    QDigestNode::~QDigestNode() {
    }

    QDigestNode::QDigestNode() = default;
}

#endif //EMP_SH2PC_Q_DIGEST_NODE_H
