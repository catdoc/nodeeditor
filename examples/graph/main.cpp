#include <nodes/ConnectionStyle>
#include <nodes/GraphicsView>
#include <nodes/NodeGraphicsScene>
#include <nodes/StyleCollection>

#include <QtWidgets/QApplication>
#include <QtWidgets/QDesktopWidget>

#include "CustomGraphModel.hpp"


using QtNodes::ConnectionStyle;
using QtNodes::GraphicsView;
using QtNodes::NodeGraphicsScene;
using QtNodes::NodeRole;
using QtNodes::StyleCollection;

int
main(int argc, char *argv[])
{
  QApplication app(argc, argv);

  CustomGraphModel graphModel;

  NodeId id1 = graphModel.addNode();
  NodeId id2 = graphModel.addNode();

  graphModel.setNodeData(id1, NodeRole::Position, QPointF(0, 0));
  graphModel.setNodeData(id2, NodeRole::Position, QPointF(300, 300));

  auto scene = new NodeGraphicsScene(graphModel);

  GraphicsView view(scene);

  view.setWindowTitle("Simplest model-based graph");
  view.resize(800, 600);

  view.move(QApplication::desktop()->screen()->rect().center() - view.rect().center());
  view.showNormal();
  view.centerScene();

  return app.exec();
}
