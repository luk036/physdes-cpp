#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

/**
 * @class UnionFind
 * @brief Implements the Union-Find data structure for disjoint set union operations.
 *
 * This class is used to keep track of a set of elements partitioned into a
 * number of disjoint (non-overlapping) subsets. It provides two main operations:
 * `find`, which determines which subset an element is in, and `unionSets`,
 * which joins two subsets into a single subset. This implementation uses both path
 * compression and union by rank for optimization.
 */
class UnionFind {
  private:
    std::vector<int> parent;
    std::vector<int> rank;

  public:
    /**
     * @brief Constructs a new UnionFind object.
     * @param size The initial number of elements in the set.
     */
    UnionFind(int size) : parent(size), rank(size, 0) {
        for (int i = 0; i < size; ++i) {
            parent[i] = i;
        }
    }

    /**
     * @brief Finds the representative of the set containing an element.
     * @param p The element to find.
     * @return The representative of the set.
     */
    int find(int p) {
        if (parent[p] != p) {
            parent[p] = find(parent[p]);
        }
        return parent[p];
    }

    /**
     * @brief Merges the sets containing two elements.
     * @param p The first element.
     * @param q The second element.
     * @return True if the sets were merged, false if they were already in the same set.
     */
    bool unionSets(int p, int q) {
        int pp = find(p);
        int pq = find(q);
        if (pp == pq) {
            return false;
        }
        if (rank[pp] < rank[pq]) {
            parent[pp] = pq;
        } else if (rank[pp] > rank[pq]) {
            parent[pq] = pp;
        } else {
            parent[pq] = pp;
            rank[pp]++;
        }
        return true;
    }
};

/**
 * @struct Edge
 * @brief Represents an edge in the graph with a source, destination, and cost.
 */
struct Edge {
    int u;
    int v;
    double cost;

    Edge(int u, int v, double cost) : u(u), v(v), cost(cost) {}
};

/**
 * @struct Point
 * @brief Represents a point in a 2D grid with x and y coordinates.
 */
struct Point {
    int x;
    int y;

    Point(int x, int y) : x(x), y(y) {}

    bool operator==(const Point& other) const { return x == other.x && y == other.y; }
};

/**
 * @struct Pair
 * @brief Represents a pair of terminals (source and target) to be connected.
 */
struct Pair {
    Point source;
    Point target;

    Pair(Point source, Point target) : source(source), target(target) {}
};

namespace std {
    /**
     * @brief Hash function for the Point struct.
     */
    template <> struct hash<Point> {
        size_t operator()(const Point& p) const {
            return hash<int>()(p.x) ^ (hash<int>()(p.y) << 1);
        }
    };

    /**
     * @brief Hash function for std::pair<int, int>.
     */
    template <> struct hash<std::pair<int, int>> {
        size_t operator()(const std::pair<int, int>& p) const {
            return hash<int>()(p.first) ^ (hash<int>()(p.second) << 1);
        }
    };
}  // namespace std

/**
 * @class SteinerForestGrid
 * @brief Computes an approximate Steiner Forest on a grid graph.
 *
 * This class implements an algorithm to find a low-cost subgraph that connects
 * specified pairs of terminal nodes on a grid. The algorithm is based on a
 * primal-dual approach and iteratively builds a solution.
 */
class SteinerForestGrid {
  private:
    int h, w, n;
    std::vector<Pair> pairs;

  public:
    /**
     * @brief Constructs a SteinerForestGrid solver.
     * @param height The height of the grid.
     * @param width The width of the grid.
     * @param terminalPairs A vector of pairs of points to be connected.
     */
    SteinerForestGrid(int height, int width, const std::vector<Pair>& terminalPairs)
        : h(height), w(width), n(height * width), pairs(terminalPairs) {}

    /**
     * @struct Result
     * @brief Stores the result of the Steiner Forest computation.
     */
    struct Result {
        std::vector<Edge> edges;
        double totalCost;
        std::unordered_set<int> sources;
        std::unordered_set<int> terminals;
        std::unordered_set<int> steinerNodes;
    };

