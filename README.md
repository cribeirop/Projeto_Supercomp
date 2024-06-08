# Projeto - Vehicle Routing Problem - VRP

## Implementação e Análise de Algoritmos para o Problema de Roteamento de Veículos Capacitados (CVRP)

### Introdução
Este projeto aborda a implementação e análise de diferentes algoritmos para resolver o Problema de Roteamento de Veículos Capacitados (CVRP). O objetivo é comparar a eficiência e a eficácia de quatro abordagens distintas: Procura Global, Algoritmo de Clarke e Wright, Paralelização Local com OpenMP e Paralelização Global com MPI.

### Implementações

1. Procura Global (ex1.cpp)
A abordagem de busca global explora todas as possíveis rotas para encontrar a solução ótima. Utiliza uma busca recursiva para gerar todas as rotas viáveis, respeitando as restrições de capacidade e número de visitas. O algoritmo carrega o grafo de um arquivo especificado, implementa a busca recursiva para encontrar todas as possíveis rotas, seleciona a rota com o menor custo e registra o tempo de execução em um arquivo de texto.

2. Algoritmo de Clarke e Wright (ex2.cpp)
O algoritmo de Clarke e Wright utiliza uma heurística para resolver o CVRP, combinando rotas de forma a minimizar o custo total. O algoritmo carrega o grafo de um arquivo especificado, calcula os "savings" para cada par de nós, ordena os savings em ordem decrescente, combina as rotas com base nos savings calculados e registra o tempo de execução em um arquivo de texto.

3. Paralelização Local com OpenMP (ex3_local.cpp)
Esta abordagem paraleliza a execução do algoritmo de Clarke e Wright utilizando OpenMP para aproveitar os múltiplos núcleos de uma única máquina. O algoritmo carrega o grafo de um arquivo especificado, paraleliza o cálculo dos savings e a combinação de rotas utilizando OpenMP e registra o tempo de execução em um arquivo de texto.

4. Paralelização Global com MPI (ex3_global.cpp)
A paralelização global distribui a execução do algoritmo de Clarke e Wright entre múltiplas máquinas em um cluster utilizando MPI, e também faz uso de OpenMP para paralelizar localmente em cada nó. O algoritmo carrega o grafo de um arquivo especificado, divide o trabalho entre os processos MPI, paraleliza o cálculo dos savings e a combinação de rotas utilizando OpenMP em cada processo MPI e registra o tempo de execução em um arquivo de texto.

# Visualização dos Resultados

Utilizou-se Python e a biblioteca Matplotlib para gerar gráficos comparativos dos tempos de execução e dos custos das rotas encontradas por cada algoritmo. Os gráficos mostram a diferença de desempenho entre as abordagens, destacando a eficiência da paralelização local e global em comparação com a busca global e o algoritmo de Clarke e Wright.

### Conclusão

Este projeto demonstra a eficácia da paralelização em melhorar o desempenho de algoritmos para resolver o CVRP. A busca global, apesar de encontrar a solução ótima, é ineficiente para grafos maiores. O algoritmo de Clarke e Wright oferece uma solução heurística eficiente. A paralelização local com OpenMP e a paralelização global com MPI mostram melhorias significativas no tempo de execução, com a paralelização global sendo a mais eficiente para grandes grafos.