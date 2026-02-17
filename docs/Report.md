# SimpleRA Graph Extension - Implementation Guide

## 1. Design Philosophy & Constraints

The primary constraint driving this implementation is **limited main memory (2 blocks max)**. We cannot load the entire graph or even a full adjacency list into memory. 

**Solution:**  
We implemented a **disk-based adjacency list** where every node's edges are stored in their own chain of small page files. Algorithms (like shortest path) process these pages in a streaming fashion, reading one block at a time.

---

## 2. Data Storage Architecture

### A. Input Format
Standard CSVs are used for input, separated into Nodes and Edges tables.
*   **Nodes Table:** `ID, Attr1, Attr2...`
*   **Edges Table:** `Src, Dst, Weight, Attr1...`

### B. Internal Page Organization
Instead of a monolithic file, we split the graph into **Node-Pages**.

*   **File Naming:** `../data/temp/<Graph>_Edges_<Type>_Node<ID>_<PageNum>`
    *   e.g., `test1_Edges_U_Node1_0` (First page of edges for Node 1)
*   **Page Structure:**
    ```
    <RowCount> <NextPageID>     // Header: Metadata for chaining
    <Dst> <Wt> <Attr1> ...      // Edge 1
    <Dst> <Wt> <Attr1> ...      // Edge 2
    ```
*   **Chaining:** If a node has too many edges to fit in `BLOCK_SIZE` (1KB), we create `Node<ID>_1`, point `Node<ID>_0`'s header to it, and continue writing. This forms a linked list of pages on disk.

### C. Why Direct I/O?
We bypass the standard `BufferManager` for edge iteration because:
1.  Variable-sized chains per node don't map well to fixed-index logical pages.
2.  We need strictly controlled eviction (read page -> process -> discard) to ensure we never exceed the 2-block limit during BFS/Dijkstra.

---

## 3. Key Components (`src/graph.cpp`)

### `loadAdjacencyList()`
*   **Purpose:** Converts the raw Edges CSV into our Node-Page format.
*   **Mechanism:**
    1.  Reads Edges CSV line-by-line (Stream processing).
    2.  Extracts `Src`, `Dst`, `Weight`.
    3.  Calls `insertEdge(Src, Data)`.
    4.  If Undirected, swaps Src/Dst and calls `insertEdge(Dst, Data)`.
*   **Efficiency:** writes happen immediately; no in-memory buffering preventing OOM on large graphs.

### `insertEdge(nodeId, data)`
*   **Locic:**
    1.  Calculates file path for `Node<ID>_0`.
    2.  Traverses the page chain (reading headers) until a page with space is found.
    3.  If a page is full and has no next page, creates a new page and updates the current page's `NextPageID` header.
    4.  Appends edge data to the file.

### `findPath(src, dst, conditions)`
*   **Algorithm:** Modified Dijkstra's Algorithm.
*   **State:** `{distance, current_node, path_history, edges_history, uniformity_constraints}`.
*   **Execution Flow:**
    1.  Push `src` to Priority Queue.
    2.  **Constraint Check:** Validate source node against `WHERE` conditions.
    3.  **Expansion:**
        *   Call `getNeighbors(current_node)`.
        *   This function reads `Node<ID>_0`, then `_1`, etc., returning a vector of edges.
    4.  **Edge/Neighbor Check:**
        *   For each neighbor, check if the **Edge** satisfies `E` conditions.
        *   Check if the **Destination Node** satisfies `N` conditions.
        *   Check **Uniformity** (Are attributes consistent with the path so far?).
    5.  If checks pass, push to PQ.

---
## 4. Degree Calculation Strategy

### Challenge
Calculating the degree of a node in a directed graph typically requires scanning the entire edge set to count incoming edges (In-Degree), as our adjacency list only naturally groups outgoing edges (Out-Degree).

### Solution: Dummy Incoming Edges
To enable O(1) page access for degree calculation, we insert **dummy edges** during the `LOAD` phase.

1.  **For Undirected Graphs:** 
    *   No change needed. An edge `A-B` is written to `A`'s file (neighbor B) and `B`'s file (neighbor A). `RowCount` naturally equals the degree.
    
2.  **For Directed Graphs (A -> B):**
    *   **Outgoing:** Write `[Dest: B, W: x, ...]` to `NodeA` file.
    *   **Incoming (Dummy):** Write `[Dest: A, W: -1, ...]` to `NodeB` file.
    *   **Marker:** The weight `-1` serves as a marker for a "back-edge".

### Impact
*   **Degree Query:** Simply summing the `rowCount` header of all page files for a node gives the total degree (`In-Degree + Out-Degree`) without parsing edge content.
*   **Path Finding:** The `findPath` iterator explicitly checks for `Weight == -1` and skips these dummy entries to prevent traversing edges backwards.

---

## 5. Other Commands

### `LOAD GRAPH`
*   Validates graph existence and name uniqueness using the graph catalogue.
*   Loads `Nodes` and `Edges` CSVs as tables into `../data/temp`.
*   Streams the edges file and builds per-node page chains (`Node<ID>_<PageNum>`).

### `EXPORT GRAPH`
*   Writes nodes and edges tables back to `../data` as `GraphName_Nodes_D/U.csv` and `GraphName_Edges_D/U.csv`.
*   Uses stored column headers and a cursor to stream rows in order.

### `PRINT GRAPH`
*   Prints node count, edge count, and graph type (`D/U`).
*   Streams and prints nodes, then edges, separated by a blank line.

## 6. Assumptions

*   CSV values are integers; boolean attributes are encoded as `0/1`.
*   `PATH` conditions reference valid column names; invalid attributes yield no valid path.
*   Graph temp files are writable and persist throughout the session.

## 7. Contributions

*   Jayanth: Graph storage design and initial implementation.
*   Himanshu: Add-ons, fixes, and proof-reading.
*   Pavan: Add-ons, fixes, and proof-reading.
