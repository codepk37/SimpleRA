// #include "graphCatalouge.h"
#include "global.h"

void GraphCatalogue::insertGraph(Graph *graph)
{
    logger.log("GraphCatalogue::~insertGraph");
    this->graphs[graph->graphName] = graph;
}
void GraphCatalogue::deleteGraph(string graphName)
{
    logger.log("GraphCatalogue::deleteGraph");
    tableCatalogue.removeTable(this->graphs[graphName]->nodesTable.tableName);
    tableCatalogue.removeTable(this->graphs[graphName]->edgesTable.tableName);
    this->graphs[graphName]->nodesTable.unload();
    this->graphs[graphName]->edgesTable.unload();
    delete this->graphs[graphName];
    this->graphs.erase(graphName);
}
Graph *GraphCatalogue::getGraph(string graphName)
{
    if (this->graphs.find(graphName) != this->graphs.end())
    {
        return this->graphs[graphName];
    }
    return nullptr;
}
bool GraphCatalogue::isGraph(string graphName)
{
    logger.log("GraphCatalogue::isGraph");
    if (this->graphs.count(graphName))
        return true;
    return false;
}
void GraphCatalogue::print()
{
    logger.log("GraphCatalogue::print");
    cout << "\nGRAPHS" << endl;

    int rowCount = 0;
    for (auto graph : this->graphs)
    {
        cout << graph.first << endl;
        rowCount++;
    }
    printRowCount(rowCount);
}

GraphCatalogue::~GraphCatalogue()
{
    logger.log("GraphCatalogue::~GraphCatalogue");
    for (auto graph : this->graphs)
    {
        tableCatalogue.removeTable(graph.second->nodesTable.tableName);
        tableCatalogue.removeTable(graph.second->edgesTable.tableName);
        graph.second->unload();
        delete graph.second;
    }
}
