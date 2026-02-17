#include "global.h"

/**
 * @brief Get all neighbors by following page chain
 */
vector<vector<int>> Graph::getNeighbors(int nodeId)
{
    logger.log("Graph::getNeighbors");
    vector<vector<int>> neighbors;

    int pageNum = 0;
    string currentFile = getPageFileName(nodeId, pageNum);

    // If first page doesn't exist, node has no edges
    if (!fileExists(currentFile))
    {
        return neighbors;
    }

    while (true)
    {
        ifstream fin(currentFile, ios::in);
        if (!fin.is_open())
            break;

        // Read header
        int rowCount, nextPage;
        fin >> rowCount >> nextPage;

        // Read edges
        for (int r = 0; r < rowCount; r++)
        {
            vector<int> row(this->storedColCount);
            for (int c = 0; c < this->storedColCount; c++)
            {
                fin >> row[c];
            }
            neighbors.push_back(row);
        }
        fin.close();

        // Follow chain
        if (nextPage == -1)
            break;
        pageNum = nextPage;
        currentFile = getPageFileName(nodeId, pageNum);
    }

    return neighbors;
}

static string uniformSignature(const unordered_map<string, int> &uniformValues)
{
    vector<pair<string, int>> items;
    items.reserve(uniformValues.size());
    for (const auto &kv : uniformValues)
    {
        items.emplace_back(kv.first, kv.second);
    }
    sort(items.begin(), items.end(), [](const auto &a, const auto &b) {
        return a.first < b.first;
    });
    string sig;
    for (const auto &kv : items)
    {
        sig += kv.first;
        sig.push_back('=');
        sig += to_string(kv.second);
        sig.push_back(';');
    }
    return sig;
}

pair<vector<vector<int>>, int> readfile(string filename, int columnCount) {
    vector<vector<int>> edges;
    int nextpage =-1;
     ifstream fin(filename, ios::in);
        if (!fin.is_open())
            return {edges,nextpage};

        // Read header
        int rowCount, nextPage;
        fin >> rowCount >> nextPage;

        // Read edges
        for (int r = 0; r < rowCount; r++)
        {
            vector<int> row(columnCount);
            for (int c = 0; c < columnCount; c++)
            {
                fin >> row[c];
            }
            edges.push_back(row);
        }
        fin.close();

    return {edges,nextPage};
}



/**
 * @brief Find path with conditions using modified Dijkstra
 * Returns {cost, {path_nodes, edges_used}} or {-1, {{}, {}}} if no path
 */

pair<long long, pair<vector<int>, vector<vector<int>>>> Graph::findPath(
    int src, int dst, vector<PathCondition> &conditions)
{
    logger.log("Graph::findPath");

    // cout << "node: " << src << ", degree: "<< this->findDegree(src) << endl;
    // cout << "node: " << dst << ", degree: "<< this->findDegree(dst) << endl;

    // cout << 1<< endl;

    // State: {distance, nodeId, path, edges, uniformValues}
    // Using string to encode uniformValues for priority queue

    struct State
    {
        long long dist;
        int node;
        vector<int> path;
        vector<vector<int>> edges;
        unordered_map<string, int> uniformValues;

        bool operator>(const State &other) const
        {
            return dist > other.dist;
        }
    };
    // cout << 1<< endl;

    priority_queue<State, vector<State>, greater<State>> pq;

    // Track best distance to each node (may have multiple valid paths with different uniform values)
    unordered_map<string, long long> bestDist;

    // Check source node conditions
    State startState;
    startState.dist = 0;
    startState.node = src;
    startState.path = {src};

    if (!checkNodeConditions(src, conditions, startState.uniformValues))
    {
        return {-1, {{}, {}}}; // Source doesn't satisfy conditions
    }

    pq.push(startState);

    while (!pq.empty())
    {
        State curr = pq.top();
        pq.pop();

        if (curr.node == dst)
        {
            return {curr.dist, {curr.path, curr.edges}};
        }

        // Skip if we've found a better path to this node with the same uniform constraints
        string stateKey = to_string(curr.node) + "|" + uniformSignature(curr.uniformValues);
        if (bestDist.count(stateKey) && curr.dist > bestDist[stateKey])
        {
            continue;
        }
        bestDist[stateKey] = curr.dist;

        // STREAMING: Process edges page-by-page directly from disk to save memory
        int pageNum = 0;
        string currentFile = getPageFileName(curr.node, pageNum);

        
        while (true)
            {
                if (!fileExists(currentFile)) break;
                auto file_contents = readfile(currentFile, this->storedColCount);
                int nextPage = file_contents.second;
                auto edges = file_contents.first;

                for (auto &edge : edges)
                {
                    // Read one edge at a time
                    

                    int v = edge[this->destColumnIndex];
                    int w = edge[this->weightColumnIndex];

                    if (w == -1)
                        continue; // Skip dummy edges

                    // Copy uniform values for this branch
                    unordered_map<string, int> newUniform = curr.uniformValues;

                    // Check edge conditions
                    if (!checkEdgeConditions(edge, conditions, newUniform))
                    {
                        continue;
                    }

                    // Check destination node conditions
                    if (!checkNodeConditions(v, conditions, newUniform))
                    {
                        continue;
                    }

                    State newState;
                    newState.dist = curr.dist + w;
                    newState.node = v;
                    newState.path = curr.path;
                    newState.path.push_back(v);
                    newState.edges = curr.edges;

                    // Reconstruct full edge for storage: [src, dst, weight, attrs...]
                    vector<int> fullEdge;
                    fullEdge.push_back(curr.node);                             // src
                    fullEdge.insert(fullEdge.end(), edge.begin(), edge.end()); // dst, weight, attrs
                    newState.edges.push_back(fullEdge);

                    newState.uniformValues = newUniform;

                    pq.push(newState);
                }

                if (nextPage == -1)
                    break;
                pageNum = nextPage;
                currentFile = getPageFileName(curr.node, pageNum);
            
        }
    }

    return {-1, {{}, {}}};
}

