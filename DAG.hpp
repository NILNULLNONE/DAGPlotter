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
    void BuildCoordinate(const NodeArray& InNodes) const
    {
        DAGArray DAGs = {};
        SplitDAG(InNodes, DAGs);

        NodeArray Entries, Exists;
        NodeColumnArray Columns;
        int RowStartIndex = 0;
        for(size_t DAGIdx = 0; DAGIdx < DAGs.size(); ++DAGIdx)
        {
            auto& DAGNodes = DAGs[DAGIdx];
            Entries.clear();

//            assert(GetEntries(DAGNodes, Entries) && "Not a DAG");

            assert(GetExits(DAGNodes, Exists) && "Not a DAG");

//            assert(CheckDAGFromEntries(Entries) && "Not a DAG!");

            assert(CheckDAGFromExits(Exists) && "Not a DAG!");

//            BuildCoordXFromEntries(Entries);

            BuildCoordXFromExits(Exists);

//            RowStartIndex += BuildCoordYFromEntries(Entries, RowStartIndex);

            RowStartIndex += BuildCoordYFromExits(Exists, RowStartIndex);
        }
    }
private:
    void SplitDAG(const NodeArray& InNodes, DAGArray& OutDAGs) const
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
                BFSQueue.pop();
                Front->QueryChildren(OutChildrens);
                Front->QueryParents(OutParents);
                for(auto& Child : OutChildrens)
                {
                    assert(Child);
                    if(std::find(VisitedNodes.begin(), VisitedNodes.end(), Child) != VisitedNodes.end())continue;
                    VisitedNodes.insert(Child);
                    BFSQueue.push(Child);
                    DAG.push_back(Child);
                }
                for(auto& Parent : OutParents)
                {
                    assert(Parent);
                    if(std::find(VisitedNodes.begin(), VisitedNodes.end(), Parent) != VisitedNodes.end())continue;
                    VisitedNodes.insert(Parent);
                    BFSQueue.push(Parent);
                    DAG.push_back(Parent);
                }
            }
            OutDAGs.push_back(std::move(DAG));
        }
    }

    bool GetEntries(const NodeArray& InNodes, NodeArray& OutEntries) const
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
            const auto& Found = std::find(OutParents.begin(), OutParents.end(), Node);
            if(Found != OutParents.end())
            {
                return false;
            }
        }
        return true;
    }

    bool GetExits(const NodeArray& InNodes, NodeArray& OutExists) const
    {
        OutExists.clear();
        NodeArray OutChildrens = {};
        for(const auto& Node : InNodes)
        {
            assert(Node);
            OutChildrens.clear();
            Node->QueryChildren(OutChildrens);
            if(OutChildrens.empty())
            {
                OutExists.push_back(Node);
                continue;
            }
            // child can't be self
            const auto& Found = std::find(OutChildrens.begin(), OutChildrens.end(), Node);
            if(Found != OutChildrens.end())
            {
                return false;
            }
        }
        return true;
    }

    bool CheckDAGFromEntries(const NodeArray& InEntries) const
    {
        NodeSet VisitedNodes{}, Ancestors{};
        for(auto& Entry : InEntries)
        {
            if(!CheckDAGFromEntriesHelper(Entry, VisitedNodes, Ancestors))return false;
        }
        return true;
    }

    bool CheckDAGFromEntriesHelper(CDAGNode* InNode, NodeSet& VisitedNodes, NodeSet& Ancestors) const
    {
        assert(InNode);
        if(std::find(Ancestors.begin(), Ancestors.end(), InNode) != Ancestors.end())return false;
        if(std::find(VisitedNodes.begin(), VisitedNodes.end(), InNode) != VisitedNodes.end())
        {
            return true;
        }
        VisitedNodes.insert(InNode);
        Ancestors.insert(InNode);
        NodeArray Children = {};
        InNode->QueryChildren(Children);
        for(auto& Child : Children)
        {
            if(!CheckDAGFromEntriesHelper(Child, VisitedNodes, Ancestors))
            {
                return false;
            }
        }
        Ancestors.erase(InNode);
        return true;
    }

    bool CheckDAGFromExits(const NodeArray& InExits) const
    {
        NodeSet VisitedNodes{}, Descendants{};
        for(auto& Exit : InExits)
        {
            if(!CheckDAGFromExitsHelper(Exit, VisitedNodes, Descendants))return false;
        }
        return true;
    }

    bool CheckDAGFromExitsHelper(CDAGNode* InNode, NodeSet& VisitedNodes, NodeSet& Descendants) const
    {
        assert(InNode);
        if(std::find(Descendants.begin(), Descendants.end(), InNode) != Descendants.end())return false;
        if(std::find(VisitedNodes.begin(), VisitedNodes.end(), InNode) != VisitedNodes.end())
        {
            return true;
        }
        VisitedNodes.insert(InNode);
        Descendants.insert(InNode);
        NodeArray Parents = {};
        InNode->QueryParents(Parents);
        for(auto& Parent : Parents)
        {
            if(!CheckDAGFromExitsHelper(Parent, VisitedNodes, Descendants))
            {
                return false;
            }
        }
        Descendants.erase(InNode);
        return true;
    }

    void BuildCoordXFromEntries(const NodeArray& InEntries) const
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
            BFSQueue.pop();
            assert(Front);
            Front->QueryChildren(OutChildren);
            if(OutChildren.empty())continue;
            auto ParentCoordX = Front->Coord.first;
            for(auto & Child : OutChildren)
            {
                assert(Child);
                auto ChildCoordX = Child->Coord.first;
                auto NewCoordX = ParentCoordX + 1;
                if(NewCoordX > ChildCoordX)
                {
                    Child->Coord.first = NewCoordX;
                    BFSQueue.push(Child);
                    continue;
                }
            }
        }
    }

    void BuildCoordXFromExits(const NodeArray& InExits) const
    {
        NodeQueue BFSQueue = {};
        // set coord of entry to 0
        for(auto& Exit : InExits)
        {
            Exit->Coord = std::make_pair(0, 0);
            BFSQueue.push(Exit);
        }

        NodeArray OutParents = {};
        while(!BFSQueue.empty())
        {
            OutParents.clear();
            auto Front = BFSQueue.front();
            BFSQueue.pop();
            assert(Front);
            Front->QueryParents(OutParents);
            if(OutParents.empty())continue;
            auto ChildCoordX = Front->Coord.first;
            for(auto & Parent : OutParents)
            {
                assert(Parent);
                auto ParentCoordX = Parent->Coord.first;
                auto NewCoordX = ChildCoordX + 1;
                if(NewCoordX > ParentCoordX)
                {
                    Parent->Coord.first = NewCoordX;
                    BFSQueue.push(Parent);
                    continue;
                }
            }
        }
    }

    int BuildCoordYFromEntries(const NodeArray& InEntries, const int RowStartIndex) const
    {
        int CurrentRow = RowStartIndex;
        for(auto& Entry : InEntries)
        {
            assert(Entry);
            Entry->Coord.second = CurrentRow;
            CurrentRow += BuildCoordYFromEntriesHelper(Entry);
        }
        return CurrentRow - RowStartIndex;
    }

    int BuildCoordYFromExits(const NodeArray& InExits, const int RowStartIndex) const
    {
        int CurrentRow = RowStartIndex;
        for(auto& Exit : InExits)
        {
            assert(Exit);
            Exit->Coord.second = CurrentRow;
            CurrentRow += BuildCoordYFromExitsHelper(Exit);
        }
        return CurrentRow - RowStartIndex;
    }

    int BuildCoordYFromEntriesHelper(CDAGNode* InNode)const
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
            CurrentY += BuildCoordYFromEntriesHelper(Child);
        }
        return std::max(CurrentY - InNode->Coord.second, 1);
    }

    int BuildCoordYFromExitsHelper(CDAGNode* InNode)const
    {
        assert(InNode);
        auto CurrentY = InNode->Coord.second;
        NodeArray OutParents = {};
        InNode->QueryParents(OutParents);
        for(auto& Parent : OutParents)
        {
            assert(Parent);
            if(Parent->Coord.second != -1)continue;
            Parent->Coord.second = CurrentY;
            CurrentY += BuildCoordYFromExitsHelper(Parent);
        }
        return std::max(CurrentY - InNode->Coord.second, 1);
    }

//    void BuildColumns(const NodeArray& InNodes, NodeColumnArray& OutColumns) const;
//    void BuildCoordY(NodeColumn& InColumn, const int DAGIdx) const;
};

}
#endif // DAG_HPP
