// #include "global.h"

// /**
//  * @brief 
//  * SYNTAX: SHORTESTPATH graphName srcNode dstNode
//  * Example: SHORTESTPATH test1 1 4
//  */
// bool syntacticParseSHORTESTPATH()
// {
//     logger.log("syntacticParseSHORTESTPATH");
    
//     // Expected: SHORTESTPATH <graphName> <srcNode> <dstNode>
//     if (tokenizedQuery.size() != 4) {
//         cout << "SYNTAX ERROR: Expected SHORTESTPATH <graphName> <srcNode> <dstNode>" << endl;
//         return false;
//     }
    
//     parsedQuery.queryType = SHORTESTPATH;
//     parsedQuery.shortestPathGraphName = tokenizedQuery[1];
    
//     try {
//         parsedQuery.shortestPathSrc = stoi(tokenizedQuery[2]);
//         parsedQuery.shortestPathDst = stoi(tokenizedQuery[3]);
//     } catch (...) {
//         cout << "SYNTAX ERROR: Source and destination must be integers" << endl;
//         return false;
//     }
    
//     return true;
// }

// bool semanticParseSHORTESTPATH()
// {
//     logger.log("semanticParseSHORTESTPATH");
    
//     // Check if graph exists
//     if (!graphCatalogue.isGraph(parsedQuery.shortestPathGraphName)) {
//         cout << "SEMANTIC ERROR: Graph " << parsedQuery.shortestPathGraphName << " does not exist" << endl;
//         return false;
//     }
    
//     return true;
// }

// void executeSHORTESTPATH()
// {
//     logger.log("executeSHORTESTPATH");
    
//     Graph* graph = graphCatalogue.getGraph(parsedQuery.shortestPathGraphName);
    
//     if (graph == nullptr) {
//         cout << "ERROR: Graph not found" << endl;
//         return;
//     }
    
//     int src = parsedQuery.shortestPathSrc;
//     int dst = parsedQuery.shortestPathDst;
    
//     cout << "Finding shortest path from " << src << " to " << dst << "..." << endl;
    
//     auto [cost, path] = graph->shortestPath(src, dst);
    
//     if (cost == -1) {
//         cout << "No path exists from " << src << " to " << dst << endl;
//     } else {
//         cout << "Shortest path cost: " << cost << endl;
//         cout << "Path: ";
//         for (size_t i = 0; i < path.size(); i++) {
//             if (i > 0) cout << " -> ";
//             cout << path[i];
//         }
//         cout << endl;
//     }
// }