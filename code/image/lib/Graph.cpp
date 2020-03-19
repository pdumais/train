#include "Graph.h"



Graph::Graph()
{
}

Graph::Graph(const Graph* g)
{
    this->copy(g);
}

void Graph::copy(const Graph* g)
{
    if (g == this) return;

    for (auto it : g->nodes)
    {
        GraphNode* n = new GraphNode();
        n->data = it.second->data;
        n->name = it.second->name;;
        this->nodes[it.first] = n;
    }

    for (auto it : g->edges)
    {
        GraphEdge* edge = new GraphEdge();
        edge->data = (*it).data;
        edge->cost = (*it).cost;
        edge->node1 = this->nodes[(*it).node1->name];
        edge->node2 = this->nodes[(*it).node2->name];
        this->edges.push_back(edge);
    }
}

Graph::~Graph()
{
    for (auto it : this->edges)
    {
        delete it;
    }
    this->edges.clear();

    for (auto it : this->nodes)
    {
        delete it.second;
    }

    this->nodes.clear();
}

int Graph::edgeCount()
{
    return this->edges.size();
}

int Graph::nodeCount()
{
    return this->nodes.size();
}

std::any Graph::getEdgeData(int i)
{
    return this->edges[i]->data;
}

std::any Graph::getNodeData(const std::string& name)
{
    return this->nodes[name]->data;
}

std::string Graph::getEdgeNodeName(int i, bool node1)
{
    GraphEdge *ge = this->edges[i];
    return node1?ge->node1->name:ge->node2->name;

}

void Graph::addNode(const std::string& name, std::any data)
{
    GraphNode* g = new GraphNode();
    g->data = data;
    g->name = name;
    nodes[name] = g;
}

void Graph::removeEdge(int i)
{
    delete this->edges[i];
    this->edges.erase(this->edges.begin()+i);
}

void Graph::addEdge(const std::string& src, const std::string& dst, std::any data, int cost)
{
    if (!this->nodes.count(src) || !this->nodes.count(dst)) return;

    for (auto it : this->edges)
    {
        if (it->node1 == this->nodes[src] && it->node2 == this->nodes[dst]) return;
        if (it->node2== this->nodes[src] && it->node1 == this->nodes[dst]) return;
    }

    GraphEdge* g = new GraphEdge();
    g->node1 = this->nodes[src];
    g->node2 = this->nodes[dst];
    g->cost = cost;
    g->data = data;
    this->edges.push_back(g);
}

std::vector<PathInfo> Graph::dijkstra(std::string source, std::string destination)
{
    if (!this->nodes.count(source) || !this->nodes.count(destination)) return std::vector<PathInfo>();

    GraphNode* srcNode = this->nodes[source];
    GraphNode* dstNode = this->nodes[destination];

    for (auto it : this->nodes)
    {
        it.second->cost = INT_MAX;
        it.second->visited = false;
    }
    srcNode->cost = 0;

    GraphNode* currentNode = nullptr;

    while (1)
    {
        currentNode = this->getSmallestUnvisitedNode();
        if (currentNode == dstNode)
        {
            break;
        }
        for (auto pi : this->getUnvisitedNeighbours(currentNode))
        {

            auto edge = this->edges[pi.edgeIndex];
            GraphNode* n = pi.direction==false?edge->node2:edge->node1;

            int cost = currentNode->cost + edge->cost;
            if (n->cost > cost)
            {
                n->cost = cost;
                n->path = currentNode->path;
                n->path.push_back(pi);
            }
        }
        currentNode->visited = true;
    }

    return dstNode->path;
}



GraphNode* Graph::getSmallestUnvisitedNode()
{
    GraphNode* ret = nullptr;
    for (auto n : this->nodes)
    {
        if (ret == nullptr || (ret->cost > n.second->cost && !n.second->visited))
        {
            ret = n.second;
        }
    }
    return ret;
}

std::vector<PathInfo> Graph::getUnvisitedNeighbours(GraphNode* g)
{
    std::vector<PathInfo> ret;
    int i = 0;
    for (auto edge : this->edges)
    {
        if (edge->node1 == g && !edge->node2->visited)
        {
            PathInfo pi;
            pi.edgeIndex = i;
            pi.direction = false;
            pi.edgeData = edge->data;
            pi.node1Data = edge->node1->data;
            pi.node2Data = edge->node2->data;
            ret.push_back(pi);
        }
        else if (edge->node2 == g && !edge->node1->visited)
        {
            PathInfo pi;
            pi.edgeIndex = i;
            pi.direction = true;
            pi.edgeData = edge->data;
            pi.node1Data = edge->node1->data;
            pi.node2Data = edge->node2->data;
            ret.push_back(pi);
        }
        i++;
    }

    return ret;
}
