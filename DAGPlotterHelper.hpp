#ifndef DAGPLOTTERHELPER_HPP
#define DAGPLOTTERHELPER_HPP
#include "DAGPlotter.hpp"
namespace DAGNS
{
    class CBaseDAGNodePlotter : public CDAGNodePlotter
    {
    public:
        QGraphicsItem * PlotNode(CDAGNode *InNode) override
        {
            QGraphicsSimpleTextItem* TextItem = new QGraphicsSimpleTextItem;
            TextItem->setText(QString("(%1,%2)").arg(InNode->Coord.first).arg(InNode->Coord.second));
            QGraphicsRectItem* BackgroundItem = new QGraphicsRectItem;
            TextItem->setFont(QFont("Times New Roman", 24));
            BackgroundItem->setRect(TextItem->boundingRect());
            TextItem->setParentItem(BackgroundItem);
            BackgroundItem->setBrush(Qt::white);
            return BackgroundItem;
            //TextItem->setBrush(Qt::white);
//            return TextItem;
        }
    };
}
#endif // DAGPLOTTERHELPER_HPP
