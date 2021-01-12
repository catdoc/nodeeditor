#include <nodes/ConnectionStyle>
#include <nodes/GraphicsView>
#include <nodes/NodeGraphicsScene>
#include <nodes/StyleCollection>

#include <QtWidgets/QApplication>
#include <QtWidgets/QDesktopWidget>
#include <QtWidgets/QAction>

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

  // Initialize two nodes.
  {
    NodeId id1 = graphModel.addNode();
    NodeId id2 = graphModel.addNode();

    graphModel.setNodeData(id1, NodeRole::Position, QPointF(0, 0));
    graphModel.setNodeData(id2, NodeRole::Position, QPointF(300, 300));
  }

  auto scene = new NodeGraphicsScene(graphModel);

  GraphicsView view(scene);

  // Setup context menu for creating new nodes.
  view.setContextMenuPolicy(Qt::ActionsContextMenu);
  QAction createNodeAction(QStringLiteral("Create Node"), &view);
  QObject::connect(&createNodeAction, &QAction::triggered,
                   [&]()
                   {
                     // Mouse position in scene coordinates.
                     QPointF posView =
                       view.mapToScene(view.mapFromGlobal(QCursor::pos()));


                     NodeId const newId = graphModel.addNode();
                     graphModel.setNodeData(newId,
                                            NodeRole::Position,
                                            posView);

                     scene->createNode(newId);
                   });
  view.insertAction(view.actions().front(), &createNodeAction);


  view.setWindowTitle("Simple Node Graph");
  view.resize(800, 600);

  // Center window.
  view.move(QApplication::desktop()->screen()->rect().center() - view.rect().center());
  view.showNormal();

  return app.exec();
}

