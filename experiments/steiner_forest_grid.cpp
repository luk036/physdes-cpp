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
        for (int idx = 0; idx < size; ++idx) {
            parent[idx] = idx;
        }
    }

    /**
     * @brief Finds the representative of the set containing an element.
     * @param p The element to find.
     * @return The representative of the set.
     */
    int find(int node_p) {
        if (parent[node_p] != node_p) {
            parent[node_p] = find(parent[node_p]);
        }
        return parent[node_p];
    }

    /**
     * @brief Merges the sets containing two elements.
     * @param node_p The first element.
     * @param node_q The second element.
     * @return True if the sets were merged, false if they were already in the same set.
     */
    bool unionSets(int node_p, int node_q) {
        int root_p = find(node_p);
        int root_q = find(node_q);
        if (root_p == root_q) {
            return false;
        }
        if (rank[root_p] < rank[root_q]) {
            parent[root_p] = root_q;
        } else if (rank[root_p] > rank[root_q]) {
            parent[root_q] = root_p;
        } else {
            parent[root_q] = root_p;
            rank[root_p]++;
        }
        return true;
    }
};

/**
 * @struct Edge
 * @brief Represents an edge in the graph with a source node, destination node, and cost.
 */
struct Edge {
    int node_u;      ///< Source node identifier
    int node_v;      ///< Destination node identifier
    double cost;     ///< Cost of the edge

    Edge(int node_u, int node_v, double cost) : node_u(node_u), node_v(node_v), cost(cost) {}
};

/**
 * @struct Point
 * @brief Represents a point in a 2D grid with x and y coordinates.
 */
struct Point {
    int coord_x;     ///< X coordinate in the grid
    int coord_y;     ///< Y coordinate in the grid

    Point(int coord_x, int coord_y) : coord_x(coord_x), coord_y(coord_y) {}

    bool operator==(const Point& other) const { return coord_x == other.coord_x && coord_y == other.coord_y; }
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
            return hash<int>()(p.coord_x) ^ (hash<int>()(p.coord_y) << 1);
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
    int height;        ///< Height of the grid
    int width;         ///< Width of the grid
    int num_nodes;     ///< Total number of nodes (height * width)
    std::vector<Pair> pairs;  ///< Terminal pairs to connect

  public:
    /**
     * @brief Constructs a SteinerForestGrid solver.
     * @param height The height of the grid.
     * @param width The width of the grid.
     * @param terminalPairs A vector of pairs of points to be connected.
     */
    SteinerForestGrid(int height, int width, const std::vector<Pair>& terminalPairs)
        : height(height), width(width), num_nodes(height * width), pairs(terminalPairs) {}

    /**
     * @struct Result
     * @brief Stores the result of the Steiner Forest computation.
     */
    struct Result {
        std::vector<Edge> edges;            ///< Edges in the forest
        double totalCost;                   ///< Total cost of all edges
        std::unordered_set<int> sources;    ///< Source node identifiers
        std::unordered_set<int> terminals;  ///< Terminal node identifiers
        std::unordered_set<int> steinerNodes; ///< Steiner node identifiers
    };

