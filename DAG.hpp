#ifndef DAG_HPP
#define DAG_HPP
#include <vector>
#include <set>
#include <queue>
#include <algorithm>
#include <cassert>
namespace DAGNS{

struct CDAGNode;
using NodeCoord = std::pair<int, int>;
using NodeArray = std::vector<CDAGNode*>;
using NodeSet = std::set<CDAGNode*>;
using NodeQueue = std::queue<CDAGNode*>;
using NodeColumn = NodeArray;
using NodeColumnArray = std::vector<NodeColumn>;
using DAGArray = std::vector<NodeArray>;

struct CDAGNode
{
    NodeCoord Coord = std::make_pair(-1, -1);
    virtual void QueryChildren(NodeArray& OutChildren) const = 0;
    virtual void QueryParents(NodeArray& OutParents) const = 0;
    virtual ~CDAGNode(){}
};

struct CDAG
{
public:
    void BuildCoordinate(const NodeArray& InNodes) const;
private:
    void SplitDAG(const NodeArray& InNodes, DAGArray& OutDAGs) const;
    bool GetEntries(const NodeArray& InNodes, NodeArray& OutEntries) const;
    bool CheckDAG(const NodeArray& InEntries) const;
    void BuildCoordX(const NodeArray& InEntries) const;
    int BuildCoordY(const NodeArray& InEntries, const int RowStartIndex) const;
    int BuildCoordYHelper(CDAGNode* InNode)const;
//    void BuildColumns(const NodeArray& InNodes, NodeColumnArray& OutColumns) const;
//    void BuildCoordY(NodeColumn& InColumn, const int DAGIdx) const;
};

void CDAG::BuildCoordinate(const NodeArray &InNodes) const
{
    DAGArray DAGs = {};
    SplitDAG(InNodes, DAGs);

    NodeArray Entries;
    NodeColumnArray Columns;
    int RowStartIndex = 0;
    for(size_t DAGIdx = 0; DAGIdx < DAGs.size(); ++DAGIdx)
    {
        auto& DAGNodes = DAGs[DAGIdx];
        Entries.clear();

        assert(GetEntries(DAGNodes, Entries) && "Not a DAG");

        assert(CheckDAG(Entries) && "Not a DAG!");

        BuildCoordX(Entries);

        RowStartIndex += BuildCoordY(Entries, RowStartIndex);

//        Columns.clear();
//        BuildColumns(DAGNodes, Columns);

//        for(auto& Column : Columns)
//        {
//            BuildCoordY(Column, static_cast<int>(DAGIdx));
//        }
    }
}

void CDAG::SplitDAG(const NodeArray &InNodes, DAGArray &OutDAGs) const
{
    OutDAGs.clear();
    NodeQueue BFSQueue = {};
    NodeSet VisitedNodes = {};
    NodeArray OutChildrens = {};
    NodeArray OutParents = {};
    NodeArray DAG = {};
    for(auto& Node : InNodes)
    {
        assert(Node);
        if(std::find(VisitedNodes.begin(), VisitedNodes.end(), Node) != VisitedNodes.end())continue;
        DAG.clear();
        VisitedNodes.insert(Node);
        BFSQueue = {};
        OutChildrens.clear();
        BFSQueue.push(Node);
        DAG.push_back(Node);
        while(!BFSQueue.empty())
        {
            auto Front = BFSQueue.front();
            Front->QueryChildren(OutChildrens);
            Front->QueryParents(OutParents);
            for(auto& Child : OutChildrens)
            {
                assert(Child);
                if(std::find(VisitedNodes.begin(), VisitedNodes.end(), Child) != VisitedNodes.end())continue;
                VisitedNodes.insert(Child);
                BFSQueue.push(Child);
                DAG.push_back(Node);
            }
            for(auto& Parent : OutParents)
            {
                assert(Parent);
                if(std::find(VisitedNodes.begin(), VisitedNodes.end(), Parent) != VisitedNodes.end())continue;
                VisitedNodes.insert(Parent);
                BFSQueue.push(Parent);
                DAG.push_back(Node);
            }
        }
        OutDAGs.push_back(std::move(DAG));
    }
}

bool CDAG::GetEntries(const NodeArray &InNodes, NodeArray &OutEntries) const
{
    OutEntries.clear();
    NodeArray OutParents = {};
    for(const auto& Node : InNodes)
    {
        assert(Node);
        OutParents.clear();
        Node->QueryParents(OutParents);
        if(OutParents.empty())
        {
            OutEntries.push_back(Node);
            continue;
        }
        // parent can't be self
        const auto& Found = std::find(InNodes.begin(), InNodes.end(), Node);
        if(Found != InNodes.end())
        {
            return false;
        }
    }
}

bool CDAG::CheckDAG(const NodeArray &InEntries) const
{
    NodeSet VisitedNodes = {};
    NodeQueue BFSQueue = {};
    for(auto Entry : InEntries)
    {
        assert(Entry);
        BFSQueue.push(Entry);
        VisitedNodes.insert(Entry);
    }
    NodeArray OutChildren = {};
    while (!BFSQueue.empty())
    {
        OutChildren.clear();
        auto Front = BFSQueue.front();
        assert(Front);
        BFSQueue.pop();
        Front->QueryChildren(OutChildren);
        if(OutChildren.empty())continue;
        for(auto& Child : OutChildren)
        {
            // has cycle
            if(std::find(VisitedNodes.begin(), VisitedNodes.end(), Child) != VisitedNodes.end())
            {
                return false;
            }
            VisitedNodes.insert(Child);
            BFSQueue.push(Child);
        }
    }
    return true;
}

void CDAG::BuildCoordX(const NodeArray &InEntries) const
{
    NodeQueue BFSQueue = {};
    // set coord of entry to 0
    for(auto& Entry : InEntries)
    {
        Entry->Coord = std::make_pair(0, 0);
        BFSQueue.push(Entry);
    }

    NodeArray OutChildren = {};
    while(!BFSQueue.empty())
    {
        OutChildren.clear();
        auto Front = BFSQueue.front();
        assert(Front);
        Front->QueryChildren(OutChildren);
        if(OutChildren.empty())continue;
        auto ParentCoordX = Front->Coord.first;
        for(auto & Child : OutChildren)
        {
            assert(Child);
            auto ChildCoordX = Child->Coord.first;
            auto NewCoordX = ParentCoordX;
            if(NewCoordX > ChildCoordX)
            {
                Child->Coord.first = NewCoordX;
                BFSQueue.push(Child);
                continue;
            }
        }
    }
}

int CDAG::BuildCoordY(const NodeArray &InEntries, const int RowStartIndex) const
{
    int CurrentRow = RowStartIndex;
    for(auto& Entry : InEntries)
    {
        assert(Entry);
        Entry->Coord.second = CurrentRow;
        CurrentRow += BuildCoordYHelper(Entry);
    }
    return CurrentRow - RowStartIndex;
}

int CDAG::BuildCoordYHelper(CDAGNode *InNode) const
{
    assert(InNode);
    auto CurrentY = InNode->Coord.second;
    NodeArray OutChildren = {};
    InNode->QueryChildren(OutChildren);
    for(auto& Child : OutChildren)
    {
        assert(Child);
        if(Child->Coord.second != -1)continue;
        Child->Coord.second = CurrentY;
        CurrentY += BuildCoordYHelper(Child);
    }
    return std::max(CurrentY - InNode->Coord.second, 1);
}

//void CDAG::BuildColumns(const NodeArray &InNodes, NodeColumnArray &OutColumns) const
//{
//    OutColumns.clear();
//    for(auto& Node : InNodes)
//    {
//        auto CoordX = static_cast<size_t>(Node->Coord.first);
//        if(CoordX >= OutColumns.size())
//        {
//            OutColumns.resize(CoordX+1, {});
//        }
//        OutColumns[CoordX].push_back(Node);
//    }
//}

//void CDAG::BuildCoordY(NodeColumn &InColumn, int DAGIndex) const
//{

//}

}
#endif // DAG_HPP
