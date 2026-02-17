#include "global.h"

int Graph::findDegree(int nodeId)
{
    logger.log("Graph::findDegree");
    
    int degree = 0;
    int pageNum = 0;

    string currentFile = this->getPageFileName(nodeId, pageNum);

    while (true)
    {
        
        auto [rowCount, nextPage] = this->readPageHeader(currentFile);
        
        // rowCount includes:
        // - Outgoing edges (normal entries)
        // - Incoming edges (dummy entries with weight -1) if graph is directed
        // Therefore, summing rowCount gives In-Degree + Out-Degree
        degree += rowCount;

        if (nextPage == -1)
            break;

        pageNum = nextPage;
        currentFile = this->getPageFileName(nodeId, pageNum);
    }
    
    return degree;
}