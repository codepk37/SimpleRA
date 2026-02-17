#include "global.h"

/**
 * @brief Parse PATH query
 * SYNTAX: RES GRAPH <- PATH <graph_name> <src> <dst> [WHERE <conditions>]
 *
 * Conditions: ATTR(N|E) [== 0|1] AND ...
 */
bool syntacticParsePATH()
{
    logger.log("syntacticParsePATH");

    // Minimum: RES GRAPH <- PATH graph_name src dst
    // tokenizedQuery[0] = RES
    // tokenizedQuery[1] = GRAPH  (optional, but let's require it)
    // tokenizedQuery[2] = <-
    // tokenizedQuery[3] = PATH
    // tokenizedQuery[4] = graph_name
    // tokenizedQuery[5] = src
    // tokenizedQuery[6] = dst
    // tokenizedQuery[7] = WHERE (optional)
    // tokenizedQuery[8+] = conditions

    if (tokenizedQuery.size() < 6)
    {
        cout << "SYNTAX ERROR: Expected RES <- PATH graph src dst [WHERE conditions]" << endl;
        return false;
    }

    // Check format: RES GRAPH <- PATH
    if (tokenizedQuery[1] != "<-" || tokenizedQuery[2] != "PATH")
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }

    parsedQuery.queryType = PATHQUERY;
    parsedQuery.pathResultGraphName = tokenizedQuery[0];
    parsedQuery.pathGraphName = tokenizedQuery[3];

    try
    {
        parsedQuery.pathSrcNode = stoi(tokenizedQuery[4]);
        parsedQuery.pathDstNode = stoi(tokenizedQuery[5]);
    }
    catch (...)
    {
        cout << "SYNTAX ERROR: Source and destination must be integers" << endl;
        return false;
    }

    // Parse WHERE conditions if present
    if (tokenizedQuery.size() > 6)
    {
        if (tokenizedQuery[6] != "WHERE")
        {
            cout << "SYNTAX ERROR: Expected WHERE" << endl;
            return false;
        }

        // Parse conditions: ATTR(N|E) [== NUMBER] AND ...
        size_t i = 7;
        while (i < tokenizedQuery.size())
        {
            string token = tokenizedQuery[i];

            // Skip AND
            if (token == "AND")
            {
                i++;
                continue;
            }

            // Parse condition: ATTR(N) or ATTR(E)
            PathCondition cond;

            // Find ( and )
            size_t openParen = token.find('(');
            size_t closeParen = token.find(')');

            if (openParen == string::npos || closeParen == string::npos)
            {
                cout << "SYNTAX ERROR: Invalid condition format" << endl;
                return false;
            }

            cond.attribute = token.substr(0, openParen);
            string type = token.substr(openParen + 1, closeParen - openParen - 1);

            if (type == "N")
            {
                cond.isNode = true;
            }
            else if (type == "E")
            {
                cond.isNode = false;
            }
            else
            {
                cout << "SYNTAX ERROR: Condition type must be N or E" << endl;
                return false;
            }

            // Check if there's == NUMBER
            cond.hasValue = false;
            cond.value = 0;

            if (i + 1 < tokenizedQuery.size() && tokenizedQuery[i + 1] == "==")
            {
                if (i + 2 >= tokenizedQuery.size())
                {
                    cout << "SYNTAX ERROR: Expected value after ==" << endl;
                    return false;
                }
                try
                {
                    cond.value = stoi(tokenizedQuery[i + 2]);
                    if (cond.value != 0 && cond.value != 1)
                    {
                        cout << "SYNTAX ERROR: Condition value must be 0 or 1" << endl;
                        return false;
                    }
                    cond.hasValue = true;
                    i += 2; // Skip == and NUMBER
                }
                catch (...)
                {
                    cout << "SYNTAX ERROR: Condition value must be 0 or 1" << endl;
                    return false;
                }
            }

            parsedQuery.pathConditions.push_back(cond);
            i++;
        }
    }

    return true;
}

bool semanticParsePATH()
{
    logger.log("semanticParsePATH");

    // Check if graph exists
    if (!graphCatalogue.isGraph(parsedQuery.pathGraphName))
    {
        cout << "SEMANTIC ERROR: Graph doesn't exist" << endl;
        return false;
    }

    Graph *graph = graphCatalogue.getGraph(parsedQuery.pathGraphName);
    if (graph == nullptr)
    {
        cout << "SEMANTIC ERROR: Graph doesn't exist" << endl;
        return false;
    }
    if (!graph->nodeExists(parsedQuery.pathSrcNode) || !graph->nodeExists(parsedQuery.pathDstNode))
    {
        cout << "Node does not exist" << endl;
        return false;
    }

    // Check if result graph already exists
    if (graphCatalogue.isGraph(parsedQuery.pathResultGraphName))
    {
        cout << "SEMANTIC ERROR: Result graph already exists" << endl;
        return false;
    }

    return true;
}

void executePATH()
{
    logger.log("executePATH");

    Graph *graph = graphCatalogue.getGraph(parsedQuery.pathGraphName);

    if (graph == nullptr)
    {
        cout << "ERROR: Graph not found" << endl;
        return;
    }

    int src = parsedQuery.pathSrcNode;
    int dst = parsedQuery.pathDstNode;

    // Execute path query with conditions
    auto result = graph->findPath(src, dst, parsedQuery.pathConditions);

    long long cost = result.first;
    vector<int> &path = result.second.first;
    vector<vector<int>> &edgesUsed = result.second.second;

    if (cost == -1)
    {
        cout << "FALSE" << endl;
        return;
    }

    cout << "TRUE " << cost << endl;

    // Create result graph
    graph->savePathAsGraph(parsedQuery.pathResultGraphName, path, edgesUsed);
}