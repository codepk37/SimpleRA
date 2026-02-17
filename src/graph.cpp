#include "global.h"

extern Logger logger;
extern BufferManager bufferManager;
extern float BLOCK_SIZE;

Graph::Graph()
{
    logger.log("Graph::Graph");
    this->nodeCount = 0;
}

Graph::Graph(string graphName, bool isDirected)
{
    logger.log("Graph::Graph");
    this->graphName = graphName;
    this->isDirected = isDirected;
    char directionChar = isDirected ? 'D' : 'U';
    this->nodesFile = graphName + "_Nodes_" + directionChar;
    this->edgesFile = graphName + "_Edges_" + directionChar;
    this->nodeCount = 0;
}

bool Graph::load()
{
    logger.log("Graph::load");

    this->nodesTable = Table(this->nodesFile);
    if (!this->nodesTable.load())
        return false;

    tableCatalogue.insertTable(&this->nodesTable);
    this->nodeCount = this->nodesTable.rowCount;

    this->edgesTable = Table(this->edgesFile);
    if (!this->edgesTable.load())
        return false;
    tableCatalogue.insertTable(&this->edgesTable);
    // cout<< this->edgesTable.rowCount << endl;

    return this->loadAdjacencyList();
}

/**
 * @brief Get page filename for a node
 */
string Graph::getPageFileName(int nodeId, int pageNum)
{
    return "../data/temp/" + this->edgesTable.tableName + "_Node" + to_string(nodeId) + "_" + to_string(pageNum);
}

/**
 * @brief Read header from page file: returns {rowCount, nextPageNum}
 */
pair<int, int> Graph::readPageHeader(const string &filename)
{
    ifstream fin(filename, ios::in);
    int rowCount = 0, nextPage = -1;
    if (fin.is_open())
    {
        fin >> rowCount >> nextPage;
        fin.close();
    }
    return {rowCount, nextPage};
}

/**
 * @brief Update header in page file (rewrite first line)
 */
void Graph::updatePageHeader(const string &filename, int rowCount, int nextPage)
{
    // Read existing content
    ifstream fin(filename, ios::in);
    string firstLine, rest;
    getline(fin, firstLine); // Skip old header
    stringstream buffer;
    buffer << fin.rdbuf();
    rest = buffer.str();
    fin.close();

    // Rewrite with new header
    ofstream fout(filename, ios::trunc);
    fout << rowCount << " " << nextPage << endl;
    fout << rest;
    fout.close();
}

/**
 * @brief Create new page file with header
 */
void Graph::createPageFile(const string &filename)
{
    ofstream fout(filename, ios::trunc);
    fout << "0 -1" << endl; // 0 rows, no next page
    fout.close();
}

/**
 * @brief Append edge to page file (after header)
 */
void Graph::appendEdgeToPage(const string &filename, vector<int> &edgeData)
{
    ofstream fout(filename, ios::app);
    for (size_t i = 0; i < edgeData.size(); i++)
    {
        if (i != 0)
            fout << " ";
        fout << edgeData[i];
    }
    fout << endl;
    fout.close();
}

/**
 * @brief Insert edge for a node - handles page creation and chaining
 */
void Graph::insertEdge(int nodeId, vector<int> &edgeData)
{
    string baseFile = getPageFileName(nodeId, 0);

    // If first page doesn't exist, create it
    if (!fileExists(baseFile))
    {
        createPageFile(baseFile);
    }

    // Find the page to insert into (follow chain to find page with space)
    string currentFile = baseFile;
    int pageNum = 0;

    while (true)
    {
        auto [rowCount, nextPage] = readPageHeader(currentFile);

        if (rowCount < (int)this->maxEdgesPerPage)
        {
            // This page has space - append edge here
            appendEdgeToPage(currentFile, edgeData);
            updatePageHeader(currentFile, rowCount + 1, nextPage);
            return;
        }

        // Page is full - check if there's a next page
        if (nextPage == -1)
        {
            // No next page - create one and link it
            int newPageNum = pageNum + 1;
            string newFile = getPageFileName(nodeId, newPageNum);
            createPageFile(newFile);

            // Update current page to point to new page
            updatePageHeader(currentFile, rowCount, newPageNum);

            // Insert into new page
            appendEdgeToPage(newFile, edgeData);
            updatePageHeader(newFile, 1, -1);
            return;
        }

        // Move to next page
        pageNum = nextPage;
        currentFile = getPageFileName(nodeId, pageNum);
    }
}

