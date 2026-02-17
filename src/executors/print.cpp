#include "global.h"
/**
 * @brief 
 * SYNTAX: PRINT relation_name
 *         PRINT GRAPH graph_name
 */
bool syntacticParsePRINT()
{
    logger.log("syntacticParsePRINT");
    if (tokenizedQuery.size() != 2)
    {
        if (tokenizedQuery.size() != 3 || tokenizedQuery[1] != "GRAPH")
        {
            cout << "SYNTAX ERROR" << endl;
            return false;
        }
    }
    parsedQuery.queryType = PRINT;
    if (tokenizedQuery.size() == 3)
    {
        parsedQuery.isGraph = true;
        parsedQuery.printRelationName = tokenizedQuery[2];
    }
    else
    {
        parsedQuery.isGraph = false;
        parsedQuery.printRelationName = tokenizedQuery[1];
    }
    return true;
}

bool semanticParsePRINT()
{
    logger.log("semanticParsePRINT");
    if (parsedQuery.isGraph)
    {
        if (!graphCatalogue.isGraph(parsedQuery.printRelationName))
        {
            cout << "SEMANTIC ERROR: Graph doesn't exist" << endl;
            return false;
        }
        return true;
    }
    if (!tableCatalogue.isTable(parsedQuery.printRelationName))
    {
        cout << "SEMANTIC ERROR: Relation doesn't exist" << endl;
        return false;
    }
    return true;
}

void printRow(const vector<int> &row)
{
    for (size_t i = 0; i < row.size(); i++)
    {
        if (i > 0)
            cout << ",";
        cout << row[i];
    }
    cout << endl;
}

void executePRINT()
{
    logger.log("executePRINT");
    if (parsedQuery.isGraph)
    {
        Graph *graph = graphCatalogue.getGraph(parsedQuery.printRelationName);
        if (graph == nullptr)
        {
            cout << "SEMANTIC ERROR: Graph doesn't exist" << endl;
            return;
        }
        cout << graph->nodesTable.rowCount << endl;
        cout << graph->edgesTable.rowCount << endl;
        cout << (graph->isDirected ? "D" : "U") << endl << endl;

        Cursor nodeCursor = graph->nodesTable.getCursor();
        vector<int> row;
        for (long long i = 0; i < graph->nodesTable.rowCount; i++)
        {
            row = nodeCursor.getNext();
            if (row.empty())
                break;
            printRow(row);
        }
        cout << endl;

        Cursor edgeCursor = graph->edgesTable.getCursor();
        for (long long i = 0; i < graph->edgesTable.rowCount; i++)
        {
            row = edgeCursor.getNext();
            if (row.empty())
                break;
            printRow(row);
        }
        return;
    }
    Table *table = tableCatalogue.getTable(parsedQuery.printRelationName);
    table->print();
    return;
}
