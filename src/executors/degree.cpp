#include "global.h"

bool syntacticParseDEGREE()
{
    logger.log("syntacticParseDEGREE");
    if (tokenizedQuery.size() != 3)
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    parsedQuery.queryType = DEGREE;
    parsedQuery.degreeGraphName = tokenizedQuery[1];
    try
    {
        parsedQuery.degreeNodeId = stoi(tokenizedQuery[2]);
    }
    catch (...)
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    return true;
}

bool semanticParseDEGREE()
{
    logger.log("semanticParseDEGREE");
    if (!graphCatalogue.isGraph(parsedQuery.degreeGraphName))
    {
        cout << "SEMANTIC ERROR: Graph doesn't exist" << endl;
        return false;
    }
    Graph *graph = graphCatalogue.getGraph(parsedQuery.degreeGraphName);
    if (graph == nullptr || !graph->nodeExists(parsedQuery.degreeNodeId))
    {
        cout << "Node does not exist" << endl;
        return false;
    }
    return true;
}

void executeDEGREE()
{
    logger.log("executeDEGREE");
    Graph *graph = graphCatalogue.getGraph(parsedQuery.degreeGraphName);
    if (graph == nullptr)
    {
        cout << "SEMANTIC ERROR: Graph doesn't exist" << endl;
        return;
    }
    cout << graph->findDegree(parsedQuery.degreeNodeId) << endl;
    return;
}
