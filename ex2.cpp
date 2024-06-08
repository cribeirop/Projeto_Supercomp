
#include <iostream>
#include <vector>
#include <iomanip>
#include <fstream>
#include <tuple>
#include <utility>
#include <map>
#include <algorithm>
#include <chrono>

// const std::string graph = "grafo.txt";

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
            distance_matrix[to][from] = weight; // Assuming undirected graph
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
        std::vector<std::tuple<int, int, int>> savings;
        for (int i = 1; i < network.total_nodes; ++i) {
            for (int j = i + 1; j < network.total_nodes; ++j) {
                int s_ij = network.distance_matrix[0][i] + network.distance_matrix[0][j] - network.distance_matrix[i][j];
                savings.emplace_back(s_ij, i, j);
            }
        }

        std::sort(savings.begin(), savings.end(), std::greater<std::tuple<int, int, int>>());

        std::vector<std::vector<int>> routes(network.total_nodes);
        std::vector<int> route_demands(network.total_nodes, 0);
        for (int i = 1; i < network.total_nodes; ++i) {
            routes[i] = {0, i, 0};
            route_demands[i] = network.nodes[i].requirement;
        }

        for (const auto& [saving, i, j] : savings) {
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
        routes[route_i].pop_back(); // Remove the trailing 0 from route_i
        routes[route_i].insert(routes[route_i].end(), routes[route_j].begin() + 1, routes[route_j].end()); // Merge route_j into route_i
        route_demands[route_i] += route_demands[route_j];
        routes[route_j].clear(); // Clear route_j as it's now merged
    }

    void printSolution(const std::vector<std::vector<int>>& routes, const std::vector<int>& route_demands) {
        int total_cost = 0;
        std::cout << "Rota a ser seguida:" << std::endl;
        for (const auto& route : routes) {
            if (route.size() > 2) { // Valid route
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

int main(int argc, char* argv[]) {
    Network network;
    network.loadFromFile(argv[1]);

    ClarkeWright clarke_wright(network, 25, 10);

    // Iniciando a contagem de tempo
    auto start_time = std::chrono::high_resolution_clock::now();

    // Chamando o método para resolver o problema
    clarke_wright.solve();

    // Finalizando a contagem de tempo
    auto end_time = std::chrono::high_resolution_clock::now();

    // Calculando o tempo decorrido em segundos com precisão
    std::chrono::duration<double> duration = end_time - start_time;
    double execution_time = duration.count();

    // Salvando o tempo de execução em um arquivo de texto
    std::ofstream time_file("execution_time.txt", std::ios_base::app); // Abre o arquivo em modo de apêndice
    if (time_file.is_open()) {
        time_file << std::fixed << std::setprecision(6) << "Tempo de execução: " << execution_time << " segundos" << std::endl;
        time_file.close();
        std::cout << "Tempo de execução de " << execution_time << " salvo em 'execution_time.txt'" << std::endl;
    } else {
        std::cerr << "Erro ao abrir o arquivo para salvar o tempo de execução." << std::endl;
    }

    return 0;
}
