# %%writefile mpic++ ex3_global.cpp -o ex3_global

#include <iostream>
#include <vector>
#include <fstream>
#include <tuple>
#include <utility>
#include <map>
#include <algorithm>
#include <mpi.h> 
#include <chrono>

const std::string graph = "grafo.txt";

class Node {
public:
    int id;
    int requirement;
};

bool existsInVector(const std::vector<int>& vec, int value) {
    return std::find(vec.begin(), vec.end(), value) != vec.end();
}

class Network {
public:
    int total_nodes;
    std::vector<Node> nodes;
    int total_edges;
    std::map<int, std::vector<std::pair<int, int>>> adjacency_list;
    std::vector<std::vector<int>> distance_matrix;

    void loadFromFile(const std::string& graph_file) {
        std::ifstream file(graph_file);
        if (!file.is_open()) {
            std::cerr << "Erro ao abrir o arquivo" << std::endl;
            return;
        }
        
        file >> total_nodes;
        nodes.resize(total_nodes);
        distance_matrix.resize(total_nodes, std::vector<int>(total_nodes, 0));
        
        for (int i = 1; i < total_nodes; ++i) {
            file >> nodes[i].id >> nodes[i].requirement;
        }
        
        file >> total_edges;
        int from, to, weight;
        for (int i = 0; i < total_edges; ++i) {
            file >> from >> to >> weight;
            adjacency_list[from].emplace_back(to, weight);
            distance_matrix[from][to] = weight;
            distance_matrix[to][from] = weight; 
        }

        file.close();
        
        std::cout << "Informações das Cidades:" << std::endl;
        for (const auto& node : nodes) {
            std::cout << "Cidade ID: " << node.id << " | Necessidade: " << node.requirement << std::endl;
        }

        std::cout << std::endl;
        
        std::cout << "Mapa de Rotas:" << std::endl;
        for (const auto& edge_list : adjacency_list) {
            std::cout << "Origem " << edge_list.first << " -> ";
            for (const auto& edge : edge_list.second) {
                std::cout << "Destino " << edge.first << " (Custo: " << edge.second << ") ";
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }

    int getRequirement(int node_id) const {
        for (const auto& node : nodes) {
            if (node.id == node_id) {
                return node.requirement;
            }
        }
        return 0;
    }

    void updateRequirement(int node_id, int new_requirement) {
        for (auto& node : nodes) {
            if (node.id == node_id) {
                node.requirement = new_requirement;
                return;
            }
        }
    }

    int totalRequirements() const {
        int total = 0;
        for (const auto& node : nodes) {
            total += node.requirement;
        }
        return total;
    }
};

class ClarkeWright {
public:
    Network network;
    int max_capacity;
    int max_visits;

    ClarkeWright(Network net, int cap, int visits) : network(net), max_capacity(cap), max_visits(visits) {}

    void solve() {
        int rank, size;
        MPI_Comm_rank(MPI_COMM_WORLD, &rank); 
        MPI_Comm_size(MPI_COMM_WORLD, &size); 

        std::vector<std::tuple<int, int, int>> savings;
        for (int i = 1 + rank; i < network.total_nodes; i += size) { 
            for (int j = i + 1; j < network.total_nodes; ++j) {
                int s_ij = network.distance_matrix[0][i] + network.distance_matrix[0][j] - network.distance_matrix[i][j];
                savings.emplace_back(s_ij, i, j);
            }
        }

        
        std::vector<std::tuple<int, int, int>> combined_savings;
        MPI_Allreduce(savings.data(), combined_savings.data(), savings.size(), MPI_INT, MPI_SUM, MPI_COMM_WORLD);

        std::sort(combined_savings.begin(), combined_savings.end(), [](const auto& a, const auto& b) { 
            return std::get<0>(a) > std::get<0>(b);
        });

        std::vector<std::vector<int>> routes(network.total_nodes);
        std::vector<int> route_demands(network.total_nodes, 0);
        #pragma omp parallel for schedule(dynamic) 
        for (int i = 1; i < network.total_nodes; ++i) {
            routes[i] = {0, i, 0};
            route_demands[i] = network.nodes[i].requirement;
        }

        #pragma omp parallel for schedule(dynamic) 
        for (size_t idx = 0; idx < savings.size(); ++idx) {
            const auto& [saving, i, j] = savings[idx];
            int route_i = findRoute(routes, i);
            int route_j = findRoute(routes, j);

            if (route_i != route_j && route_demands[route_i] + route_demands[route_j] <= max_capacity && routes[route_i].size() + routes[route_j].size() - 2 <= max_visits) {
                mergeRoutes(routes, route_i, route_j, route_demands);
            }
        }

        printSolution(routes, route_demands);

    }

private:
    int findRoute(const std::vector<std::vector<int>>& routes, int node) {
        for (int r = 1; r < routes.size(); ++r) {
            if (std::find(routes[r].begin(), routes[r].end(), node) != routes[r].end()) {
                return r;
            }
        }
        return -1;
    }

    void mergeRoutes(std::vector<std::vector<int>>& routes, int route_i, int route_j, std::vector<int>& route_demands) {
        routes[route_i].pop_back(); 
        routes[route_i].insert(routes[route_i].end(), routes[route_j].begin() + 1, routes[route_j].end()); 
        route_demands[route_i] += route_demands[route_j];
        routes[route_j].clear(); 
    }

    void printSolution(const std::vector<std::vector<int>>& routes, const std::vector<int>& route_demands) {
        int total_cost = 0;
        std::cout << "Rota a ser seguida:" << std::endl;
        for (const auto& route : routes) {
            if (route.size() > 2) { 
                int route_cost = 0;
                for (size_t i = 0; i < route.size() - 1; ++i) {
                    route_cost += network.distance_matrix[route[i]][route[i + 1]];
                }
                total_cost += route_cost;

                std::cout << "Rota: ";
                for (int node : route) {
                    std::cout << node << " ";
                }
                std::cout << "| Custo da rota: " << route_cost << std::endl;
            }
        }
        std::cout << "Total de passos: " << routes.size() << std::endl;
        std::cout << std::endl;

        std::cout << "Detalhamento do custo:" << std::endl;
        std::cout << "Custo total para percorrer a rota ótima: " << total_cost << std::endl;
        std::cout << "Isso inclui os custos de viagem entre todas as cidades na rota." << std::endl;
    }
};

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv); 

    std::vector<std::string> graph_files = {"grafos/grafo_03.txt", "grafos/grafo_05.txt", "grafos/grafo_07.txt", "grafos/grafo_09.txt", "grafos/grafo_11.txt"};
    
    for (const auto& file : graph_files) {
        Network network;
        network.loadFromFile(file);

        auto start = std::chrono::high_resolution_clock::now();

        ClarkeWright clarke_wright(network, 25, 10);
        clarke_wright.solve();

        auto end = std::chrono::high_resolution_clock::now();

        std::chrono::duration<double> duration = end - start;
        double seconds = duration.count();

        std::ofstream output_file("execution_time_" + file + ".txt");

        std::cout << "Tempo de " << file <<": " << seconds << std::endl;

        output_file << "Tempo de execução de " << file <<": " << seconds << " segundos" << std::endl;
        output_file.close();
    }

    MPI_Finalize(); 

    return 0;
}
