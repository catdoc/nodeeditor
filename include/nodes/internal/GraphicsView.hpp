#pragma once

#include <QtWidgets/QGraphicsView>

#include "Export.hpp"

namespace QtNodes
{

class NodeGraphicsScene;

class NODE_EDITOR_PUBLIC GraphicsView
  : public QGraphicsView
{
  Q_OBJECT
public:
  GraphicsView(QWidget *parent = Q_NULLPTR);
  GraphicsView(NodeGraphicsScene *scene, QWidget *parent = Q_NULLPTR);

  GraphicsView(const GraphicsView &) = delete;
  GraphicsView operator=(const GraphicsView &) = delete;

  QAction* clearSelectionAction() const;

  QAction* deleteSelectionAction() const;

  void setScene(NodeGraphicsScene *scene);

public Q_SLOTS:

  void scaleUp();

  void scaleDown();

  void deleteSelectedNodes();

protected:

  void contextMenuEvent(QContextMenuEvent *event) override;

  void wheelEvent(QWheelEvent *event) override;

  void keyPressEvent(QKeyEvent *event) override;

  void keyReleaseEvent(QKeyEvent *event) override;

  void mousePressEvent(QMouseEvent *event) override;

  void mouseMoveEvent(QMouseEvent *event) override;

  void drawBackground(QPainter* painter, const QRectF & r) override;

  void showEvent(QShowEvent *event) override;

protected:

  NodeGraphicsScene * scene();

private:

  QAction* _clearSelectionAction;
  QAction* _deleteSelectionAction;

  QPointF _clickPos;

  NodeGraphicsScene* _scene;
};
}
