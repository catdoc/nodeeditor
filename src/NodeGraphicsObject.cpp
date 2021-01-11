#include "NodeGraphicsObject.hpp"

#include <iostream>
#include <cstdlib>

#include <QtWidgets/QtWidgets>
#include <QtWidgets/QGraphicsEffect>

#include "ConnectionGraphicsObject.hpp"
#include "ConnectionIdUtils.hpp"
#include "ConnectionState.hpp"
#include "NodeConnectionInteraction.hpp"
#include "NodeGeometry.hpp"
#include "NodeGraphicsScene.hpp"
#include "NodePainter.hpp"
#include "StyleCollection.hpp"

using QtNodes::NodeGraphicsObject;
using QtNodes::NodeId;
using QtNodes::NodeGraphicsScene;

NodeGraphicsObject::
NodeGraphicsObject(NodeGraphicsScene & scene,
                   NodeId nodeId)
  : _nodeId(nodeId)
  , _nodeState(*this)
  , _proxyWidget(nullptr)
{
  scene.addItem(this);

  setFlag(QGraphicsItem::ItemDoesntPropagateOpacityToChildren, true);
  setFlag(QGraphicsItem::ItemIsFocusable,                      true);
  setFlag(QGraphicsItem::ItemIsMovable,                        true);
  setFlag(QGraphicsItem::ItemIsSelectable,                     true);
  setFlag(QGraphicsItem::ItemSendsScenePositionChanges,        true);

  setCacheMode(QGraphicsItem::DeviceCoordinateCache);

  // TODO: Take style from model.
  auto const & nodeStyle = StyleCollection::nodeStyle();

  {
    auto effect = new QGraphicsDropShadowEffect;
    effect->setOffset(4, 4);
    effect->setBlurRadius(20);
    effect->setColor(nodeStyle.ShadowColor);

    setGraphicsEffect(effect);
  }

  setOpacity(nodeStyle.Opacity);

  setAcceptHoverEvents(true);

  setZValue(0);

  embedQWidget();

  NodeGeometry geometry(*this);
  geometry.recalculateSize();

  //
  GraphModel & model = nodeScene()->graphModel();
  QPointF const pos =
    model.nodeData(_nodeId, NodeRole::Position).value<QPointF>();

  setPos(pos);

  // connect to the move signals to emit the move signals in NodeGraphicsScene
  //auto onMoveSlot = [this] { _scene.nodeMoved(_nodeId, pos()); };

  //connect(this, &QGraphicsObject::xChanged, this, onMoveSlot);
  //connect(this, &QGraphicsObject::yChanged, this, onMoveSlot);
}


NodeGraphicsObject::
~NodeGraphicsObject()
{
}


NodeGraphicsScene *
NodeGraphicsObject::
nodeScene() const
{
  return dynamic_cast<NodeGraphicsScene*>(scene());
}


void
NodeGraphicsObject::
embedQWidget()
{
  NodeGeometry geom(*this);

  GraphModel const & model = nodeScene()->graphModel();

  if (auto w = model.nodeData(_nodeId, NodeRole::Widget).value<QWidget*>())
  {
    _proxyWidget = new QGraphicsProxyWidget(this);

    _proxyWidget->setWidget(w);

    _proxyWidget->setPreferredWidth(5);

    if (w->sizePolicy().verticalPolicy() & QSizePolicy::ExpandFlag)
    {
      // If the widget wants to use as much vertical space as possible, set
      // it to have the geom's equivalentWidgetHeight.
      _proxyWidget->setMinimumHeight(geom.equivalentWidgetHeight());
    }

    _proxyWidget->setPos(geom.widgetPosition());

    update();

    _proxyWidget->setOpacity(1.0);
    _proxyWidget->setFlag(QGraphicsItem::ItemIgnoresParentOpacity);
  }
}


QRectF
NodeGraphicsObject::
boundingRect() const
{
  return NodeGeometry(*this).boundingRect();
}


void
NodeGraphicsObject::
setGeometryChanged()
{
  prepareGeometryChange();
}


void
NodeGraphicsObject::
moveConnections() const
{
  GraphModel const & model = nodeScene()->graphModel();

  auto moveConns =
    [&model, this](PortType portType, NodeRole nodeRole)
    {
      size_t const n =
        model.nodeData(_nodeId, nodeRole).toUInt();

      for (PortIndex portIndex = 0; portIndex < n; ++portIndex)
      {
        auto const & connectedNodes =
          model.connectedNodes(_nodeId, portType, portIndex);

        for (auto & cn: connectedNodes)
        {
          // out node id, out port index, in node id, in port index.
          ConnectionId connectionId =
            (portType == PortType::In) ?
            std::make_tuple(cn.first, cn.second, _nodeId, portIndex) :
            std::make_tuple(_nodeId, portIndex, cn.first, cn.second);

          auto cgo = nodeScene()->connectionGraphicsObject(connectionId);

          // TODO: Directly move the connection's end?
          cgo->move();
        }
      }
    };

  moveConns(PortType::In, NodeRole::NumberOfInPorts);
  moveConns(PortType::Out, NodeRole::NumberOfOutPorts);
}


void
NodeGraphicsObject::
paint(QPainter * painter,
      QStyleOptionGraphicsItem const * option,
      QWidget *)
{
  painter->setClipRect(option->exposedRect);

  NodePainter::paint(painter, *this);
}


QVariant
NodeGraphicsObject::
itemChange(GraphicsItemChange change, const QVariant & value)
{
  if (change == ItemPositionChange && scene())
  {
    moveConnections();
  }

  return QGraphicsItem::itemChange(change, value);
}


