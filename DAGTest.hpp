#ifndef DAGTEST_HPP
#define DAGTEST_HPP
#include "DAGHelper.hpp"
#include "DAGPlotterHelper.hpp"
namespace DAGTestNS
{
    static void Run()
    {
        using namespace DAGNS;
        NodeArray DAG = {};
        MakeDAG(
            {{0, 1}, {0, 2}},
        DAG);

        MakeDAG(
            {
                {0, 1},
                {0, 2},
                {1, 3},
                {2, 3},
                {3, 4},
                {3, 5},
                {2, 5},
                {5, 6},
                {5, 7},
                {1, 7},
                {3, 7},

                {8, 9},
                {9, 10},
                {9, 11},
                {9, 12},
                {10, 13},
                {10, 14},
                {11, 15},
                {11, 16},
                {12, 17},
                {12, 18},
            },
            DAG
        );

        PrintBaseDAG(DAG);
        CDAG Tmp;
        Tmp.BuildCoordinate(DAG);
        PrintBaseDAG(DAG);

        CDAGPlotter* Plotter = new CDAGPlotter(new CBaseDAGNodePlotter);
        Plotter->PlotNodes(DAG);
        Plotter->show();
    }
}
#endif // DAGTEST_HPP
