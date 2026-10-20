#include <cstdio>
#include <cstring>
#include <vector>
#include <algorithm>
#include <limits>
#include <queue>
#include <iostream>
#include <memory>

using namespace std;

class Graph
{
public:
    using Distance = long long;

private:
    typedef vector<vector<pair<int, int>>> AdjList;
    typedef pair<Distance, int> PQElement;
    typedef priority_queue<PQElement, vector<PQElement>, greater<PQElement>> PQ;

    static constexpr Distance INF = numeric_limits<Distance>::max() / 4;

    static constexpr int EDGE_DIFF_WEIGHT = 14;
    static constexpr int CONTRACTED_NEIGHBORS_WEIGHT = 25;
    static constexpr int SHORTCUT_COUNT_WEIGHT = 10;
    static constexpr int LEVEL_WEIGHT = 10;
    static constexpr int MAX_SETTLED = 10;

    int N;
    AdjList outgoing_edges;
    AdjList incoming_edges;
    AdjList up_outgoing;
    AdjList up_incoming;
    vector<int> level;
    vector<int> rank;
    vector<Distance> dist_forward;
    vector<Distance> dist_backward;
    vector<int> query_workset;
    vector<Distance> witness_dist;
    vector<int> witness_workset;

    struct Shortcut {
        int from;
        int to;
        int cost;
    };

    void relax_step(PQ& q,
                    const AdjList& up_edges,
                    vector<Distance>& dist_own,
                    const vector<Distance>& dist_other,
                    Distance& estimate)
    {
        if (q.empty()) return;

        auto top = q.top();
        q.pop();
        Distance d = top.first;
        int u = top.second;

        if (d > dist_own[u] || d >= estimate) return;

        for (const auto& edge : up_edges[u]) {
            int v = edge.first;
            Distance new_d = d + edge.second;
            if (new_d < dist_own[v]) {
                if (dist_forward[v] == INF && dist_backward[v] == INF) {
                    query_workset.push_back(v);
                }
                dist_own[v] = new_d;
                q.emplace(new_d, v);
                if (dist_other[v] < INF && new_d + dist_other[v] < estimate) {
                    estimate = new_d + dist_other[v];
                }
            }
        }
    }

    void witness_search(int u, int v, Distance max_cost) {
        PQ pq;
        witness_dist[u] = 0;
        witness_workset.push_back(u);
        pq.emplace(0, u);

        int settled = 0;

        while (!pq.empty()) {
            auto top = pq.top();
            pq.pop();
            Distance d = top.first;
            int x = top.second;

            if (d > witness_dist[x]) continue;
            if (d > max_cost) break;
            if (++settled > MAX_SETTLED) break;

            for (const auto& edge : outgoing_edges[x]) {
                int next_node = edge.first;
                if (next_node == v || rank[next_node] != -1) continue;

                Distance new_dist = d + edge.second;
                if (new_dist < witness_dist[next_node]) {
                    witness_dist[next_node] = new_dist;
                    witness_workset.push_back(next_node);
                    pq.emplace(new_dist, next_node);
                }
            }
        }
    }

    void reset_witness() {
        for (int node : witness_workset) {
            witness_dist[node] = INF;
        }
        witness_workset.clear();
    }

    void set_n(int n) {
        N = n;
        outgoing_edges.resize(n);
        incoming_edges.resize(n);
        level.assign(n, 0);
        rank.assign(n, -1);
        dist_forward.assign(n, INF);
        dist_backward.assign(n, INF);
        witness_dist.assign(n, INF);
    }

    void add_edge_to_list(vector<pair<int, int>>& list, int w, int c) {
        for (auto& p : list) {
            if (p.first == w) {
                if (c < p.second) {
                    p.second = c;
                }
                return;
            }
        }
        list.emplace_back(w, c);
    }

    void add_directed_edge(int u, int v, int c) {
        add_edge_to_list(outgoing_edges[u], v, c);
        add_edge_to_list(incoming_edges[v], u, c);
    }

