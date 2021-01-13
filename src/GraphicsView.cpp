#include "GraphicsView.hpp"

#include <QtWidgets/QGraphicsScene>

#include <QtGui/QPen>
#include <QtGui/QBrush>
#include <QtWidgets/QMenu>

#include <QtCore/QRectF>
#include <QtCore/QPointF>
#include <QtCore/QDebug>

#include <QtOpenGL>
#include <QtWidgets>

#include <iostream>
#include <cmath>

#include "ConnectionGraphicsObject.hpp"
#include "NodeGraphicsObject.hpp"
#include "BasicGraphicsScene.hpp"
#include "StyleCollection.hpp"

using QtNodes::GraphicsView;
using QtNodes::BasicGraphicsScene;

GraphicsView::
GraphicsView(QWidget *parent)
  : QGraphicsView(parent)
  , _clearSelectionAction(Q_NULLPTR)
  , _deleteSelectionAction(Q_NULLPTR)
{
  setDragMode(QGraphicsView::ScrollHandDrag);
  setRenderHint(QPainter::Antialiasing);

  auto const &flowViewStyle = StyleCollection::flowViewStyle();

  setBackgroundBrush(flowViewStyle.BackgroundColor);

  //setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
  //setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate);
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

  setCacheMode(QGraphicsView::CacheBackground);

  //setViewport(new QGLWidget(QGLFormat(QGL::SampleBuffers)));
}


GraphicsView::
GraphicsView(BasicGraphicsScene *scene, QWidget *parent)
  : GraphicsView(parent)
{
  setScene(scene);
}


QAction*
GraphicsView::
clearSelectionAction() const
{
  return _clearSelectionAction;
}


QAction*
GraphicsView::
deleteSelectionAction() const
{
  return _deleteSelectionAction;
}


void
GraphicsView::setScene(BasicGraphicsScene *scene)
{
  QGraphicsView::setScene(scene);

  // setup actions
  delete _clearSelectionAction;
  _clearSelectionAction = new QAction(QStringLiteral("Clear Selection"), this);
  _clearSelectionAction->setShortcut(Qt::Key_Escape);
  connect(_clearSelectionAction, &QAction::triggered,
          scene, &QGraphicsScene::clearSelection);
  addAction(_clearSelectionAction);

  delete _deleteSelectionAction;
  _deleteSelectionAction = new QAction(QStringLiteral("Delete Selection"), this);
  _deleteSelectionAction->setShortcut(Qt::Key_Delete);
  connect(_deleteSelectionAction, &QAction::triggered,
          this, &GraphicsView::deleteSelectedNodes);
  addAction(_deleteSelectionAction);
}


void
GraphicsView::
centerScene()
{
  if (scene())
  {
    scene()->setSceneRect(QRectF());

    QRectF sceneRect = scene()->sceneRect();

    if (sceneRect.width() > this->rect().width() ||
        sceneRect.height() > this->rect().height())
    {
      fitInView(sceneRect, Qt::KeepAspectRatio);
    }

    centerOn(sceneRect.center());
  }
}


void
GraphicsView::
contextMenuEvent(QContextMenuEvent *event)
{
  if (itemAt(event->pos()))
  {
    QGraphicsView::contextMenuEvent(event);
    return;
  }

  //QMenu modelMenu;

  //auto skipText = QStringLiteral("skip me");

  ////Add filterbox to the context menu
  //auto *txtBox = new QLineEdit(&modelMenu);

  //txtBox->setPlaceholderText(QStringLiteral("Filter"));
  //txtBox->setClearButtonEnabled(true);

  //auto *txtBoxAction = new QWidgetAction(&modelMenu);
  //txtBoxAction->setDefaultWidget(txtBox);

  //modelMenu.addAction(txtBoxAction);

  ////Add result treeview to the context menu
  //auto *treeView = new QTreeWidget(&modelMenu);
  //treeView->header()->close();

  //auto *treeViewAction = new QWidgetAction(&modelMenu);
  //treeViewAction->setDefaultWidget(treeView);

  //modelMenu.addAction(treeViewAction);

  //QMap<QString, QTreeWidgetItem*> topLevelItems;
  //for (auto const & cat : _scene->registry().categories())
  //{
  //auto item = new QTreeWidgetItem(treeView);
  //item->setText(0, cat);
  //item->setData(0, Qt::UserRole, skipText);
  //topLevelItems[cat] = item;
  //}

  //for (auto const & assoc : _scene->registry().registeredModelsCategoryAssociation())
  //{
  //auto parent = topLevelItems[assoc.second];
  //auto item   = new QTreeWidgetItem(parent);
  //item->setText(0, assoc.first);
  //item->setData(0, Qt::UserRole, assoc.first);
  //}

  //treeView->expandAll();

#if 0
  connect(treeView, &QTreeWidget::itemClicked,
          [&](QTreeWidgetItem *item, int)
          {
            QString modelName = item->data(0, Qt::UserRole).toString();

            if (modelName == skipText)
            {
              return;
            }

            auto type = _scene->registry().create(modelName);

            if (type)
            {
              auto &node = _scene->createNode(std::move(type));

              QPoint pos = event->pos();

              QPointF posView = this->mapToScene(pos);

              node.nodeGraphicsObject().setPos(posView);

              _scene->nodePlaced(node);
            }
            else
            {
              qDebug() << "Model not found";
            }

            modelMenu.close();
          });

#endif

  ////Setup filtering
  //connect(txtBox, &QLineEdit::textChanged,
  //[&](const QString & text)
  //{
  //for (auto & topLvlItem : topLevelItems)
  //{
  //for (int i = 0; i < topLvlItem->childCount(); ++i)
  //{
  //auto child       = topLvlItem->child(i);
  //auto modelName   = child->data(0, Qt::UserRole).toString();
  //const bool match = (modelName.contains(text, Qt::CaseInsensitive));
  //child->setHidden(!match);
  //}
  //}
  //});

  //// make sure the text box gets focus so the user doesn't have to click on it
  //txtBox->setFocus();

  //modelMenu.exec(event->globalPos());
}


