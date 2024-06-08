#include <iostream>
#include <vector>
#include <fstream>
#include <tuple>
#include <utility>
#include <map>
#include <algorithm> 
#include <chrono>
#include <iomanip> // Para configurar a precisão ao escrever no arquivo

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

    void loadFromFile(const std::string& graph_file) {
        std::ifstream file(graph_file);
        if (!file.is_open()) {
            std::cerr << "Erro ao abrir o arquivo" << std::endl;
            return;
        }
        
        file >> total_nodes;
        for (int i = 1; i < total_nodes; ++i) {
            Node node;
            file >> node.id >> node.requirement;
            nodes.push_back(node);
        }
        
        file >> total_edges;
        int from, to, weight;
        for (int i = 0; i < total_edges; ++i) {
            file >> from >> to >> weight;
            adjacency_list[from].emplace_back(to, weight);
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

class RouteFinder {
public:
    Network network;
    int max_capacity;
    int max_visits;

    RouteFinder(Network net, int cap, int visits) : network(net), max_capacity(cap), max_visits(visits) {}

    void findRoutes(
        int cost, int current_node, int remaining_capacity, int visited_count,
        std::vector<int> visited_nodes,
        std::vector<std::tuple<std::vector<int>, int>>& solutions) {

        int temp_node = current_node;
        std::vector<int> temp_visited_nodes;
        auto original_requirements = network.nodes;

        for (const auto& edge : network.adjacency_list[temp_node]) {
            int next_node = edge.first;
            int travel_cost = edge.second;

            int node_requirement = network.getRequirement(next_node);

            if (next_node != 0 && (visited_count + 1 > max_visits || remaining_capacity - node_requirement <= 0)) {
                continue;
            }

            if (next_node == 0) {
                temp_visited_nodes = visited_nodes;
                temp_visited_nodes.push_back(next_node);

                
                if (network.totalRequirements() == 0) {
                    std::tuple<std::vector<int>, int> solution = std::make_tuple(temp_visited_nodes, cost + travel_cost);
                    solutions.push_back(solution);
                    return;
                }

                findRoutes(cost + travel_cost, next_node, max_capacity, 0, temp_visited_nodes, solutions);
            } else {
                if (node_requirement != 0 && !existsInVector(temp_visited_nodes, next_node)) {
                    temp_visited_nodes = visited_nodes;
                    temp_visited_nodes.push_back(next_node);
                    network.updateRequirement(next_node, 0);

                    findRoutes(cost + travel_cost, next_node, remaining_capacity - node_requirement, visited_count + 1, temp_visited_nodes, solutions);
                }
            }
            
            network.nodes = original_requirements;
        }
    }

    void discoverOptimalRoute() {
        int remaining_capacity = max_capacity;
        int total_cost = 0, starting_node = 0, visit_count = 0;
        std::vector<int> nodes_visited = {0};

        std::vector<std::tuple<std::vector<int>, int>> solutions;

        findRoutes(total_cost, starting_node, remaining_capacity, visit_count, nodes_visited, solutions);

        std::vector<int> optimal_route;
        int minimal_cost = 999999;

        for (const auto& solution : solutions) {
            const std::vector<int>& route = std::get<0>(solution);
            int cost = std::get<1>(solution);

            if (cost < minimal_cost) {
                optimal_route = route;
                minimal_cost = cost;
            }
        }

        std::cout << "Rota a ser seguida:" << std::endl;
        for (size_t i = 0; i < optimal_route.size(); ++i) {
            std::cout << "Passo " << i + 1 << ": Cidade " << optimal_route[i] << std::endl;
        }
        std::cout << "Total de passos: " << optimal_route.size() << std::endl;
        std::cout << std::endl;

        std::cout << "Detalhamento do custo:" << std::endl;
        std::cout << "Custo total para percorrer a rota ótima: " << minimal_cost << std::endl;
        std::cout << "Isso inclui os custos de viagem entre todas as cidades na rota." << std::endl;
    }
};

int main(int argc, char* argv[]) {
    Network network;
    network.loadFromFile(argv[1]);

    RouteFinder route_finder(network, 25, 10);

    // Iniciando a contagem de tempo
    auto start_time = std::chrono::high_resolution_clock::now();

    // Chamando o método para descobrir a rota ótima
    route_finder.discoverOptimalRoute();

    // Finalizando a contagem de tempo
    auto end_time = std::chrono::high_resolution_clock::now();

    // Calculando o tempo decorrido em segundos com precisão
    std::chrono::duration<double> duration = end_time - start_time;
    double execution_time = duration.count();

    // Salvando o tempo de execução em um arquivo de texto
    std::ofstream time_file("ex1_time.txt", std::ios_base::app); // Abre o arquivo em modo de apêndice
    if (time_file.is_open()) {
        time_file << std::fixed << std::setprecision(6) << "Tempo de execução: " << execution_time << " segundos" << std::endl;
        time_file.close();
        std::cout << "Tempo de execução de " << execution_time << " salvo em 'ex1_time.txt'" << std::endl;
    } else {
        std::cerr << "Erro ao abrir o arquivo para salvar o tempo de execução." << std::endl;
    }

    return 0;
}
