#include <cstdio>
#include <cassert>
#include <vector>
#include <queue>
#include <limits>
#include <utility>
#include <cmath>

using namespace std;

typedef long long Len;
typedef long long Coord;
typedef priority_queue<pair<Len, int>, vector<pair<Len, int>>, greater<pair<Len, int>>> PQ;

const Len INF = numeric_limits<Len>::max() / 4;

inline Len heuristic(Coord x1, Coord y1, Coord x2, Coord y2) {
    Len dx = x1 - x2;
    Len dy = y1 - y2;
    return (Len)round(sqrt((double)dx * dx + (double)dy * dy));
}

class A_star {
    int n_;
    const vector<int>& adjHead_;
    const vector<int>& adjNext_;
    const vector<int>& adjTo_;
    const vector<int>& adjCost_;
    const vector<Coord>& x_;
    const vector<Coord>& y_;
    Coord tx_;
    Coord ty_;
    vector<Len> dist_;
    vector<Len> f_;
    vector<int> workset_;
    vector<char> closed_;
    PQ q_;
    PQ emptyPQ;

public:
    A_star(int n, const vector<int>& adjHead, const vector<int>& adjNext,
           const vector<int>& adjTo, const vector<int>& adjCost,
           const vector<Coord>& x, const vector<Coord>& y)
        : n_(n), adjHead_(adjHead), adjNext_(adjNext), adjTo_(adjTo),
          adjCost_(adjCost), x_(x), y_(y),
          dist_(n, INF), f_(n, INF), visited_(n, 0), closed_(n, 0)
    {
        workset_.reserve(n);
    }

    vector<char> visited_;

    void clear() {
      for (size_t i = 0; i < workset_.size(); ++i) {
        int v = workset_[i];
        dist_[v] = INF;
        f_[v] = INF;
        visited_[v] = 0;
        closed_[v] = 0;
      }
      workset_.clear();
      q_ = emptyPQ;
    }

    Len query(int s, int t) {
        if (s == t) {
            return 0;
        }

        clear();
        tx_ = x_[t];
        ty_ = y_[t];

        dist_[s] = 0;
        Len h = heuristic(x_[s], y_[s], tx_, ty_);
        f_[s] = h;
        q_.emplace(h, s);
        visited_[s] = 1;
        workset_.push_back(s);

        while (!q_.empty()) {
            pair<Len, int> cur = q_.top();
            q_.pop();
            int u = cur.second;
            Len fVal = cur.first;

            if (fVal > f_[u] || closed_[u]) {
                continue;
            }
            closed_[u] = 1;

            if (u == t) {
                return dist_[t];
            }

            Len d = dist_[u];
            for (int e = adjHead_[u]; e != -1; e = adjNext_[e]) {
                int v = adjTo_[e];
                Len newDist = d + adjCost_[e];
                if (newDist < dist_[v]) {
                    dist_[v] = newDist;
                    Len h = heuristic(x_[v], y_[v], tx_, ty_);
                    Len newF = newDist + h;
                    if (newF < f_[v]) {
                        f_[v] = newF;
                    }
                    q_.emplace(newF, v);
                    if (!visited_[v]) {
                        visited_[v] = 1;
                        workset_.push_back(v);
                    }
                }
            }
        }

        return -1;
    }
};

int main() {
    int n, m;
    scanf("%d%d", &n, &m);
    vector<Coord> x(n), y(n);
    for (int i = 0; i < n; ++i) {
        scanf("%lld%lld", &x[i], &y[i]);
    }

    vector<int> adjHead(n, -1);
    vector<int> adjNext(m);
    vector<int> adjTo(m);
    vector<int> adjCost(m);
    for (int i = 0; i < m; ++i) {
        int u, v, c;
        scanf("%d%d%d", &u, &v, &c);
        u--; v--;
        adjTo[i] = v;
        adjCost[i] = c;
        adjNext[i] = adjHead[u];
        adjHead[u] = i;
    }

    A_star a_star(n, adjHead, adjNext, adjTo, adjCost, x, y);

    int q;
    scanf("%d", &q);
    for (int i = 0; i < q; ++i) {
        int u, v;
        scanf("%d%d", &u, &v);
        printf("%lld\n", a_star.query(u - 1, v - 1));
    }
}