/**
 * @brief Save path as a new graph (nodes and edges CSV files)
 */
void Graph::savePathAsGraph(const string &resultName, vector<int> &path,
                            vector<vector<int>> &edges)
{
    logger.log("Graph::savePathAsGraph");

    char dirChar = this->isDirected ? 'D' : 'U';

    // Write nodes file
    string nodesFileName = "../data/" + resultName + "_Nodes_" + dirChar + ".csv";
    ofstream nodesOut(nodesFileName);

    // Write header
    for (size_t i = 0; i < this->nodesTable.columns.size(); i++)
    {
        if (i > 0)
            nodesOut << ",";
        nodesOut << this->nodesTable.columns[i];
    }
    nodesOut << endl;

    // Write nodes in path
    for (int nodeId : path)
    {
        vector<int> attrs = getNodeAttributes(nodeId);
        nodesOut << nodeId;
        for (int attr : attrs)
        {
            nodesOut << "," << attr;
        }
        nodesOut << endl;
    }
    nodesOut.close();

    // Write edges file
    string edgesFileName = "../data/" + resultName + "_Edges_" + dirChar + ".csv";
    ofstream edgesOut(edgesFileName);

    // Write header
    for (size_t i = 0; i < this->edgesTable.columns.size(); i++)
    {
        if (i > 0)
            edgesOut << ",";
        edgesOut << this->edgesTable.columns[i];
    }
    edgesOut << endl;

    // Write edges
    for (auto &edge : edges)
    {
        for (size_t i = 0; i < edge.size(); i++)
        {
            if (i > 0)
                edgesOut << ",";
            edgesOut << edge[i];
        }
        edgesOut << endl;
    }
    edgesOut.close();

    // Load the new graph into catalogue
    Graph *newGraph = new Graph(resultName, this->isDirected);
    if (newGraph->load())
    {
        graphCatalogue.insertGraph(newGraph);
    }
}

/**
 * @brief Check if node satisfies all node conditions
 * uniformValues tracks values for conditions without == (must be uniform)
 */
bool Graph::checkNodeConditions(int nodeId, vector<PathCondition> &conditions,
                                unordered_map<string, int> &uniformValues)
{
    vector<int> attrs = getNodeAttributes(nodeId);
    if (attrs.empty())
        return false;

    // Get column names (skip NodeID)
    vector<string> &columns = this->nodesTable.columns;

    for (auto &cond : conditions)
    {
        if (!cond.isNode)
            continue; // Skip edge conditions

        if (cond.attribute == "ANY")
        {
            // ANY(N) == value: some attribute must be 'value' for all nodes
            // ANY(N): some attribute must be uniform across all nodes
            bool found = false;

            for (size_t i = 1; i < columns.size(); i++)
            {
                int attrVal = attrs[i - 1];

                if (cond.hasValue)
                {
                    if (attrVal == cond.value)
                    {
                        if (uniformValues.find("ANY_N_attr") == uniformValues.end())
                        {
                            uniformValues["ANY_N_attr"] = i;
                            uniformValues["ANY_N_val"] = attrVal;
                            found = true;
                            break;
                        }
                        else if (uniformValues["ANY_N_attr"] == (int)i &&
                                 uniformValues["ANY_N_val"] == attrVal)
                        {
                            found = true;
                            break;
                        }
                    }
                }
                else
                {
                    if (uniformValues.find("ANY_N_attr") == uniformValues.end())
                    {
                        uniformValues["ANY_N_attr"] = i;
                        uniformValues["ANY_N_val"] = attrVal;
                        found = true;
                        break;
                    }
                    else if (uniformValues["ANY_N_attr"] == (int)i &&
                             uniformValues["ANY_N_val"] == attrVal)
                    {
                        found = true;
                        break;
                    }
                }
            }
            if (!found)
                return false;
        }
        else
        {
            // Find attribute index
            int attrIdx = -1;
            for (size_t i = 1; i < columns.size(); i++)
            {
                if (columns[i] == cond.attribute)
                {
                    attrIdx = i - 1;
                    break;
                }
            }
            if (attrIdx == -1)
                return false; // Attribute not found

            int attrVal = attrs[attrIdx];

            if (cond.hasValue)
            {
                // ATTR(N) == value
                if (attrVal != cond.value)
                    return false;
            }
            else
            {
                // ATTR(N) - must be uniform across path
                string key = cond.attribute + "_N";
                if (uniformValues.find(key) == uniformValues.end())
                {
                    uniformValues[key] = attrVal;
                }
                else if (uniformValues[key] != attrVal)
                {
                    return false;
                }
            }
        }
    }
    return true;
}

