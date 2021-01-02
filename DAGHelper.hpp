#ifndef DAGHELPER_HPP
#define DAGHELPER_HPP
#include "DAG.hpp"
namespace DAGNS
{
    struct CBaseNode : CDAGNode
    {
        NodeArray Children = {};
        NodeArray Parents = {};
        virtual void QueryChildren(NodeArray& OutChildren) const override
        {
            OutChildren = Children;
        }
        virtual void QueryParents(NodeArray &OutParents) const override
        {
            OutParents = Parents;
        }
    };

    static void MakeDAG(const std::vector<std::pair<int, int>>& Indices, NodeArray& OutDAG)
    {
        OutDAG.clear();
        constexpr int MaxNodes = 2048;
        CDAGNode* NodeMap[MaxNodes] = {};
        for(const auto& Index : Indices)
        {
            assert(Index.first < MaxNodes && Index.second < MaxNodes
                   && Index.first >= 0 && Index.second >= 0);
            assert(Index.first != Index.second);
            auto& ParentNode = NodeMap[Index.first];
            auto& ChildNode = NodeMap[Index.second];
            if(!ParentNode)
            {
                ParentNode = new CBaseNode;
                OutDAG.push_back(ParentNode);
            }
            if(!ChildNode)
            {
                ChildNode = new CBaseNode;
                OutDAG.push_back(ChildNode);
            }
            dynamic_cast<CBaseNode*>(ParentNode)->Children.push_back(ChildNode);
            dynamic_cast<CBaseNode*>(ChildNode)->Parents.push_back(ParentNode);
        }
    }

    static void PrintBaseDAG(const NodeArray& InDAG)
    {
        printf("Node Num: %llu\n", InDAG.size());
        for(auto& Node : InDAG)
        {
            NodeArray Children = {};
            NodeArray Parents = {};
            Node->QueryChildren(Children);
            Node->QueryParents(Parents);
            printf("Self: %p, (%d, %d)\n", static_cast<void*>(Node), Node->Coord.first, Node->Coord.second);
            printf("Children:\n");
            for(auto& Child : Children)
            {
                printf("\t%p, (%d, %d)\n", static_cast<void*>(Child), Child->Coord.first, Child->Coord.second);
            }
            printf("Parents:\n");
            for(auto& Parent : Parents)
            {
                printf("\t%p, (%d, %d)\n", static_cast<void*>(Parent), Parent->Coord.first, Parent->Coord.second);
            }
            printf("\n");
        }
    }
}
#endif // DAGHELPER_HPP