/**
 * @brief Load edges - streaming, no buffering
 */
bool Graph::loadAdjacencyList()
{
    logger.log("Graph::loadAdjacencyList");

    // this->edgesTable = Table(this->edgesFile);
    string edgeFilePath = "../data/" + this->edgesFile + ".csv";
    // cout<< edgeFilePath << this->edgesTable.columnCount << endl;

    fstream fin(edgeFilePath, ios::in);
    if (!fin.is_open())
    {
        cout << "Error: Cannot open edges file" << endl;
        return false;
    }

    string line;
    if (!getline(fin, line))
    {
        fin.close();
        return false;
    }

    // if (!this->edgesTable.extractColumnNames(line)) {
    //     fin.close();
    //     return false;
    // }

    // Stored columns = original - 1 (no source)
    this->storedColCount = this->edgesTable.columnCount - 1;

    // Max edges per page (reserve space for header row worth of integers)
    uint maxRowsPerBlock = (uint)((BLOCK_SIZE * 1000) / (sizeof(int) * this->storedColCount));
    if (maxRowsPerBlock <= 1)
        maxRowsPerBlock = 2;
    this->maxEdgesPerPage = maxRowsPerBlock - 1;

    // cout << "DEBUG: Stored columns per edge = " << this->storedColCount << endl;
    // cout << "DEBUG: maxEdgesPerPage = " << this->maxEdgesPerPage << endl;

    // Stream through edges - write directly to disk
    while (getline(fin, line))
    {
        stringstream s(line);
        string word;
        vector<int> row;

        while (getline(s, word, ','))
        {
            word.erase(remove_if(word.begin(), word.end(), ::isspace), word.end());
            if (word.empty())
                continue;
            try
            {
                row.push_back(stoi(word));
            }
            catch (...)
            {
                continue;
            }
        }

        if (row.size() != this->edgesTable.columnCount)
            continue;

        int src = row[0];
        int dst = row[1];

        // Edge data: [dst, weight, ...]
        vector<int> edgeData(row.begin() + 1, row.end());

        // Insert edge for source node
        insertEdge(src, edgeData);

        // For undirected, insert reverse edge
        if (!this->isDirected)
        {
            vector<int> reverseEdge = edgeData;
            reverseEdge[0] = src;
            insertEdge(dst, reverseEdge);
        }
        else{
            // Insert a dummy edge for the destination node to ensure it exists in the adjacency list
            // Pad with 0s to match storedColCount so getNeighbors reads the file correctly
            vector<int> dummy_E(this->storedColCount, 0);
            dummy_E[0] = src; // dst is the original src
            dummy_E[1] = -1;  // weight marker for dummy
            insertEdge(dst, dummy_E);
        }
    }
    fin.close();

    // cout << "Graph adjacency list loaded." << endl;

    return true;
}

/**
 * @brief Get node attributes by reading from nodesTable
 */
vector<int> Graph::getNodeAttributes(int nodeId)
{
    logger.log("Graph::getNodeAttributes");
    vector<int> attributes;

    // Read through nodes table pages to find the node
    Cursor cursor = this->nodesTable.getCursor();
    vector<int> row;

    while (!(row = cursor.getNext()).empty())
    {
        if (row[0] == nodeId)
        {
            // Return all attributes (skip NodeID at index 0)
            attributes = vector<int>(row.begin() + 1, row.end());
            break;
        }
    }

    return attributes;
}

/**
 * @brief Check if node exists in the graph
 */
bool Graph::nodeExists(int nodeId)
{
    Cursor cursor = this->nodesTable.getCursor();
    vector<int> row;

    while (!(row = cursor.getNext()).empty())
    {
        if (row[0] == nodeId)
        {
            return true;
        }
    }
    return false;
}

bool Graph::unload()
{
    logger.log("Graph::unload");
    this->nodesTable.unload();
    this->edgesTable.unload();
    // Page files will be cleaned up by system temp cleanup
    return true;
}

/**
 * @brief Check if file exists
 */
bool fileExists(const string &filename)
{
    struct stat buffer;
    return (stat(filename.c_str(), &buffer) == 0);
}