void
GraphicsView::
wheelEvent(QWheelEvent *event)
{
  QPoint delta = event->angleDelta();

  if (delta.y() == 0)
  {
    event->ignore();
    return;
  }

  double const d = delta.y() / std::abs(delta.y());

  if (d > 0.0)
    scaleUp();
  else
    scaleDown();
}


void
GraphicsView::
scaleUp()
{
  double const step   = 1.2;
  double const factor = std::pow(step, 1.0);

  QTransform t = transform();

  if (t.m11() > 2.0)
    return;

  scale(factor, factor);
}


void
GraphicsView::
scaleDown()
{
  double const step   = 1.2;
  double const factor = std::pow(step, -1.0);

  scale(factor, factor);
}


void
GraphicsView::
deleteSelectedNodes()
{
  // Delete the selected connections first, ensuring that they won't be
  // automatically deleted when selected nodes are deleted (deleting a
  // node deletes some connections as well)
  for (QGraphicsItem * item : scene()->selectedItems())
  {
    if (auto c = qgraphicsitem_cast<ConnectionGraphicsObject*>(item))
      nodeScene()->deleteConnection(c->connectionId());
  }

  // Delete the nodes; this will delete many of the connections.
  // Selected connections were already deleted prior to this loop,
  // otherwise qgraphicsitem_cast<NodeGraphicsObject*>(item) could be a
  // use-after-free when a selected connection is deleted by deleting
  // the node.
  for (QGraphicsItem * item : scene()->selectedItems())
  {
    if (auto n = qgraphicsitem_cast<NodeGraphicsObject*>(item))
      nodeScene()->deleteNode(n->nodeId());
  }
}


void
GraphicsView::
keyPressEvent(QKeyEvent *event)
{
  switch (event->key())
  {
    case Qt::Key_Shift:
      setDragMode(QGraphicsView::RubberBandDrag);
      break;

    default:
      break;
  }

  QGraphicsView::keyPressEvent(event);
}


void
GraphicsView::
keyReleaseEvent(QKeyEvent *event)
{
  switch (event->key())
  {
    case Qt::Key_Shift:
      setDragMode(QGraphicsView::ScrollHandDrag);
      break;

    default:
      break;
  }
  QGraphicsView::keyReleaseEvent(event);
}


void
GraphicsView::
mousePressEvent(QMouseEvent *event)
{
  QGraphicsView::mousePressEvent(event);
  if (event->button() == Qt::LeftButton)
  {
    _clickPos = mapToScene(event->pos());
  }
}


void
GraphicsView::
mouseMoveEvent(QMouseEvent *event)
{
  QGraphicsView::mouseMoveEvent(event);
  if (scene()->mouseGrabberItem() == nullptr && event->buttons() == Qt::LeftButton)
  {
    // Make sure shift is not being pressed
    if ((event->modifiers() & Qt::ShiftModifier) == 0)
    {
      QPointF difference = _clickPos - mapToScene(event->pos());
      setSceneRect(sceneRect().translated(difference.x(), difference.y()));
    }
  }
}


void
GraphicsView::
drawBackground(QPainter* painter, const QRectF &r)
{
  QGraphicsView::drawBackground(painter, r);

  auto drawGrid =
    [&](double gridStep)
    {
      QRect windowRect = rect();
      QPointF tl       = mapToScene(windowRect.topLeft());
      QPointF br       = mapToScene(windowRect.bottomRight());

      double left   = std::floor(tl.x() / gridStep - 0.5);
      double right  = std::floor(br.x() / gridStep + 1.0);
      double bottom = std::floor(tl.y() / gridStep - 0.5);
      double top    = std::floor(br.y() / gridStep + 1.0);

      // vertical lines
      for (int xi = int(left); xi <= int(right); ++xi)
      {
        QLineF line(xi * gridStep, bottom * gridStep,
                    xi * gridStep, top * gridStep);

        painter->drawLine(line);
      }

      // horizontal lines
      for (int yi = int(bottom); yi <= int(top); ++yi)
      {
        QLineF line(left * gridStep, yi * gridStep,
                    right * gridStep, yi * gridStep);
        painter->drawLine(line);
      }
    };

  auto const &flowViewStyle = StyleCollection::flowViewStyle();

  QPen pfine(flowViewStyle.FineGridColor, 1.0);

  painter->setPen(pfine);
  drawGrid(15);

  QPen p(flowViewStyle.CoarseGridColor, 1.0);

  painter->setPen(p);
  drawGrid(150);
}


void
GraphicsView::
showEvent(QShowEvent *event)
{
  QGraphicsView::showEvent(event);

  scene()->setSceneRect(this->rect());
  centerScene();
}


BasicGraphicsScene *
GraphicsView::
nodeScene()
{
  return dynamic_cast<BasicGraphicsScene*>(scene());
}

