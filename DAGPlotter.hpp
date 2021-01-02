#ifndef DAGPLOTTER_HPP
#define DAGPLOTTER_HPP
#include "DAG.hpp"

#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QGraphicsLineItem>
#include <QVBoxLayout>
namespace DAGNS
{
    class CDAGNodePlotter
    {
    public:
        virtual QGraphicsItem* PlotNode(CDAGNode* InNode) = 0;
        virtual QMarginsF GetNodeMargins() {return QMarginsF(5, 5, 5, 5);}
        virtual ~CDAGNodePlotter() {}
    };

    class CDAGPlotter : public QWidget
    {
    protected:
        QGraphicsView* View = new QGraphicsView;
        QGraphicsScene* Scene = new QGraphicsScene;
        CDAGNodePlotter* Plotter = nullptr;
    public:
        CDAGPlotter(CDAGNodePlotter* InPlotter = nullptr) ;
        virtual void PlotNodes(const NodeArray& InNodes, CDAGNodePlotter* InPlotter = nullptr);
        virtual ~CDAGPlotter(){}
    };

    CDAGPlotter::CDAGPlotter(CDAGNodePlotter* InPlotter): Plotter(InPlotter)
    {
        View->setScene(Scene);
        QVBoxLayout* Layout = new QVBoxLayout();
        this->setLayout(Layout);
        Layout->addWidget(View);
    }

    void CDAGPlotter::PlotNodes(const NodeArray &InNodes, CDAGNodePlotter* InPlotter)
    {
        if(!InPlotter)
        {
            InPlotter = Plotter;
        }
        assert(InPlotter);
        Scene->clear();
        using NodeItemArray = std::vector<QGraphicsItem*>;
        NodeItemArray NodeItems = {};
        constexpr int MaxSize = 16384;
        qreal MaxRowSize[MaxSize] = {};
        qreal MaxColSize[MaxSize] = {};
        qreal RowPosition[MaxSize] = {};
        qreal ColPosition[MaxSize] = {};
        NodeCoord MaxCoord = {0, 0};
        auto NodeMargin = InPlotter->GetNodeMargins();
        std::map<CDAGNode*, QSizeF> NodeItemSizes = {};
        for(auto& Node : InNodes)
        {
            assert(Node);
            auto NodeItem = InPlotter->PlotNode(Node);
            NodeItems.push_back(NodeItem);
            const auto& Coord = Node->Coord;
            assert(Coord.first >= 0 && Coord.first < MaxSize
                   && Coord.second >= 0 && Coord.second < MaxSize);
            MaxCoord.first = std::max(MaxCoord.first, Coord.first);
            MaxCoord.second = std::max(MaxCoord.second, Coord.second);
            auto Bounding = NodeItem->boundingRect();
            MaxRowSize[Coord.second] = std::max(MaxRowSize[Coord.second], Bounding.height() + NodeMargin.top() + NodeMargin.bottom());
            MaxColSize[Coord.first] = std::max(MaxColSize[Coord.first], Bounding.width() + NodeMargin.left() + NodeMargin.right());
            NodeItemSizes[Node] = {Bounding.width(), Bounding.height()};
        }
        // sum up
        for(int i = 0; i < MaxCoord.first; i++)
        {
            ColPosition[i+1] = ColPosition[i] + MaxColSize[i];
        }
        for(int i = 0; i < MaxCoord.second; i++)
        {
            RowPosition[i+1] = RowPosition[i] + MaxRowSize[i];
        }

        auto GetPos = [&](auto* InNode)->auto
        {
            auto& Coord = InNode->Coord;
            auto ColPos = ColPosition[Coord.first] + NodeMargin.left();
            auto RowPos = RowPosition[Coord.second] + NodeMargin.top();
            return QPointF{ColPos, RowPos};
        };

        auto GetCenterPos = [&](auto* InNode)->auto
        {
            auto Pos = GetPos(InNode);
            auto& Bounding = NodeItemSizes[InNode];
            return Pos + QPointF{Bounding.width(), Bounding.height()}/2.0;
        };

        NodeArray Children = {};
        for(size_t i = 0; i < InNodes.size(); i++)
        {
            auto Node = InNodes[i];
            auto NodeItem = NodeItems[i];
            auto ParentPos = GetPos(Node);
            NodeItem->setPos(ParentPos);
            Scene->addItem(NodeItem);

            auto ParentCenterPos = GetCenterPos(Node);
            Node->QueryChildren(Children);
            for(auto& Child : Children)
            {
                auto ChildCenterPos = GetCenterPos(Child);
                auto Link = new QGraphicsLineItem(QLineF{ParentCenterPos, ChildCenterPos});
                Link->setZValue(-2);
                Scene->addItem(Link);
            }
        }

    }
}
#endif // DAGPLOTTER_HPP
