#ifndef GRAPH_H
#define GRAPH_H
#include "table.h"
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <queue>
#include <climits>
#include <unordered_set>
#include <sys/stat.h>

// Forward declaration
struct PathCondition;

class Graph{
    public:
        string graphName;
        bool isDirected;
        string nodesFile;
        string edgesFile;
        Table nodesTable;
        Table edgesTable;
        
        int nodeCount;
        int storedColCount;
        uint maxEdgesPerPage;
        
        int destColumnIndex = 0;
        int weightColumnIndex = 1;

        Graph();
        Graph(string graphName, bool isDirected);
        bool load();
        bool loadAdjacencyList();
        bool unload();
        
        // Page file operations
        string getPageFileName(int nodeId, int pageNum);
        pair<int, int> readPageHeader(const string& filename);
        void updatePageHeader(const string& filename, int rowCount, int nextPage);
        void createPageFile(const string& filename);
        void appendEdgeToPage(const string& filename, vector<int>& edgeData);
        void insertEdge(int nodeId, vector<int>& edgeData);
        
        vector<vector<int>> getNeighbors(int nodeId);
        
        // Get node attributes (reads from nodesTable)
        vector<int> getNodeAttributes(int nodeId);
        
        // Check if node exists
        bool nodeExists(int nodeId);
        
        // Path finding with conditions
        // Returns {cost, {path_nodes, edges_used}}
        pair<long long, pair<vector<int>, vector<vector<int>>>> findPath(
            int src, int dst, vector<PathCondition>& conditions);
        
        // Check if node satisfies conditions
        bool checkNodeConditions(int nodeId, vector<PathCondition>& conditions, 
                                  unordered_map<string, int>& uniformValues);
        
        // Check if edge satisfies conditions
        bool checkEdgeConditions(vector<int>& edge, vector<PathCondition>& conditions,
                                  unordered_map<string, int>& uniformValues);
        
        // Save path as new graph
        void savePathAsGraph(const string& resultName, vector<int>& path, 
                             vector<vector<int>>& edges);
        
        // Legacy shortest path (no conditions)
        // pair<long long, vector<int>> shortestPath(int src, int dst);
        int findDegree(int nodeId);
};

bool fileExists(const string& filename);
#endif