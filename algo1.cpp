#include <algorithm>
#include <iostream>
#include <optional>
#include <queue>
#include <set>
#include <string>
#include <utility>
#include <vector>

struct Edge {
    enum class State {
        Unknown,
        OK,
        Broken,
    };

    int id;
    int u;
    int v;
    double prob;
    State state = State::Unknown;
};

struct Vertex {
    double profit;
    std::vector<int> adj;
};

struct Expedition {
    Expedition(int n, double cost, int entrance)
        : vertices(n), visited(n, false), looked(n, false),
          dayCost(cost), entrance(entrance), current(entrance) {}

    void addEdge(int u, int v, double prob) {
        int id = edges.size();
        edges.push_back(Edge{id, u, v, prob});
        vertices[u].adj.push_back(id);
        vertices[v].adj.push_back(id);
    }

    void run() {
        visited[entrance] = true;

        while (true) {
            lookAtCurrentRoom();

            auto openEdge = chooseCandidateEdge(false);
            if (openEdge) {
                int edgeId = openEdge->first;
                int from = reachedEndpoint(edgeId);
                int to = unreachedEndpoint(edgeId);
                if (moveTo(from)) {
                    moveAcross(edgeId, to);
                }
                continue;
            }

            auto brokenEdge = chooseCandidateEdge(true);
            if (!brokenEdge || brokenEdge->second <= dayCost) {
                finish();
                return;
            }

            if (!clearAndVisit(brokenEdge->first)) {
                finish();
                return;
            }
        }
    }

    std::vector<Vertex> vertices;
    std::vector<Edge> edges;
    std::vector<bool> visited;
    std::vector<bool> looked;
    double dayCost;
    int entrance;
    int current;

    void lookAtCurrentRoom() {
        if (looked[current]) {
            return;
        }

        std::cout << "LOOK" << std::endl;
        int readCount = 0;
        int edgeCount = vertices[current].adj.size();

        while (readCount < edgeCount) {
            int inputId, status;
            std::cin >> inputId >> status;
            int id = inputId - 1;
            Edge& edge = edges[id];
            edge.state = (status == 1 ? Edge::State::OK : Edge::State::Broken);
            ++readCount;
        }

        looked[current] = true;
    }

    int otherEndpoint(int edgeId, int vertex) {
        Edge& edge = edges[edgeId];
        return (edge.u == vertex ? edge.v : edge.u);
    }

    int reachedEndpoint(int edgeId) {
        Edge& edge = edges[edgeId];
        return (visited[edge.u] ? edge.u : edge.v);
    }

    int unreachedEndpoint(int edgeId) {
        Edge& edge = edges[edgeId];
        return (visited[edge.u] ? edge.v : edge.u);
    }

    bool isTraversable(Edge& edge) {
        return edge.state == Edge::State::OK;
    }

    std::vector<int> shortestRoute(int from, int to) {
        std::vector<int> parentV(vertices.size(), -1);
        std::vector<int> parentE(vertices.size(), -1);
        std::queue<int> queue;
        parentV[from] = from;
        queue.push(from);

        while (!queue.empty() && parentV[to] == -1) {
            int vertex = queue.front();
            queue.pop();

            for (int edgeId : vertices[vertex].adj) {
                Edge& edge = edges[edgeId];
                if (!isTraversable(edge)) {
                    continue;
                }
                int next = otherEndpoint(edgeId, vertex);
                if (parentV[next] != -1) {
                    continue;
                }
                parentV[next] = vertex;
                parentE[next] = edgeId;
                queue.push(next);
            }
        }

        if (parentV[to] == -1) {
            return {};
        }

        std::vector<int> route;
        for (int vertex = to; vertex != from; vertex = parentV[vertex]) {
            route.push_back(parentE[vertex]);
        }
        std::reverse(route.begin(), route.end());
        return route;
    }

    bool moveAlong(std::vector<int>& route) {
        for (int edgeId : route) {
            int to = otherEndpoint(edgeId, current);
            if (!moveAcross(edgeId, to)) {
                return false;
            }

            lookAtCurrentRoom();
        }
        return true;
    }

    bool moveTo(int to) {
        if (current == to) {
            return true;
        }
        std::vector<int> route = shortestRoute(current, to);
        if (route.empty()) {
            return false;
        }
        return moveAlong(route);
    }

    bool moveAcross(int edgeId, int to) {
        std::cout << "MOVE " << (edgeId + 1) << std::endl;
        std::string response;
        std::cin >> response;

        if (response == "OK") {
            current = to;
            visited[current] = true;
            edges[edgeId].state = Edge::State::OK;
            return true;
        }

        edges[edgeId].state = Edge::State::Broken;
        return false;
    }

