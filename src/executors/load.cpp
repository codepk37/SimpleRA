#include "global.h"
/**
 * @brief
 * SYNTAX: LOAD relation_name
 */
bool syntacticParseLOAD()
{
    logger.log("syntacticParseLOAD");
    // cout << tokenizedQuery[0];
    if (tokenizedQuery.size() != 2)
    {
        if (tokenizedQuery.size() != 4 || tokenizedQuery[1] != "GRAPH" || (tokenizedQuery[3] != "U" && tokenizedQuery[3] != "D"))
        {
            cout << "SYNTAX ERROR" << endl;
            return false;
        }
    }
    parsedQuery.queryType = LOAD;
    if (tokenizedQuery.size() == 4)
    {
        parsedQuery.loadRelationName = tokenizedQuery[2];
        parsedQuery.isGraph = true;
        parsedQuery.graphType = tokenizedQuery[3][0];
        return true;
    }
    parsedQuery.loadRelationName = tokenizedQuery[1];
    return true;
}

bool semanticParseLOAD()
{
    logger.log("semanticParseLOAD");
    if (!parsedQuery.isGraph && tableCatalogue.isTable(parsedQuery.loadRelationName))
    {

        cout << "SEMANTIC ERROR: Relation already exists" << endl;
        return false;
    }
    if (parsedQuery.isGraph && graphCatalogue.isGraph(parsedQuery.loadRelationName))
    {
        cout << "SEMANTIC ERROR: Graph already exists" << endl;
        return false;
    }

    if ((!parsedQuery.isGraph && !isFileExists(parsedQuery.loadRelationName)) ||
        (parsedQuery.isGraph && !isGraphExists(parsedQuery.loadRelationName, parsedQuery.graphType)))
    {
        cout << "SEMANTIC ERROR: Data file doesn't exist" << endl;
        return false;
    }

    return true;
}

void executeLOAD()
{
    logger.log("executeLOAD");
    if (parsedQuery.isGraph)
    {
        Graph *graph = new Graph(parsedQuery.loadRelationName, parsedQuery.graphType == 'D');
        if (graph->load())
        {
            graphCatalogue.insertGraph(graph);
            cout << "Loaded Graph.Node Count:" << graph->nodesTable.rowCount << ",Edge Count:" << graph->edgesTable.rowCount << endl;
        }
        return;
    }
    Table *table = new Table(parsedQuery.loadRelationName);
    if (table->load())
    {
        tableCatalogue.insertTable(table);
        cout << "Loaded Table. Column Count: " << table->columnCount << " Row Count: " << table->rowCount << endl;
    }
    return;
}