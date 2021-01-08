#include "NodeGraphicsObject.hpp"

#include <iostream>
#include <cstdlib>

#include <QtWidgets/QtWidgets>
#include <QtWidgets/QGraphicsEffect>

#include "ConnectionGraphicsObject.hpp"
#include "ConnectionState.hpp"

#include "NodeGeometry.hpp"
#include "NodeGraphicsScene.hpp"
#include "NodePainter.hpp"

#include "NodeConnectionInteraction.hpp"

#include "StyleCollection.hpp"

using QtNodes::NodeGraphicsObject;
using QtNodes::NodeId;
using QtNodes::NodeGraphicsScene;

NodeGraphicsObject::
NodeGraphicsObject(NodeGraphicsScene & scene,
                   NodeId nodeId)
  : _scene(scene)
  , _nodeId(nodeId)
  , _nodeState(*this)
  , _proxyWidget(nullptr)
{
  _scene.addItem(this);

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

  // connect to the move signals to emit the move signals in NodeGraphicsScene
  //auto onMoveSlot = [this] { _scene.nodeMoved(_nodeId, pos()); };

  //connect(this, &QGraphicsObject::xChanged, this, onMoveSlot);
  //connect(this, &QGraphicsObject::yChanged, this, onMoveSlot);
}


NodeGraphicsObject::
~NodeGraphicsObject()
{
  _scene.removeItem(this);
}


void
NodeGraphicsObject::
embedQWidget()
{
  NodeGeometry geom(*this);

  GraphModel const & model = _scene.graphModel();

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
  GraphModel const & model = _scene.graphModel();

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

          auto cgo = _scene.connectionGraphicsObject(connectionId);

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
  if (change == ItemPositionChange) // && scene())
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
    _scene.clearSelection();
  }

  for (PortType portToCheck: {PortType::In, PortType::Out})
  {
    NodeGeometry nodeGeometry(*this);

    PortIndex const portIndex =
      nodeGeometry.checkHitScenePoint(portToCheck,
                                      event->scenePos(),
                                      sceneTransform());

    if (portIndex != InvalidPortIndex)
    {
      GraphModel const & model = _scene.graphModel();

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
                      *_scene.connectionGraphicsObject(connectionId),
                      _scene);

        interaction.disconnect(portToCheck);
      }
      else // initialize new Connection
      {
        qDebug() << "Initialize new Connection";

        if (portToCheck == PortType::Out)
        {
          GraphModel const & model = _scene.graphModel();

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

              _scene.deleteConnection(connectionId);
            }
          }
        } // if port == out

        ConnectionId const newConnectionId =
          (portToCheck == PortType::In) ?
          std::make_tuple(InvalidNodeId, InvalidPortIndex, _nodeId, portIndex) :
          std::make_tuple(_nodeId, portIndex, InvalidNodeId, InvalidPortIndex);

        auto uniqueCgo =
          std::make_unique<ConnectionGraphicsObject>(_scene, newConnectionId);

        _scene.makeDraftConnection(std::move(uniqueCgo),
                                   newConnectionId);
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

  QRectF r = _scene.sceneRect();

  r = r.united(mapToScene(boundingRect()).boundingRect());

  _scene.setSceneRect(r);
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
  _scene.nodeHovered(_nodeId, event->screenPos());

  event->accept();
}


void
NodeGraphicsObject::
hoverLeaveEvent(QGraphicsSceneHoverEvent * event)
{
  _nodeState.setHovered(false);

  update();

  // Signal
  _scene.nodeHoverLeft(_nodeId);

  event->accept();
}


void
NodeGraphicsObject::
hoverMoveEvent(QGraphicsSceneHoverEvent * event)
{
  auto pos = event->pos();

  NodeGeometry geometry(*this);

  if ((_scene.graphModel().nodeFlags(_nodeId) | NodeFlag::Resizable) &&
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

  _scene.nodeDoubleClicked(_nodeId);
}


void
NodeGraphicsObject::
contextMenuEvent(QGraphicsSceneContextMenuEvent * event)
{
  _scene.nodeContextMenu(_nodeId, mapToScene(event->pos()));
}

