#include <cstdio>
#include <cassert>
#include <vector>
#include <queue>
#include <limits>
#include <utility>

using namespace std;

typedef vector<vector<vector<int>>> Adj;
typedef long long Len;
typedef priority_queue<pair<Len, int>, vector<pair<Len, int>>, greater<pair<Len, int>>> PQ;

const Len INF = numeric_limits<Len>::max() / 4;

class Bidijkstra {
    int n_;
    Adj& adj_;
    Adj& cost_;
    vector<Len> dist0_;
    vector<Len> dist1_;
    vector<int> workset_;
    vector<char> visited_;
    vector<char> done0_;
    vector<char> done1_;
    Len best_;
    PQ q0_;
    PQ q1_;
    PQ emptyPQ;

public:
    Bidijkstra(int n, Adj& adj, Adj& cost)
        : n_(n), adj_(adj), cost_(cost),
          dist0_(n, INF),
          dist1_(n, INF),
          visited_(n, 0),
          done0_(n, 0),
          done1_(n, 0)
    {
        workset_.reserve(n);
    }

    void clear() {
      for (size_t i = 0; i < workset_.size(); ++i) {
        int v = workset_[i];
        dist0_[v] = INF;
        dist1_[v] = INF;
        visited_[v] = 0;
        done0_[v] = 0;
        done1_[v] = 0;
      }
      workset_.clear();
      best_ = INF;
      q0_ = emptyPQ;
      q1_ = emptyPQ;
    }

     void visit0(int v, Len dist) {
        if (dist < dist0_[v]) {
            dist0_[v] = dist;
            q0_.emplace(dist, v);
            if (!visited_[v]) {
                visited_[v] = 1;
                workset_.push_back(v);
            }
            Len other = dist1_[v];
            if (other < INF && dist + other < best_) {
                best_ = dist + other;
            }
        }
    }

    void visit1(int v, Len dist) {
        if (dist < dist1_[v]) {
            dist1_[v] = dist;
            q1_.emplace(dist, v);
            if (!visited_[v]) {
                visited_[v] = 1;
                workset_.push_back(v);
            }
            Len other = dist0_[v];
            if (other < INF && dist + other < best_) {
                best_ = dist + other;
            }
        }
    }

    Len query(int s, int t) {
        if (s == t) {
          return 0;
        }

        clear();
        visit0(s, 0);
        visit1(t, 0);

        while (!q0_.empty() || !q1_.empty()) {
            Len top0 = q0_.empty() ? INF : q0_.top().first;
            Len top1 = q1_.empty() ? INF : q1_.top().first;

            if (best_ < INF && top0 + top1 >= best_) {
                break;
            }

            int side;
            if (q0_.empty()) side = 1;
            else if (q1_.empty()) side = 0;
            else side = (top0 <= top1) ? 0 : 1;

            if (side == 0) {
                pair<Len, int> cur = q0_.top();
                q0_.pop();
                int u = cur.second;
                Len d = cur.first;

                if (d > dist0_[u] || done0_[u]) {
                    continue;
                }
                done0_[u] = 1;

                const vector<int>& nb = adj_[0][u];
                const vector<int>& cs = cost_[0][u];
                for (size_t i = 0; i < nb.size(); ++i) {
                    visit0(nb[i], d + cs[i]);
                }
            } else {
                pair<Len, int> cur = q1_.top();
                q1_.pop();
                int u = cur.second;
                Len d = cur.first;

                if (d > dist1_[u] || done1_[u]) {
                    continue;
                }
                done1_[u] = 1;

                const vector<int>& nb = adj_[1][u];
                const vector<int>& cs = cost_[1][u];
                for (size_t i = 0; i < nb.size(); ++i) {
                    visit1(nb[i], d + cs[i]);
                }
            }
        }

        return best_ < INF ? best_ : -1;
    }
};

int main() {
    int n, m;
    if (scanf("%d%d", &n, &m) != 2) return 0;

    Adj adj(2, vector<vector<int>>(n));
    Adj cost(2, vector<vector<int>>(n));
    for (int i = 0; i < m; ++i) {
        int u, v, c;
        scanf("%d%d%d", &u, &v, &c);
        adj[0][u - 1].push_back(v - 1);
        cost[0][u - 1].push_back(c);
        adj[1][v - 1].push_back(u - 1);
        cost[1][v - 1].push_back(c);
    }

    Bidijkstra bidij(n, adj, cost);

    int t;
    if (scanf("%d", &t) != 1) return 0;
    for (int i = 0; i < t; ++i) {
        int u, v;
        scanf("%d%d", &u, &v);
        printf("%lld\n", bidij.query(u - 1, v - 1));
    }

    return 0;
}