void
NodeGraphicsObject::
mousePressEvent(QGraphicsSceneMouseEvent * event)
{
  //if (_nodeState.locked())
  //return;

  // deselect all other items after this one is selected
  if (!isSelected() &&
      !(event->modifiers() & Qt::ControlModifier))
  {
    scene()->clearSelection();
  }

  NodeGraphicsScene * nodeScene = this->nodeScene();

  for (PortType portToCheck: {PortType::In, PortType::Out})
  {
    NodeGeometry nodeGeometry(*this);

    PortIndex const portIndex =
      nodeGeometry.checkHitScenePoint(portToCheck,
                                      event->scenePos(),
                                      sceneTransform());

    if (portIndex != InvalidPortIndex)
    {
      GraphModel const & model = nodeScene->graphModel();

      auto const & connectedNodes =
        model.connectedNodes(_nodeId, portToCheck, portIndex);

      // Start dragging existing connection.
      if (!connectedNodes.empty() && portToCheck == PortType::In)
      {
        auto const & cn = *connectedNodes.begin();

        // Need "reversed" connectin id if enabled for both port types.
        ConnectionId connectionId =
          std::make_tuple(cn.first, cn.second, _nodeId, portIndex);

        // Need ConnectionGraphicsObject

        NodeConnectionInteraction
          interaction(*this,
                      *nodeScene->connectionGraphicsObject(connectionId),
                      *nodeScene);

        interaction.disconnect(portToCheck);

        if (nodeScene->draftConnection())
          nodeScene->draftConnection()->update();
      }
      else // initialize new Connection
      {
        if (portToCheck == PortType::Out)
        {
          GraphModel const & model = nodeScene->graphModel();

          auto const outPolicy =
            model.portData(_nodeId,
                           portToCheck,
                           portIndex,
                           PortRole::ConnectionPolicy).value<ConnectionPolicy>();

          if (!connectedNodes.empty() &&
              outPolicy == ConnectionPolicy::One)
          {
            for (auto & cn : connectedNodes)
            {
              ConnectionId connectionId =
                std::make_tuple(cn.first, cn.second, _nodeId, portIndex);

              nodeScene->deleteConnection(connectionId);
            }
          }
        } // if port == out

        ConnectionId const incompleteConnectionId =
          makeIncompleteConnectionId(portToCheck, _nodeId, portIndex);

        nodeScene->makeDraftConnection(incompleteConnectionId);
      }
    }
  }

  //auto pos     = event->pos();
  //auto & geom  = _node.nodeGeometry();
  //auto & state = _node.nodeState();

  //if (_node.nodeDataModel()->resizable() &&
  //geom.resizeRect().contains(QPoint(pos.x(),
  //pos.y())))
  //{
  //state.setResizing(true);
  //}
}


void
NodeGraphicsObject::
mouseMoveEvent(QGraphicsSceneMouseEvent * event)
{
  //NodeGeometry geometry(*this);

  if (_nodeState.resizing())
  {
    //auto diff = event->pos() - event->lastPos();

    //if (auto w = _node.nodeDataModel()->embeddedWidget())
    //{
    //prepareGeometryChange();

    //auto oldSize = w->size();

    //oldSize += QSize(diff.x(), diff.y());

    //w->setFixedSize(oldSize);

    //_proxyWidget->setMinimumSize(oldSize);
    //_proxyWidget->setMaximumSize(oldSize);
    //_proxyWidget->setPos(geom.widgetPosition());

    //geom.recalculateSize();
    //update();

    //moveConnections();

    //event->accept();
    //}
  }
  else
  {
    QGraphicsObject::mouseMoveEvent(event);

    if (event->lastPos() != event->pos())
      moveConnections();

    event->ignore();
  }

  QRectF r = scene()->sceneRect();

  r = r.united(mapToScene(boundingRect()).boundingRect());

  scene()->setSceneRect(r);
}


void
NodeGraphicsObject::
mouseReleaseEvent(QGraphicsSceneMouseEvent * event)
{
  _nodeState.setResizing(false);

  QGraphicsObject::mouseReleaseEvent(event);

  // position connections precisely after fast node move
  moveConnections();
}


void
NodeGraphicsObject::
hoverEnterEvent(QGraphicsSceneHoverEvent * event)
{
  // bring all the colliding nodes to background
  QList<QGraphicsItem *> overlapItems = collidingItems();

  for (QGraphicsItem * item : overlapItems)
  {
    if (item->zValue() > 0.0)
    {
      item->setZValue(0.0);
    }
  }

  // bring this node forward
  setZValue(1.0);

  _nodeState.setHovered(true);

  update();

  // Signal
  nodeScene()->nodeHovered(_nodeId, event->screenPos());

  event->accept();
}


void
NodeGraphicsObject::
hoverLeaveEvent(QGraphicsSceneHoverEvent * event)
{
  _nodeState.setHovered(false);

  update();

  // Signal
  nodeScene()->nodeHoverLeft(_nodeId);

  event->accept();
}


void
NodeGraphicsObject::
hoverMoveEvent(QGraphicsSceneHoverEvent * event)
{
  auto pos = event->pos();

  NodeGeometry geometry(*this);

  if ((nodeScene()->graphModel().nodeFlags(_nodeId) | NodeFlag::Resizable) &&
      geometry.resizeRect().contains(QPoint(pos.x(), pos.y())))
  {
    setCursor(QCursor(Qt::SizeFDiagCursor));
  }
  else
  {
    setCursor(QCursor());
  }

  event->accept();
}


void
NodeGraphicsObject::
mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event)
{
  QGraphicsItem::mouseDoubleClickEvent(event);

  nodeScene()->nodeDoubleClicked(_nodeId);
}


void
NodeGraphicsObject::
contextMenuEvent(QGraphicsSceneContextMenuEvent * event)
{
  nodeScene()->nodeContextMenu(_nodeId, mapToScene(event->pos()));
}

