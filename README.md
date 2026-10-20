# BasketWise AI
Smart shopping assistant: picks the best basket within a budget and the shortest route inside the store.

## Run (inside this folder)
    g++ -O2 -std=c++17 engine.cpp -o engine
    node server.js
Planner: http://localhost:3100   |   Algorithm Lab: http://localhost:3100/lab
(`lab.html` also works by double-clicking it, no server needed.)
CLI: `engine plan 300 dairy,snacks` | `engine search ch` | `engine bench`

## Algorithms
- Sorting: custom Merge Sort (benchmarked against Quick Sort)
- Searching: Binary Search (name prefix)
- Shortest path: A* with Manhattan heuristic (benchmarked against Dijkstra)
- AI: utility scoring, Knapsack DP vs Greedy vs Genetic Algorithm, Nearest Neighbor + 2-opt route

## Data
data/products.txt: id|name|category|price|rating|x|y (39 products)  |  data/map.txt: '#' shelf, 'E' entrance
