/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   textdiff.cc
 *
 *  Author          :   Martijn Koedam (m.l.p.j.koedam@tue.nl)
 *
 *  Date            :   Nov 15, 2011
 *
 *  Function        :
 *
 *  History         :
 *      15-11-11    :   Initial version.
 *      07-03-16    :   Fixed segfault my_sort (Rvk)
 *
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
#include "base/basic_types.h" // IWYU pragma: keep

// Needed for sort
#include "base/string/cstring.h"
#include "textdiff.h"
#include <algorithm>
#include <memory>

/**
 * This function can do a diff on 2 text strings.
 * The diff is done line by line.
 *
 * The diff implements: "An algorithm for differential file comparison - J.W. Hunt (1976)
 * @todo jackpot detection.
 */

/**
 * Candidate
 */
struct Candidate {
    int a, b;
    Candidate *prev;
    Candidate(int la, int lb, Candidate *lprev) : a(la), b(lb), prev(lprev) {}
};

class CandidateList : public std::vector<std::unique_ptr<Candidate>> {
public:
    CandidateList() = default;
    Candidate *newCandidate(int a, int b, Candidate *prev) {
        this->emplace_back(std::make_unique<Candidate>(a, b, prev));
        return this->back().get();
    }
};

namespace {

bool my_sort(std::pair<int, size_t> a, std::pair<int, size_t> b) {
    // primary hash
    if (a.second < b.second) {
        return true;
    }
    if (a.second == b.second) {
        // second line number
        if (a.first < b.first) {
            return true;
        }
    }
    return false;
}

/**
 * Merge algorithm
 */
void merge(CandidateList &candidates,
           std::vector<Candidate *> &K,
           int &k,
           int index,
           std::vector<std::pair<int, bool>> E,
           int p) {
    int r = 0;
    Candidate *c = K[0];
    do {
        int j = E[p].first;
        for (int s = r; s <= k; s++) {
            if (K[s]->b < j && K[s + 1]->b > j) {
                if (K[s + 1]->b > j) {
                    K[r] = c;
                    r = s + 1;
                    c = candidates.newCandidate(index, j, K[s]);
                }
                if (s == k) {
                    // move fence.
                    K[k + 2] = K[k + 1];
                    k = k + 1;
                }

                break;
            }
        }
        if (E[p].second) {
            break;
        }
        p = p + 1;
    } while (true);
    K[r] = c;
}

CString diff(CStrings &lines_a, CStrings &lines_b) {
    
    CandidateList candidates;
    std::vector<std::pair<size_t, CString *>> hash_a(lines_a.size());
    std::vector<std::pair<size_t, CString *>> hash_b(lines_b.size());

    // Generate Hash, String std::pairs for both files.
    // Do not copy the string, so pass pointers around.
    // I know the lifetime of the strings is longer than hash_a/b
    int index = 0;
    for (auto &i : lines_a) {
        CString *a = &(i);
        hash_a[index++] = std::pair<size_t, CString *>(std::hash<std::string>{}(*a), a);
    }
    index = 0;
    for (auto &i : lines_b) {
        CString *a = &(i);
        hash_b[index++] = std::pair<size_t, CString *>(std::hash<std::string>{}(*a), a);
    }

    // Start diff
    // Length off both strings.
    // (+1 because we start counting @ 1)
    int n = static_cast<int>(hash_b.size() + 1);
    int m = static_cast<int>(hash_a.size() + 1);

    // V[j] = (j, Hash(hash_b[j])) j = 1,...,n
    std::vector<std::pair<int, size_t>> V(n);
    index = 1;
    for (auto i : hash_b) {
        V[index] = std::pair<int, size_t>(index, i.first);
        index++;
    }

    // Sort list, first on hash, then on line number
    std::sort(V.begin(), V.end(), my_sort);

    // E[j] <-- (V[j].line_number, f(j)) j=1,....,n
    // f(j) = (j == n || V[j].hash != V[j+1].hash)
    std::vector<std::pair<int, bool>> E(n);
    // E[0] <-- (0,true)
    E[0] = std::pair<int, bool>(0, true);
    for (index = 1; index < n; index += 1) {
        bool fj = (index + 1 == n) || V[index].second != V[index + 1].second;
        E[index] = std::pair<int, bool>(V[index].first, fj);
    }

    // P[i] = (E[j-1].f(j-1) == true && H(i) = V[j].hash)? j:0 i=1,...,m
    std::vector<int> P(m);
    for (index = 1; index < m; index++) {
        P[index] = 0;
        /// @todo: use binary search for this? V[].hash is ordered.
        for (int j = 1; j < n; j++) {
            if (E[j - 1].second && hash_a[index - 1].first == V[j].second) {
                P[index] = j;
                break;
            }
        }
    }

    // K
    std::vector<Candidate *> K((std::min)(m, n) + 1);
    int k = 0;
    K[0] = candidates.newCandidate(0, 0, nullptr);
    K[1] = candidates.newCandidate(m + 1, n + 1, nullptr);
    for (index = 1; index < m; index++) {
        if (P[index] != 0) {
            int p = P[index];
            merge(candidates, K, k, index, E, p);
        }
    }

    // Construct matching.
    std::vector<int> J(m, 0);
    Candidate *c = K[k];
    while (c != nullptr) {
        J[c->a] = c->b;
        c = c->prev;
    }
    // Construct diff
    int sa = 1;
    int sb = 1;
    int sa_o = 0;
    CString diff;

    ///  @todo Do this a bit more efficient.
    while (sa < m) {
        if (J[sa] == 0) {
            sa_o--;
            (diff) += CString(sa) + CString("d") + CString(sa + sa_o) + CString("\n");
            (diff) += CString("<") + CString(*hash_a[sa - 1].second) + CString("\n");
            sa++;
        } else if (J[sa] != (sb)) {
            (diff) += CString(sa + (sa_o));
            (diff) += CString("a") + CString(sa + sa_o) + CString("\n");
            (diff) += CString(">") + CString(*hash_b[sb - 1].second) + CString("\n");
            sa_o++;
            sb++;
        } else {
            sa++;
            sb++;
        }
    }

    while (sb < n) {
        (diff) += CString(sa + (sa_o));
        (diff) += CString("a") + CString(sa + sa_o) + CString("\n");
        (diff) += CString(">") + CString(*hash_b[sb - 1].second) + CString("\n");
        sa_o++;
        sb++;
    }

    // Cleanup
    K.clear();
    return diff;
}

} // namespace

/**
 * Constructor
 */
CString TextDiff(CString &file_a, CString &file_b, char split) {
    CStrings lines_a = file_a.split(split);
    CStrings lines_b = file_b.split(split);
    return diff(lines_a, lines_b);
}