    /**
     * @brief Computes the Steiner Forest.
     * @return A Result struct containing the edges of the forest, total cost, and node sets.
     */
    Result compute() {
        UnionFind uf(num_nodes);
        std::unordered_set<int> sources;
        std::unordered_set<int> terminals;
        std::unordered_map<int, std::vector<int>> pairDict;

        // Initialize sources, terminals, and pair dictionary
        for (const auto& pair : this->pairs) {
            int source_node = pair.source.coord_x * this->width + pair.source.coord_y;
            int target_node = pair.target.coord_x * this->width + pair.target.coord_y;
            sources.insert(source_node);
            terminals.insert(target_node);
            pairDict[source_node].push_back(target_node);
            pairDict[target_node].push_back(source_node);
        }

        std::unordered_set<int> allTerm;
        allTerm.insert(sources.begin(), sources.end());
        allTerm.insert(terminals.begin(), terminals.end());

        // Generate grid edges (horizontal and vertical only)
        std::vector<Edge> edges;
        for (int row_idx = 0; row_idx < this->height; ++row_idx) {
            for (int col_idx = 0; col_idx < this->width; ++col_idx) {
                int node = row_idx * this->width + col_idx;
                // Horizontal edges
                if (col_idx + 1 < this->width) {
                    edges.emplace_back(node, node + 1, 1.0);
                }
                // Vertical edges
                if (row_idx + 1 < this->height) {
                    edges.emplace_back(node, node + this->width, 1.0);
                }
            }
        }

        std::unordered_map<std::pair<int, int>, double, std::hash<std::pair<int, int>>> paid;
        std::vector<Edge> F;

        while (true) {
            // Compute term_root
            std::unordered_map<int, int> termRoot;
            for (int terminal : allTerm) {
                termRoot[terminal] = uf.find(terminal);
            }

            // Check feasibility
            bool feasible = true;
            for (const auto& [source, partners] : pairDict) {
                int root_source = termRoot[source];
                for (int terminal : partners) {
                    if (termRoot[terminal] != root_source) {
                        feasible = false;
                        break;
                    }
                }
                if (!feasible) break;
            }
            if (feasible) break;

            // Compute comp_terms
            std::unordered_map<int, std::unordered_set<int>> compTerms;
            for (int terminal : allTerm) {
                compTerms[termRoot[terminal]].insert(terminal);
            }

            // Compute active_comps
            std::unordered_set<int> activeComps;
            for (const auto& [root, terms] : compTerms) {
                bool isActive = false;
                for (int terminal : terms) {
                    for (int partner : pairDict[terminal]) {
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
                if (uf.find(edge.node_u) == uf.find(edge.node_v)) {
                    continue;
                }

                int root_u = uf.find(edge.node_u);
                int root_v = uf.find(edge.node_v);
                int num = 0;
                if (activeComps.count(root_u)) num++;
                if (activeComps.count(root_v)) num++;
                if (num == 0) continue;

                auto key = std::make_pair(std::min(edge.node_u, edge.node_v), std::max(edge.node_u, edge.node_v));
                double paidVal = paid[key];
                if (paidVal > edge.cost) continue;

                double deltaE = (edge.cost - paidVal) / num;
                if (deltaE < minDelta) {
                    minDelta = deltaE;
                    candidateEdges.clear();
                    candidateEdges.emplace_back(edge.node_u, edge.node_v, edge.cost, key);
                } else if (std::abs(deltaE - minDelta) < 1e-9) {
                    candidateEdges.emplace_back(edge.node_u, edge.node_v, edge.cost, key);
                }
            }

            if (minDelta == std::numeric_limits<double>::infinity()) {
                throw std::runtime_error("Graph is not connected or cannot connect pairs");
            }

            // Pick first candidate
            auto [chosenU, chosenV, chosenC, chosenKey] = candidateEdges[0];

            // Update paid for all eligible edges
            for (const auto& edge : edges) {
                if (uf.find(edge.node_u) == uf.find(edge.node_v)) continue;

                int root_u = uf.find(edge.node_u);
                int root_v = uf.find(edge.node_v);
                int num = 0;
                if (activeComps.count(root_u)) num++;
                if (activeComps.count(root_v)) num++;
                if (num == 0) continue;

                auto key = std::make_pair(std::min(edge.node_u, edge.node_v), std::max(edge.node_u, edge.node_v));
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
        for (int idx = FPruned.size() - 1; idx >= 0; --idx) {
            UnionFind tempUF(this->num_nodes);
            for (int jdx = 0; jdx < static_cast<int>(F.size()); ++jdx) {
                if (jdx != idx) {
                    tempUF.unionSets(F[jdx].node_u, F[jdx].node_v);
                }
            }

            bool connected = true;
            for (int source : sources) {
                for (int terminal : pairDict[source]) {
                    if (tempUF.find(source) != tempUF.find(terminal)) {
                        connected = false;
                        break;
                    }
                }
                if (!connected) break;
            }

            if (connected) {
                FPruned.erase(FPruned.begin() + idx);
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
            usedNodes.insert(edge.node_u);
            usedNodes.insert(edge.node_v);
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
void generateSVG(const SteinerForestGrid::Result& result, int height, int width,
                 const std::string& filename) {
    int cellSize = 50;
    int margin = 20;
    int svg_width = width * cellSize + 2 * margin;
    int svg_height = height * cellSize + 2 * margin;

    std::ofstream file(filename);
    file << "<svg width=\"" << svg_width << "\" height=\"" << svg_height
         << "\" xmlns=\"http://www.w3.org/2000/svg\">\n";

    // Grid lines horizontal
    for (int row_idx = 0; row_idx <= height; ++row_idx) {
        int coord_y = margin + row_idx * cellSize;
        file << "<line x1=\"" << margin << "\" y1=\"" << coord_y << "\" x2=\"" << (svg_width - margin)
             << "\" y2=\"" << coord_y << "\" stroke=\"gray\" stroke-width=\"1\"/>\n";
    }

    // Grid lines vertical
    for (int col_idx = 0; col_idx <= width; ++col_idx) {
        int coord_x = margin + col_idx * cellSize;
        file << "<line x1=\"" << coord_x << "\" y1=\"" << margin << "\" x2=\"" << coord_x << "\" y2=\""
             << (svg_height - margin) << "\" stroke=\"gray\" stroke-width=\"1\"/>\n";
    }

    // Nodes
    auto allTerm = result.sources;
    allTerm.insert(result.terminals.begin(), result.terminals.end());

    for (int row_idx = 0; row_idx < height; ++row_idx) {
        for (int col_idx = 0; col_idx < width; ++col_idx) {
            int cx = margin + col_idx * cellSize + cellSize / 2;
            int cy = margin + row_idx * cellSize + cellSize / 2;
            int node = row_idx * width + col_idx;

            std::string fill;
            int radius;

            if (result.sources.count(node)) {
                radius = 10;
                fill = "red";
            } else if (result.terminals.count(node)) {
                radius = 10;
                fill = "green";
            } else if (result.steinerNodes.count(node)) {
                radius = 7;
                fill = "blue";
            } else {
                radius = 5;
                fill = "black";
            }

            file << "<circle cx=\"" << cx << "\" cy=\"" << cy << "\" r=\"" << radius << "\" fill=\""
                 << fill << "\"/>\n";
            file << "<text x=\"" << cx << "\" y=\"" << (cy + 4)
                 << "\" font-size=\"10\" text-anchor=\"middle\">" << node << "</text>\n";
        }
    }

    // Selected edges
    for (const auto& edge : result.edges) {
        int ui = edge.node_u / width, uj = edge.node_u % width;
        int vi = edge.node_v / width, vj = edge.node_v % width;
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
    int height = 8;
    int width = 8;
    std::vector<Pair> pairs = {{Point(0, 0), Point(3, 2)},
                               {Point(0, 0), Point(0, 5)},
                               {Point(4, 4), Point(7, 5)},
                               {Point(4, 4), Point(5, 7)},
                               {Point(0, 1), Point(4, 1)}};

    SteinerForestGrid solver(height, width, pairs);
    auto result = solver.compute();

    generateSVG(result, height, width, "steiner_forest_grid.svg");

    std::cout << "SVG file 'steiner_forest_grid.svg' generated successfully.\n";
    std::cout << "Total cost: " << result.totalCost << "\n";
    std::cout << "Edges: ";
    for (const auto& edge : result.edges) {
        std::cout << "(" << edge.node_u << "," << edge.node_v << "," << edge.cost << ") ";
    }
    std::cout << "\n";

    return 0;
}