    double estimatedNetValueFrom(int start, int skipId) {
        std::vector<double> bestCost(vertices.size(), 1e9);

        std::set<std::vector<double>> queue;
        bestCost[start] = 0;
        std::vector<double> first;
        first.push_back(0);
        first.push_back(start);

        queue.insert(first);

        while (!queue.empty()) {
            std::vector<double> entry = *queue.begin();
            queue.erase(queue.begin());
            double entryCost = entry[0];
            int vertex = entry[1];
            if (entryCost != bestCost[vertex]) {
                continue;
            }

            for (int edgeId : vertices[vertex].adj) {
                if (edgeId == skipId) {
                    continue;
                }
                Edge& edge = edges[edgeId];
                double edgeCost = 0;
                if (edge.state == Edge::State::Unknown) {
                    edgeCost = (1 - edge.prob) * dayCost;
                } else if (edge.state == Edge::State::Broken) {
                    edgeCost = dayCost;
                }

                int next = otherEndpoint(edgeId, vertex);
                if (next != start && visited[next]) {
                    continue;
                }
                double candCost = entryCost + edgeCost;
                if (candCost >= bestCost[next]) {
                    continue;
                }

                bestCost[next] = candCost;
                std::vector<double> key;
                key.push_back(candCost);
                key.push_back(next);
                queue.insert(key);
            }
        }

        int n = vertices.size();
        double bestValue = 0;
        for (int i = 0; i < n; ++i) {
            if (visited[i] || bestCost[i] == 1e9) {
                continue;
            }
            double value = vertices[i].profit - bestCost[i];
            if (value > bestValue) {
                bestValue = value;
            }
        }
        return bestValue;
    }

    std::vector<int> distancesFromCurrent() {
        std::vector<int> distance(vertices.size(), 1e9);
        std::queue<int> queue;
        distance[current] = 0;
        queue.push(current);

        while (!queue.empty()) {
            int vertex = queue.front();
            queue.pop();
            for (int edgeId : vertices[vertex].adj) {
                Edge& edge = edges[edgeId];
                if (!isTraversable(edge)) {
                    continue;
                }
                int next = otherEndpoint(edgeId, vertex);
                if (distance[next] == 1e9) {
                    distance[next] = distance[vertex] + 1;
                    queue.push(next);
                }
            }
        }
        return distance;
    }

    std::optional<std::pair<int, double>> chooseCandidateEdge(bool broken) {
        std::vector<int> distance = distancesFromCurrent();
        std::optional<std::pair<int, double>> best;
        int bestDist = 1e9;
        Edge::State needState = (broken ? Edge::State::Broken : Edge::State::OK);

        for (Edge& edge : edges) {
            if (edge.state != needState) {
                continue;
            }

            int from = -1;
            int to = -1;
            if (visited[edge.u] && !visited[edge.v]) {
                from = edge.u;
                to = edge.v;
            } else if (visited[edge.v] && !visited[edge.u]) {
                from = edge.v;
                to = edge.u;
            } else {
                continue;
            }
            if (distance[from] == 1e9) {
                continue;
            }

            double candValue = estimatedNetValueFrom(to, edge.id);
            int candDist = distance[from];

            if (!best || candValue > best->second ||
                (candValue == best->second && candDist < bestDist) ||
                (candValue == best->second && candDist == bestDist && edge.id < best->first)) {
                best = std::make_pair(edge.id, candValue);
                bestDist = candDist;
            }
        }
        return best;
    }

    bool clearAndVisit(int edgeId) {
        int from = reachedEndpoint(edgeId);
        int to = unreachedEndpoint(edgeId);
        std::cout << "CLEAR " << (edgeId + 1) << std::endl;
        std::string response;
        std::cin >> response;
        if (response != "OK") {
            return false;
        }

        edges[edgeId].state = Edge::State::OK;
        current = entrance;

        if (!moveTo(from)) {
            return false;
        }
        return moveAcross(edgeId, to);
    }

    void finish() {
        std::cout << "END" << std::endl;
    }

};

int main() {
    int n, m, start;
    double cost;
    std::cin >> n >> m >> cost >> start;
    Expedition expedition(n, cost, start - 1);
    for (int i = 0; i < n; ++i) {
        double profit;
        std::cin >> profit;
        expedition.vertices[i].profit = profit;
    }

    for (int j = 0; j < m; ++j) {
        int u, v;
        double prob;
        std::cin >> u >> v >> prob;
        expedition.addEdge(u - 1, v - 1, prob);
    }

    expedition.run();
}
