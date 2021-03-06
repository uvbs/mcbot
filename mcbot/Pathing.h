#ifndef AI_PATHING_H_
#define AI_PATHING_H_

#include <functional>
#include <deque>
#include <map>
#include <list>

#include <mclib/Common.h>

namespace ai {
namespace path {

class Edge;

class Node {
private:
    Vector3i m_Position;
    std::vector<Edge*> m_Edges;

    Edge* FindNodeEdge(Node* other);

public:
    Node(Vector3i position);

    Vector3i GetPosition() { return m_Position; }
    
    void AddEdge(Edge* edge);
    // Follow all of the edges to grab any immediately connected nodes
    std::vector<Node*> GetNeighbors();
    float GetCostFrom(Node* node);
};

class Edge {
private:
    Node* m_Nodes[2];
    float m_Weight;

public:
    Edge(float weight = 1.0) {
        m_Weight = weight;
    }

    float GetWeight() { return m_Weight; }

    Node* GetNode(std::size_t index) { return m_Nodes[index]; }
    Node* GetConnected(Node* from);
    void LinkNodes(Node* first, Node* second);
};

class Plan {
private:
    std::vector<Node*> m_Path;
    typedef std::vector<Node*>::iterator iterator;
    iterator m_Iterator;

public:
    Plan() {
        m_Iterator = m_Path.end();
    }

    bool HasNext() {
        return m_Iterator != m_Path.end();
    }

    void Reset() {
        m_Iterator = m_Path.begin();
    }

    Node* GetCurrent() {
        if (m_Iterator == m_Path.end()) return nullptr;
        return *m_Iterator;
    }

    Node* GetGoal() {
        if (m_Path.empty()) return nullptr;
        return m_Path.back();
    }

    Node* Next() {
        Node* result = *m_Iterator;
        ++m_Iterator;
        return result;
    }

    void AddNode(Node* node) {
        m_Path.push_back(node);
    }
};

class PlanningNode {
private:
    PlanningNode* m_Prev;
    Node* m_Node;
    Node* m_Goal;
    float m_GoalCost;
    float m_HeuristicCost;
    float m_FitnessCost;
    bool m_Closed;

public:
    PlanningNode(PlanningNode* prev, Node* node, Node* goal);

    PlanningNode* GetPrevious() { return m_Prev; }
    Node* GetNode() { return m_Node; }
    Node* GetGoal() { return m_Goal; }
    float GetGoalCost() { return m_GoalCost; }
    float GetHeuristicCost() { return m_HeuristicCost; }
    float GetFitnessCost() { return m_FitnessCost; }
    bool IsClosed() { return m_Closed; }
    void SetClosed(bool closed) { m_Closed = closed; }

    void SetPrevious(PlanningNode* previous) { 
        m_Prev = previous;

        if (m_Prev) {
            m_GoalCost = m_Prev->GetGoalCost() + m_Node->GetCostFrom(m_Prev->GetNode());
        } else {
            m_GoalCost = 0;
        }

        m_HeuristicCost = (float)(m_Node->GetPosition() - m_Goal->GetPosition()).Length();
        m_FitnessCost = m_GoalCost + m_HeuristicCost;
    }

    bool IsBetterThan(PlanningNode* other) {
        return m_FitnessCost < other->GetFitnessCost();
    }
};

template <typename T, typename Compare, typename Container = std::vector<T>>
class PriorityQueue {
private:
    Container m_Container;
    Compare m_Comp;

public:
    void Push(T item) {
        m_Container.push_back(item);
        std::push_heap(m_Container.begin(), m_Container.end(), m_Comp);
    }

    T Pop() {
        T item = m_Container.front();
        std::pop_heap(m_Container.begin(), m_Container.end(), m_Comp);
        m_Container.pop_back();
        return item;
    }

    void Update() {
        std::make_heap(m_Container.begin(), m_Container.end(), m_Comp);
    }

    bool Empty() const { return m_Container.empty(); }
};

struct PlanningNodeComparator {
    bool operator()(PlanningNode* first, PlanningNode* second) {
        return !first->IsBetterThan(second);
    }
};

class AStar {
private:
    std::map<Node*, PlanningNode*> m_NodeMap;
    PriorityQueue<PlanningNode*, PlanningNodeComparator> m_OpenSet;
    Node* m_Goal;

    PlanningNode* AddToOpenSet(Node* node, PlanningNode* prev);

    // Backtrace path and reverse it to get finished plan
    Plan* BuildPath(PlanningNode* goal);

public:
    Plan* operator()(Node* start, Node* goal);
};

// Extend this and fill out nodes/edges with world data
class Graph {
protected:
    std::map<Vector3i, Node*> m_Nodes;
    std::vector<Edge*> m_Edges;

    void LinkNodes(Node* first, Node* second, float weight = 1.0);
public:
    ~Graph();

    Node* FindClosest(const Vector3i& pos);
    Plan* FindPath(const Vector3i& start, const Vector3i& end);

    void Destroy();
};

} // ns path
} // ns ai

#endif
