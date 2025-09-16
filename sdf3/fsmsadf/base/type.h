/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   type.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   April 22, 1009
 *
 *  Function        :   FSM-based SADF type definitions
 *
 *  History         :
 *      22-04-09    :   Initial version.
 *
 * $Id: type.h,v 1.1.2.2 2010-04-22 07:07:11 mgeilen Exp $
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * In other words, you are welcome to use, share and improve this program.
 * You are forbidden to forbid anyone else to use, share and improve
 * what you give them.   Happy coding!
 */

#ifndef FSMSADF_BASE_TYPE_H_INCLUDED
#define FSMSADF_BASE_TYPE_H_INCLUDED

#include "component.h"

namespace FSMSADF {

// Bandwidth
using Bandwidth = double;

// Throughput
using Throughput = CDouble;

// Time
using Time = int;

#define TIME_MAX INT_MAX

// Size
using Size = uint;
const uint MAX_Size = UINT_MAX;

/**
 * Buffer Location
 */
class BufferLocation {
public:
    enum class Value {
        Src = 0,
        Dst,
        Mem,
    };

    explicit BufferLocation(const CString &l) : _v(asLoc(l)) {}
    explicit BufferLocation(const Value &l) : _v(l) {}
    static CString toString(BufferLocation::Value v) {
        auto a = BufferLocation(v);
        return CString(a);
    }
    [[nodiscard]] CString asString() const {
        if (_v == Value::Src) {
            return "src";
        }
        if (_v == Value::Dst) {
            return "dst";
        }
        return "mem";
    }
    [[nodiscard]] static Value asLoc(const CString &l) {
        if (l == "src") {
            return Value::Src;
        }
        if (l == "dst") {
            return Value::Dst;
        }
        return Value::Mem;
    }

    explicit operator CString() const { return asString(); }

    explicit operator int() const {
        return static_cast< std::underlying_type_t<Value>>(_v);
    }

    [[nodiscard]] int asInt() const {
        return static_cast< std::underlying_type_t<Value>>(_v);
    }

    BufferLocation &operator=(Value const &a) {
        _v = a;
        return *this;
    }

    BufferLocation &operator=(CString const &a) {
        _v = asLoc(a);
        return *this;
    }

    bool operator==(Value const &b) const { return this->_v == b; }
    bool operator==(BufferLocation const &b) const { return _v == b._v; }
    bool operator==(CString const &b) const { return _v == asLoc(b); }

    bool operator!=(Value const &b) const { return this->_v != b; }
    bool operator!=(BufferLocation const &b) const { return _v != b._v; }
    bool operator!=(CString const &b) const { return _v != asLoc(b); }

private:
    Value _v;
};
// Buffer size

class BufferSize : public std::vector<Size> {
public:
    Size &operator[](size_type n) {
        if (size() <= n) {
            resize(n + 1, 0);
        }

        return at(n);
    }
    Size operator[](size_type n) const {
        if (size() < n) {
            return 0;
        }

        return at(n);
    }
};

// Storage distribution

class StorageDistribution : public std::map<CString, Size> {
public:
    // Construct
    StorageDistribution() = default;

    explicit StorageDistribution(const std::map<CString, Size>& storageMap);
    // Construct
    void constructFromXML(CNodePtr storageDistributionNode);

    // Convert
    void convertToXML(CNodePtr storageDistributionNode) const;

    // Size of the distribution
    [[nodiscard]] Size getSize() const {
        Size sz = 0;
        for (const auto& [name, ssz] : *this) {
            sz = sz + ssz;
        }
        return sz;
    };

    // Throughput
    [[nodiscard]] Throughput getThroughput() const { return throughput; };
    void setThroughput(const Throughput thr) { throughput = thr; };

    // Access storage size of a specific schannel
    Size &operator[](GraphComponent *c) {
        if (find(c->getName()) == end()) {
            insert(std::pair<CString, Size>(c->getName(), 0));
        }

        return find(c->getName())->second;
    };

    // Access storage size of a specific schannel
    Size operator[](GraphComponent *c) const {
        if (find(c->getName()) == end()) {
            return 0;
        }

        return find(c->getName())->second;
    };

    // Compare storage distributions
    bool operator<(const StorageDistribution &s) const {
        if (getThroughput() < s.getThroughput()) {
            return true;
        }
        return static_cast<const std::map<CString, Size>&>(*this) < s;
    };

private:
    Throughput throughput{};
};

// Storage distributions

class StorageDistributions : public std::set<StorageDistribution> {
public:
    // Construct
    void constructFromXML(CNodePtr storageDistributionsNode);

    // Convert
    void convertToXML(CNodePtr storageDistributionsNode);
};

/**
 * Binding method
 */

class BindingMethod {
public:
    enum class Method { MaxOfScenarios = 0, SumOfScenarios };
    explicit BindingMethod(const CString &l) : _m(asLoc(l)) {}
    explicit BindingMethod(const Method &l) : _m(l) {}
    static CString toString(BindingMethod::Method m) {
        auto a = BindingMethod(m);
        return CString(a);
    }
    [[nodiscard]] CString asString() const {
        if (_m == Method::MaxOfScenarios) {
            return "max";
        }

        return "sum";
    }
    [[nodiscard]] static Method asLoc(const CString &l) {
        if (l == "max") {
            return Method::MaxOfScenarios;
        }

        return Method::SumOfScenarios;
    }

    explicit operator CString() const { return asString(); }

    explicit operator int() const {
        return static_cast<typename std::underlying_type<Method>::type>(_m);
    }
    BindingMethod &operator=(Method const &a) {
        _m = a;
        return *this;
    }
    BindingMethod &operator=(CString const &a) {
        _m = asLoc(a);
        return *this;
    }
    bool operator==(Method const &b) const { return this->_m == b; }
    bool operator==(BindingMethod const &b) const { return _m == b._m; }
    bool operator==(CString const &b) const { return _m == asLoc(b); }

    bool operator!=(Method const &b) const { return this->_m != b; }
    bool operator!=(BindingMethod const &b) const { return _m != b._m; }
    bool operator!=(CString const &b) const { return _m != asLoc(b); }

private:
    Method _m;
};

} // End namespace FSMSADF

#endif