    /**
     * @brief Computes the Steiner Forest.
     * @return A Result struct containing the edges of the forest, total cost, and node sets.
     */
    Result compute() {
        UnionFind uf(n);
        std::unordered_set<int> sources;
        std::unordered_set<int> terminals;
        std::unordered_map<int, std::vector<int>> pairDict;

        // Initialize sources, terminals, and pair dictionary
        for (const auto& pair : pairs) {
            int s = pair.source.x * w + pair.source.y;
            int t = pair.target.x * w + pair.target.y;
            sources.insert(s);
            terminals.insert(t);
            pairDict[s].push_back(t);
            pairDict[t].push_back(s);
        }

        std::unordered_set<int> allTerm;
        allTerm.insert(sources.begin(), sources.end());
        allTerm.insert(terminals.begin(), terminals.end());

        // Generate grid edges (horizontal and vertical only)
        std::vector<Edge> edges;
        for (int i = 0; i < h; ++i) {
            for (int j = 0; j < w; ++j) {
                int node = i * w + j;
                // Horizontal edges
                if (j + 1 < w) {
                    edges.emplace_back(node, node + 1, 1.0);
                }
                // Vertical edges
                if (i + 1 < h) {
                    edges.emplace_back(node, node + w, 1.0);
                }
            }
        }

        std::unordered_map<std::pair<int, int>, double, std::hash<std::pair<int, int>>> paid;
        std::vector<Edge> F;

        while (true) {
            // Compute term_root
            std::unordered_map<int, int> termRoot;
            for (int t : allTerm) {
                termRoot[t] = uf.find(t);
            }

            // Check feasibility
            bool feasible = true;
            for (const auto& [s, partners] : pairDict) {
                int rs = termRoot[s];
                for (int t : partners) {
                    if (termRoot[t] != rs) {
                        feasible = false;
                        break;
                    }
                }
                if (!feasible) break;
            }
            if (feasible) break;

            // Compute comp_terms
            std::unordered_map<int, std::unordered_set<int>> compTerms;
            for (int t : allTerm) {
                compTerms[termRoot[t]].insert(t);
            }

            // Compute active_comps
            std::unordered_set<int> activeComps;
            for (const auto& [root, terms] : compTerms) {
                bool isActive = false;
                for (int t : terms) {
                    for (int partner : pairDict[t]) {
                        if (termRoot[partner] != root) {
                            isActive = true;
                            break;
                        }
                    }
                    if (isActive) break;
                }
                if (isActive) {
                    activeComps.insert(root);
                }
            }

            // Find min_delta and candidate edges
            double minDelta = std::numeric_limits<double>::infinity();
            std::vector<std::tuple<int, int, double, std::pair<int, int>>> candidateEdges;

            for (const auto& edge : edges) {
                if (uf.find(edge.u) == uf.find(edge.v)) {
                    continue;
                }

                int ru = uf.find(edge.u);
                int rv = uf.find(edge.v);
                int num = 0;
                if (activeComps.count(ru)) num++;
                if (activeComps.count(rv)) num++;
                if (num == 0) continue;

                auto key = std::make_pair(std::min(edge.u, edge.v), std::max(edge.u, edge.v));
                double paidVal = paid[key];
                if (paidVal > edge.cost) continue;

                double deltaE = (edge.cost - paidVal) / num;
                if (deltaE < minDelta) {
                    minDelta = deltaE;
                    candidateEdges.clear();
                    candidateEdges.emplace_back(edge.u, edge.v, edge.cost, key);
                } else if (std::abs(deltaE - minDelta) < 1e-9) {
                    candidateEdges.emplace_back(edge.u, edge.v, edge.cost, key);
                }
            }

            if (minDelta == std::numeric_limits<double>::infinity()) {
                throw std::runtime_error("Graph is not connected or cannot connect pairs");
            }

            // Pick first candidate
            auto [chosenU, chosenV, chosenC, chosenKey] = candidateEdges[0];

            // Update paid for all eligible edges
            for (const auto& edge : edges) {
                if (uf.find(edge.u) == uf.find(edge.v)) continue;

                int ru = uf.find(edge.u);
                int rv = uf.find(edge.v);
                int num = 0;
                if (activeComps.count(ru)) num++;
                if (activeComps.count(rv)) num++;
                if (num == 0) continue;

                auto key = std::make_pair(std::min(edge.u, edge.v), std::max(edge.u, edge.v));
                paid[key] += minDelta * num;
                if (paid[key] > edge.cost + 1e-6) {
                    paid[key] = edge.cost;
                }
            }

            // Add chosen edge if not overpaid
            if (paid[chosenKey] >= chosenC - 1e-6) {
                F.emplace_back(chosenU, chosenV, chosenC);
                uf.unionSets(chosenU, chosenV);
            }
        }

        // Reverse delete to prune the forest
        std::vector<Edge> FPruned = F;
        for (int i = FPruned.size() - 1; i >= 0; --i) {
            UnionFind tempUF(n);
            for (int j = 0; j < static_cast<int>(F.size()); ++j) {
                if (j != i) {
                    tempUF.unionSets(F[j].u, F[j].v);
                }
            }

            bool connected = true;
            for (int s : sources) {
                for (int t : pairDict[s]) {
                    if (tempUF.find(s) != tempUF.find(t)) {
                        connected = false;
                        break;
                    }
                }
                if (!connected) break;
            }

            if (connected) {
                FPruned.erase(FPruned.begin() + i);
            }
        }

        // Compute final cost
        double totalCost = 0.0;
        for (const auto& edge : FPruned) {
            totalCost += edge.cost;
        }

        // Identify Steiner nodes
        std::unordered_set<int> usedNodes;
        for (const auto& edge : FPruned) {
            usedNodes.insert(edge.u);
            usedNodes.insert(edge.v);
        }

        std::unordered_set<int> steinerNodes;
        for (int node : usedNodes) {
            if (!allTerm.count(node)) {
                steinerNodes.insert(node);
            }
        }

        return {FPruned, totalCost, sources, terminals, steinerNodes};
    }
};

