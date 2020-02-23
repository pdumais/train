#pragma once

#include <vector>
#include <functional>
#include <map>
#include <string>
#include <climits>

//template<class T>
//struct GraphEdge;

class PathInfo;

class IGraphEdge
{
};

class IGraphNode
{
public:
    std::string name;
    int cost;
    bool visited;
    std::vector<PathInfo> path;
};

template<class T>
class GraphNode: public IGraphNode
{
public:
    GraphNode() {}

    T data;
};

template<class T2>
class GraphEdge: public IGraphEdge
{
public:
    GraphEdge() {}

    IGraphNode* node1;
    IGraphNode* node2;
    int cost;
    T2 data;
};

class PathInfo
{
public:
    bool direction;
    IGraphEdge* edge;
};

template<class T, class T2>
class Graph
{
public:
    Graph()
    {
    }

    Graph(const Graph* g)
    {
        this->copy(g);
    }

    void copy(const Graph* g)
    {
        if (g == this) return;

        for (auto it : g->nodes)
        {
            GraphNode<T>* n = new GraphNode<T>();
            n->data = it.second->data;
            n->name = it.second->name;;
            this->nodes[it.first] = n;
        }

        for (auto it : g->edges)
        {
            GraphEdge<T2>* edge = new GraphEdge<T2>();
            edge->data = (*it).data;
            edge->cost = (*it).cost;
            edge->node1 = this->nodes[(*it).node1->name];
            edge->node2 = this->nodes[(*it).node2->name];
            this->edges.push_back(edge);
        }
    }

    ~Graph() 
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

    void addNode(const std::string& name, T data)
    {
        GraphNode<T>* g = new GraphNode<T>();
        g->data = data;
        g->name = name;
        nodes[name] = g;
    }

    void removeEdge(GraphEdge<T2>* edge)
    {
        for (auto it = this->edges.begin(); it != this->edges.end(); it++)
        {
            if (*it == edge)
            {
                this->edges.erase(it);
                delete edge;
                return;
            }
        }
    }

    void addEdge(const std::string& src, const std::string& dst, T2 data, int cost)
    {
        if (!this->nodes.count(src) || !this->nodes.count(dst)) return;

        for (auto it : this->edges) 
        {
            if (it->node1 == this->nodes[src] && it->node2 == this->nodes[dst]) return;
            if (it->node2== this->nodes[src] && it->node1 == this->nodes[dst]) return;
        }

        GraphEdge<T2>* g = new GraphEdge<T2>();
        g->node1 = this->nodes[src];
        g->node2 = this->nodes[dst];
        g->cost = cost;
        g->data = data;
        this->edges.push_back(g);
    }

    std::map<std::string,GraphNode<T>*> getNodes()
    {
        return this->nodes;
    }

    std::vector<GraphEdge<T2>*> getEdges()
    {
        return this->edges;
    }

    std::vector<PathInfo> dijkstra(std::string source, std::string destination)
    {
        if (!this->nodes.count(source) || !this->nodes.count(destination)) return std::vector<PathInfo>();

        GraphNode<T> * srcNode = this->nodes[source];
        GraphNode<T> * dstNode = this->nodes[destination];

        for (auto it : this->nodes)
        {
            it.second->cost = INT_MAX;
            it.second->visited = false;
        }
        srcNode->cost = 0;

        GraphNode<T>* currentNode = nullptr;

        while (1)
        {
            currentNode = this->getSmallestUnvisitedNode();
            if (currentNode == dstNode)
            {
                break;
            }
            for (auto pi : this->getUnvisitedNeighbours(currentNode))
            {

                auto edge = (GraphEdge<T2>*)pi.edge;
                GraphNode<T>* n = (GraphNode<T>*)(pi.direction==false?edge->node2:edge->node1);

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


private:
    std::vector<GraphEdge<T2>*> edges;
    std::map<std::string,GraphNode<T>*> nodes;


    GraphNode<T>* getSmallestUnvisitedNode()
    {
        GraphNode<T>* ret = nullptr;
        for (auto n : this->nodes)
        {
            if (ret == nullptr || (ret->cost > n.second->cost && !n.second->visited))
            {
                ret = n.second;
            }
        }
        return ret;
    }

    std::vector<PathInfo> getUnvisitedNeighbours(GraphNode<T>* g)
    {
        std::vector<PathInfo> ret;
        for (auto edge : this->edges)
        {
            if (edge->node1 == g && !edge->node2->visited)
            {
                PathInfo pi;
                pi.edge = edge;
                pi.direction = false;
                ret.push_back(pi);
            }
            else if (edge->node2 == g && !edge->node1->visited)
            {
                PathInfo pi;
                pi.edge = edge;
                pi.direction = true;
                ret.push_back(pi);
            }
        }

        return ret;
    }
};






