    int do_shortcut(int v, vector<Shortcut>& shortcuts, int& mylevel) {
        shortcuts.clear();

        vector<pair<int, int>> in_neighbors;
        for (const auto& edge : incoming_edges[v]) {
            int u = edge.first;
            int c = edge.second;
            if (rank[u] == -1 && u != v) {
                in_neighbors.emplace_back(u, c);
            }
        }

        vector<pair<int, int>> out_neighbors;
        for (const auto& edge : outgoing_edges[v]) {
            int w = edge.first;
            int c = edge.second;
            if (rank[w] == -1 && w != v) {
                out_neighbors.emplace_back(w, c);
            }
        }

        mylevel = 0;
        int contracted_neighbors = 0;
        for (const auto& edge : incoming_edges[v]) {
            int u = edge.first;
            if (rank[u] != -1) {
                mylevel = max(mylevel, level[u] + 1);
                contracted_neighbors++;
            }
        }
        for (const auto& edge : outgoing_edges[v]) {
            int w = edge.first;
            if (rank[w] != -1) {
                mylevel = max(mylevel, level[w] + 1);
                contracted_neighbors++;
            }
        }

        if (in_neighbors.empty() || out_neighbors.empty()) {
            int edge_diff = -(int)in_neighbors.size() - (int)out_neighbors.size();
            return edge_diff * EDGE_DIFF_WEIGHT
                 + contracted_neighbors * CONTRACTED_NEIGHBORS_WEIGHT
                 + mylevel * LEVEL_WEIGHT;
        }

        for (const auto& in_edge : in_neighbors) {
            int u = in_edge.first;
            int c_uv = in_edge.second;

            int max_cost = 0;
            for (const auto& out_edge : out_neighbors) {
                int w = out_edge.first;
                int c_vw = out_edge.second;
                if (u != w) {
                    max_cost = max(max_cost, c_uv + c_vw);
                }
            }

            if (max_cost == 0) continue;

            witness_search(u, v, max_cost);

            for (const auto& out_edge : out_neighbors) {
                int w = out_edge.first;
                int c_vw = out_edge.second;
                if (u == w) continue;

                if (witness_dist[w] > (Distance)c_uv + c_vw) {
                    shortcuts.push_back({u, w, c_uv + c_vw});
                }
            }

            reset_witness();
        }

        int edge_diff = (int)shortcuts.size() - (int)in_neighbors.size() - (int)out_neighbors.size();
        return edge_diff * EDGE_DIFF_WEIGHT
             + contracted_neighbors * CONTRACTED_NEIGHBORS_WEIGHT
             + (int)shortcuts.size() * SHORTCUT_COUNT_WEIGHT
             + mylevel * LEVEL_WEIGHT;
    }

    void build_upward_graphs() {
        up_outgoing.resize(N);
        up_incoming.resize(N);
        for (int u = 0; u < N; ++u) {
            for (const auto& edge : outgoing_edges[u]) {
                int v = edge.first;
                int c = edge.second;
                if (rank[u] < rank[v]) {
                    up_outgoing[u].emplace_back(v, c);
                }
            }
            for (const auto& edge : incoming_edges[u]) {
                int v = edge.first;
                int c = edge.second;
                if (rank[u] < rank[v]) {
                    up_incoming[u].emplace_back(v, c);
                }
            }
        }
    }

public:
    Graph() : N(0) {}

    bool read_input() {
        int n, m;
        if (scanf("%d %d", &n, &m) != 2) return false;
        set_n(n);
        for (int i = 0; i < m; ++i) {
            int u, v, c;
            if (scanf("%d %d %d", &u, &v, &c) != 3) return false;
            add_directed_edge(u - 1, v - 1, c);
        }
        return true;
    }

    void preprocess() {
        using QEntry = pair<int, int>;
        priority_queue<QEntry, vector<QEntry>, greater<QEntry>> queue;

        vector<Shortcut> shortcuts;
        for (int i = 0; i < N; ++i) {
            int mylevel = 0;
            int imp = do_shortcut(i, shortcuts, mylevel);
            queue.emplace(imp, i);
        }

        int current_rank = 0;
        while (!queue.empty()) {
            auto top = queue.top();
            queue.pop();
            int v = top.second;

            if (rank[v] != -1) continue;

            int mylevel = 0;
            int new_imp = do_shortcut(v, shortcuts, mylevel);

            if (!queue.empty() && new_imp > queue.top().first) {
                queue.emplace(new_imp, v);
                continue;
            }

            rank[v] = current_rank++;
            level[v] = mylevel;

            for (const auto& sc : shortcuts) {
                add_directed_edge(sc.from, sc.to, sc.cost);
            }
        }

        build_upward_graphs();
    }