/**
 * @brief Check if edge satisfies all edge conditions
 * edge format: [dst, weight, attr1, attr2, ...]
 */
bool Graph::checkEdgeConditions(vector<int> &edge, vector<PathCondition> &conditions,
                                unordered_map<string, int> &uniformValues)
{
    // Edge columns (original): Src, Dst, Weight, B1, B2, ...
    // Stored edge: [Dst, Weight, B1, B2, ...]
    // So B1 is at index 2 in stored edge

    vector<string> &columns = this->edgesTable.columns;

    for (auto &cond : conditions)
    {
        if (cond.isNode)
            continue; // Skip node conditions

        if (cond.attribute == "ANY")
        {
            bool found = false;

            // Attributes start at index 3 in original (Src, Dst, Weight, B1...)
            // In stored edge, they start at index 2 (Dst, Weight, B1...)
            for (size_t i = 3; i < columns.size(); i++)
            {
                int attrIdx = i - 1; // Index in stored edge
                if (attrIdx >= (int)edge.size())
                    continue;

                int attrVal = edge[attrIdx];

                if (cond.hasValue)
                {
                    if (attrVal == cond.value)
                    {
                        if (uniformValues.find("ANY_E_attr") == uniformValues.end())
                        {
                            uniformValues["ANY_E_attr"] = i;
                            uniformValues["ANY_E_val"] = attrVal;
                            found = true;
                            break;
                        }
                        else if (uniformValues["ANY_E_attr"] == (int)i &&
                                 uniformValues["ANY_E_val"] == attrVal)
                        {
                            found = true;
                            break;
                        }
                    }
                }
                else
                {
                    if (uniformValues.find("ANY_E_attr") == uniformValues.end())
                    {
                        uniformValues["ANY_E_attr"] = i;
                        uniformValues["ANY_E_val"] = attrVal;
                        found = true;
                        break;
                    }
                    else if (uniformValues["ANY_E_attr"] == (int)i &&
                             uniformValues["ANY_E_val"] == attrVal)
                    {
                        found = true;
                        break;
                    }
                }
            }
            if (!found)
                return false;
        }
        else
        {
            // Find attribute index in original columns
            int origIdx = -1;
            for (size_t i = 3; i < columns.size(); i++)
            {
                if (columns[i] == cond.attribute)
                {
                    origIdx = i;
                    break;
                }
            }
            if (origIdx == -1)
                return false;

            // In stored edge, index is origIdx - 1 (since we removed Src)
            int storedIdx = origIdx - 1;
            if (storedIdx >= (int)edge.size())
                return false;

            int attrVal = edge[storedIdx];

            if (cond.hasValue)
            {
                if (attrVal != cond.value)
                    return false;
            }
            else
            {
                string key = cond.attribute + "_E";
                if (uniformValues.find(key) == uniformValues.end())
                {
                    uniformValues[key] = attrVal;
                }
                else if (uniformValues[key] != attrVal)
                {
                    return false;
                }
            }
        }
    }
    return true;
}

// /**
//  * @brief Dijkstra's shortest path
//  */
// pair<long long, vector<int>> Graph::shortestPath(int src, int dst)
// {
//     logger.log("Graph::shortestPath");

//     priority_queue<pair<long long, int>, vector<pair<long long, int>>, greater<pair<long long, int>>> pq;
//     unordered_map<int, long long> dist;
//     unordered_map<int, int> parent;

//     dist[src] = 0;
//     parent[src] = -1;
//     pq.push({0, src});

//     while (!pq.empty()) {
//         auto [d, u] = pq.top();
//         pq.pop();

//         if (u == dst) {
//             vector<int> path;
//             int curr = dst;
//             while (curr != -1) {
//                 path.push_back(curr);
//                 curr = parent[curr];
//             }
//             reverse(path.begin(), path.end());
//             return {d, path};
//         }

//         if (dist.count(u) && d > dist[u]) continue;

//         vector<vector<int>> neighbors = this->getNeighbors(u);

//         for (auto& edge : neighbors) {
//             int v = edge[this->destColumnIndex];
//             int w = edge[this->weightColumnIndex];

//             long long newDist = d + w;

//             if (!dist.count(v) || newDist < dist[v]) {
//                 dist[v] = newDist;
//                 parent[v] = u;
//                 pq.push({newDist, v});
//             }
//         }
//     }

//     return {-1, {}};
// }