/**
 * @brief Generates an SVG visualization of the Steiner Forest.
 * @param result The result from the SteinerForestGrid computation.
 * @param h The height of the grid.
 * @param w The width of the grid.
 * @param filename The name of the SVG file to generate.
 */
void generateSVG(const SteinerForestGrid::Result& result, int h, int w,
                 const std::string& filename) {
    int cellSize = 50;
    int margin = 20;
    int width = w * cellSize + 2 * margin;
    int height = h * cellSize + 2 * margin;

    std::ofstream file(filename);
    file << "<svg width=\"" << width << "\" height=\"" << height
         << "\" xmlns=\"http://www.w3.org/2000/svg\">\n";

    // Grid lines horizontal
    for (int i = 0; i <= h; ++i) {
        int y = margin + i * cellSize;
        file << "<line x1=\"" << margin << "\" y1=\"" << y << "\" x2=\"" << (width - margin)
             << "\" y2=\"" << y << "\" stroke=\"gray\" stroke-width=\"1\"/>\n";
    }

    // Grid lines vertical
    for (int j = 0; j <= w; ++j) {
        int x = margin + j * cellSize;
        file << "<line x1=\"" << x << "\" y1=\"" << margin << "\" x2=\"" << x << "\" y2=\""
             << (height - margin) << "\" stroke=\"gray\" stroke-width=\"1\"/>\n";
    }

    // Nodes
    auto allTerm = result.sources;
    allTerm.insert(result.terminals.begin(), result.terminals.end());

    for (int i = 0; i < h; ++i) {
        for (int j = 0; j < w; ++j) {
            int cx = margin + j * cellSize + cellSize / 2;
            int cy = margin + i * cellSize + cellSize / 2;
            int node = i * w + j;

            std::string fill;
            int r;

            if (result.sources.count(node)) {
                r = 10;
                fill = "red";
            } else if (result.terminals.count(node)) {
                r = 10;
                fill = "green";
            } else if (result.steinerNodes.count(node)) {
                r = 7;
                fill = "blue";
            } else {
                r = 5;
                fill = "black";
            }

            file << "<circle cx=\"" << cx << "\" cy=\"" << cy << "\" r=\"" << r << "\" fill=\""
                 << fill << "\"/>\n";
            file << "<text x=\"" << cx << "\" y=\"" << (cy + 4)
                 << "\" font-size=\"10\" text-anchor=\"middle\">" << node << "</text>\n";
        }
    }

    // Selected edges
    for (const auto& edge : result.edges) {
        int ui = edge.u / w, uj = edge.u % w;
        int vi = edge.v / w, vj = edge.v % w;
        int ux = margin + uj * cellSize + cellSize / 2;
        int uy = margin + ui * cellSize + cellSize / 2;
        int vx = margin + vj * cellSize + cellSize / 2;
        int vy = margin + vi * cellSize + cellSize / 2;

        file << "<line x1=\"" << ux << "\" y1=\"" << uy << "\" x2=\"" << vx << "\" y2=\"" << vy
             << "\" stroke=\"orange\" stroke-width=\"5\" opacity=\"0.5\"/>\n";
    }

    file << "</svg>\n";
    file.close();
}

/**
 * @brief Main entry point of the program.
 *
 * Initializes a Steiner Forest problem on a grid, computes the solution,
 * generates an SVG visualization, and prints the results to the console.
 * @return 0 on success.
 */
int main() {
    int h = 8;
    int w = 8;
    std::vector<Pair> pairs = {{Point(0, 0), Point(3, 2)},
                               {Point(0, 0), Point(0, 5)},
                               {Point(4, 4), Point(7, 5)},
                               {Point(4, 4), Point(5, 7)},
                               {Point(0, 1), Point(4, 1)}};

    SteinerForestGrid solver(h, w, pairs);
    auto result = solver.compute();

    generateSVG(result, h, w, "steiner_forest_grid.svg");

    std::cout << "SVG file 'steiner_forest_grid.svg' generated successfully.\n";
    std::cout << "Total cost: " << result.totalCost << "\n";
    std::cout << "Edges: ";
    for (const auto& edge : result.edges) {
        std::cout << "(" << edge.u << "," << edge.v << "," << edge.cost << ") ";
    }
    std::cout << "\n";

    return 0;
}