    Distance query(int s, int t) {
        if (s == t) return 0;

        PQ q_f, q_b;

        dist_forward[s] = 0;
        dist_backward[t] = 0;
        query_workset.push_back(s);
        query_workset.push_back(t);
        q_f.emplace(0, s);
        q_b.emplace(0, t);

        Distance estimate = INF;

        while (!q_f.empty() || !q_b.empty()) {
            relax_step(q_f, up_outgoing, dist_forward,  dist_backward, estimate);
            relax_step(q_b, up_incoming, dist_backward, dist_forward,  estimate);
        }

        for (int v : query_workset) {
            dist_forward[v] = INF;
            dist_backward[v] = INF;
        }
        query_workset.clear();

        return estimate >= INF ? -1 : estimate;
    }

};

constexpr Graph::Distance Graph::INF;
static const Graph::Distance TSP_INF = numeric_limits<Graph::Distance>::max() / 4;

struct TspResult {
    Graph::Distance cost;
    vector<int> order;
};

TspResult solve_tsp(Graph& g, const vector<int>& cities) {
    int k = (int)cities.size();
    if (k == 0) return {0, {}};
    if (k == 1) return {0, {cities[0]}};

    vector<vector<Graph::Distance>> d(k, vector<Graph::Distance>(k, TSP_INF));
    for (int i = 0; i < k; ++i) {
        for (int j = 0; j < k; ++j) {
            if (i == j) {
                d[i][j] = 0;
            } else {
                Graph::Distance x = g.query(cities[i], cities[j]);
                d[i][j] = (x < 0) ? TSP_INF : x;
            }
        }
    }

    int m = k - 1;
    size_t full = (size_t)1 << m;
    vector<Graph::Distance> dp(full * m, TSP_INF);
    vector<signed char> parent(full * m, -1);

    for (int j = 0; j < m; ++j) {
        dp[((size_t)1 << j) * m + j] = d[0][j + 1];
    }

    for (size_t mask = 1; mask < full; ++mask) {
        for (int j = 0; j < m; ++j) {
            if (!(mask >> j & 1)) continue;
            Graph::Distance cur = dp[mask * m + j];
            if (cur >= TSP_INF) continue;

            for (int nxt = 0; nxt < m; ++nxt) {
                if (mask >> nxt & 1) continue;
                Graph::Distance step = d[j + 1][nxt + 1];
                if (step >= TSP_INF) continue;

                size_t nmask = mask | ((size_t)1 << nxt);
                Graph::Distance val = cur + step;
                if (val < dp[nmask * m + nxt]) {
                    dp[nmask * m + nxt] = val;
                    parent[nmask * m + nxt] = (signed char)j;
                }
            }
        }
    }

    Graph::Distance best = TSP_INF;
    int best_last = -1;
    for (int j = 0; j < m; ++j) {
        Graph::Distance back = d[j + 1][0];
        Graph::Distance path = dp[(full - 1) * m + j];
        if (back >= TSP_INF || path >= TSP_INF) continue;
        if (path + back < best) {
            best = path + back;
            best_last = j;
        }
    }
    if (best_last == -1) return {-1, {}};

    vector<int> seq;
    size_t mask = full - 1;
    int j = best_last;
    while (j != -1) {
        seq.push_back(j + 1);
        int pj = parent[mask * m + j];
        mask ^= (size_t)1 << j;
        j = pj;
    }
    reverse(seq.begin(), seq.end());

    TspResult res;
    res.cost = best;
    res.order.push_back(cities[0]);
    for (int idx : seq) res.order.push_back(cities[idx]);
    return res;
}

int main() {
    Graph g;
    if (!g.read_input()) return 1;
    g.preprocess();
    cout << "Ready" << endl;

    int t;
    if (scanf("%d", &t) != 1) return 0;

    for (int test = 0; test < t; ++test) {
        int k;
        if (scanf("%d", &k) != 1) return 1;

        vector<int> cities(k);
        for (int i = 0; i < k; ++i) {
            if (scanf("%d", &cities[i]) != 1) return 1;
            --cities[i];
        }

        TspResult res = solve_tsp(g, cities);
        printf("%lld\n", res.cost);
    }

    return 0;
}