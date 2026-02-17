#include "global.h"

/**
 * @brief 
 * SYNTAX: EXPORT <relation_name> 
 *         EXPORT GRAPH <graph_name>
 */

bool syntacticParseEXPORT()
{
    logger.log("syntacticParseEXPORT");
    if (tokenizedQuery.size() != 2)
    {
        if (tokenizedQuery.size() != 3 || tokenizedQuery[1] != "GRAPH")
        {
            cout << "SYNTAX ERROR" << endl;
            return false;
        }
    }
    parsedQuery.queryType = EXPORT;
    if (tokenizedQuery.size() == 3)
    {
        parsedQuery.isGraph = true;
        parsedQuery.exportRelationName = tokenizedQuery[2];
    }
    else
    {
        parsedQuery.isGraph = false;
        parsedQuery.exportRelationName = tokenizedQuery[1];
    }
    return true;
}

bool semanticParseEXPORT()
{
    logger.log("semanticParseEXPORT");
    if (parsedQuery.isGraph)
    {
        if (graphCatalogue.isGraph(parsedQuery.exportRelationName))
            return true;
        cout << "SEMANTIC ERROR: Graph doesn't exist" << endl;
        return false;
    }
    if (tableCatalogue.isTable(parsedQuery.exportRelationName))
        return true;
    cout << "SEMANTIC ERROR: No such relation exists" << endl;
    return false;
}

void writeRow(const vector<string> &row, ostream &out)
{
    for (size_t i = 0; i < row.size(); i++)
    {
        if (i > 0)
            out << ",";
        out << row[i];
    }
    out << endl;
}

void writeRow(const vector<int> &row, ostream &out)
{
    for (size_t i = 0; i < row.size(); i++)
    {
        if (i > 0)
            out << ",";
        out << row[i];
    }
    out << endl;
}

void executeEXPORT()
{
    logger.log("executeEXPORT");
    if (parsedQuery.isGraph)
    {
        Graph *graph = graphCatalogue.getGraph(parsedQuery.exportRelationName);
        if (graph == nullptr)
        {
            cout << "SEMANTIC ERROR: Graph doesn't exist" << endl;
            return;
        }

        string nodesFile = "../data/" + graph->nodesTable.tableName + ".csv";
        ofstream nodesOut(nodesFile, ios::out);
        writeRow(graph->nodesTable.columns, nodesOut);
        Cursor nodeCursor = graph->nodesTable.getCursor();
        vector<int> row;
        for (long long i = 0; i < graph->nodesTable.rowCount; i++)
        {
            row = nodeCursor.getNext();
            if (row.empty())
                break;
            writeRow(row, nodesOut);
        }
        nodesOut.close();

        string edgesFile = "../data/" + graph->edgesTable.tableName + ".csv";
        ofstream edgesOut(edgesFile, ios::out);
        writeRow(graph->edgesTable.columns, edgesOut);
        Cursor edgeCursor = graph->edgesTable.getCursor();
        for (long long i = 0; i < graph->edgesTable.rowCount; i++)
        {
            row = edgeCursor.getNext();
            if (row.empty())
                break;
            writeRow(row, edgesOut);
        }
        edgesOut.close();
        return;
    }
    Table *table = tableCatalogue.getTable(parsedQuery.exportRelationName);
    table->makePermanent();
    return;
}
