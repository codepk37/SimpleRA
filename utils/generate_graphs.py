import pandas as pd
import numpy as np

# CONFIG
NUM_NODES = 5000
NUM_EDGES = 50000

# 1. Generate Nodes (NodeID, A1..A4)
nodes = pd.DataFrame({
    'NodeID': range(1, NUM_NODES + 1),
    'A1': np.random.randint(0, 2, NUM_NODES),
    'A2': np.random.randint(0, 2, NUM_NODES),
    'A3': np.random.randint(0, 2, NUM_NODES),
    'A4': np.random.randint(0, 2, NUM_NODES)
})
nodes.to_csv(f"../data/{NUM_NODES}_{NUM_EDGES}_Nodes_D.csv", index=False)

# 2. Generate Edges (Src, Dst, Weight, B1..B4)
edges = pd.DataFrame({
    'Src_NodeID': np.random.randint(1, NUM_NODES + 1, NUM_EDGES),
    'Dest_NodeID': np.random.randint(1, NUM_NODES + 1, NUM_EDGES),
    'Weight': np.random.randint(1, 100, NUM_EDGES),
    'B1': np.random.randint(0, 2, NUM_EDGES),
    'B2': np.random.randint(0, 2, NUM_EDGES),
    'B3': np.random.randint(0, 2, NUM_EDGES),
    'B4': np.random.randint(0, 2, NUM_EDGES)
})
edges = edges[edges['Src_NodeID'] != edges['Dest_NodeID']]
edges.to_csv(f"../data/{NUM_NODES}_{NUM_EDGES}_Edges_D.csv", index=False)
