#pragma once

#include <vector>
#include <functional>
#include <map>
#include <string>
#include <climits>
#include <any>



class PathInfo
{
public:
    bool direction;
    int edgeIndex;
    std::any edgeData;
    std::any node1Data;
    std::any node2Data;
};

class GraphNode
{
public:
    GraphNode() {}

    std::string name;
    int cost;
    bool visited;
    std::vector<PathInfo> path;
    std::any data;
};

class GraphEdge
{
public:
    GraphEdge() {}
    GraphNode* node1;
    GraphNode* node2;
    int cost;
    std::any data;
};


class Graph
{
public:
    Graph();
    Graph(const Graph* g);
    ~Graph();

    void addNode(const std::string& name, std::any data);
    void removeEdge(int i);
    void addEdge(const std::string& src, const std::string& dst, std::any data, int cost);
    std::vector<PathInfo> dijkstra(std::string source, std::string destination);
    int edgeCount();
    int nodeCount();

    std::any getEdgeData(int i);
    std::any getNodeData(const std::string& name);
    std::string getEdgeNodeName(int i, bool node1);

private:
    std::vector<GraphEdge*> edges;
    std::map<std::string,GraphNode*> nodes;

    void copy(const Graph* g);
    GraphNode* getSmallestUnvisitedNode();
    std::vector<PathInfo> getUnvisitedNeighbours(GraphNode* g);
};
