#include "ConnectionGraphicsObject.hpp"

#include <QtWidgets/QGraphicsSceneMouseEvent>
#include <QtWidgets/QGraphicsDropShadowEffect>
#include <QtWidgets/QGraphicsBlurEffect>
#include <QtWidgets/QStyleOptionGraphicsItem>
#include <QtWidgets/QGraphicsView>

#include "ConnectionPainter.hpp"
#include "ConnectionState.hpp"
#include "ConnectionStyle.hpp"
#include "NodeConnectionInteraction.hpp"
#include "NodeGeometry.hpp"
#include "NodeGraphicsObject.hpp"
#include "NodeGraphicsScene.hpp"
#include "StyleCollection.hpp"
#include "locateNode.hpp"


namespace QtNodes
{

ConnectionGraphicsObject::
ConnectionGraphicsObject(NodeGraphicsScene & scene,
                         ConnectionId connectionId)
  : _scene(scene)
  , _connectionId(connectionId)
  , _connectionState(*this)
  , _out{0, 0}
  , _in{0, 0}
{
  _scene.addItem(this);

  setFlag(QGraphicsItem::ItemIsMovable,    true);
  setFlag(QGraphicsItem::ItemIsFocusable,  true);
  setFlag(QGraphicsItem::ItemIsSelectable, true);

  setAcceptHoverEvents(true);

  // addGraphicsEffect();

  setZValue(-1.0);
}


ConnectionGraphicsObject::
~ConnectionGraphicsObject()
{
  _scene.removeItem(this);
}


ConnectionId
ConnectionGraphicsObject::
connectionId() const
{
  return _connectionId;
}


QRectF
ConnectionGraphicsObject::
boundingRect() const
{
  auto points = pointsC1C2();

  // `normalized()` fixes inverted rects.
  QRectF basicRect = QRectF(_out, _in).normalized();

  QRectF c1c2Rect = QRectF(points.first, points.second).normalized();

  QRectF commonRect = basicRect.united(c1c2Rect);

  auto const & connectionStyle = StyleCollection::connectionStyle();
  float const diam = connectionStyle.pointDiameter();
  QPointF const cornerOffset(diam, diam);

  // Expand rect by port circle diameter
  commonRect.setTopLeft(commonRect.topLeft() - cornerOffset);
  commonRect.setBottomRight(commonRect.bottomRight() + 2 * cornerOffset);

  return commonRect;
}


QPainterPath
ConnectionGraphicsObject::
shape() const
{
#ifdef DEBUG_DRAWING

  //QPainterPath path;

  //path.addRect(boundingRect());
  //return path;

#else
  return ConnectionPainter::getPainterStroke(*this);
#endif
}


QPointF const &
ConnectionGraphicsObject::
endPoint(PortType portType) const
{
  Q_ASSERT(portType != PortType::None);

  return (portType == PortType::Out ?
          _out :
          _in);
}


void
ConnectionGraphicsObject::
setEndPoint(PortType portType, QPointF const & point)
{
  QPointF & p = endPoint(portType);
  p = point;
}


void
ConnectionGraphicsObject::
moveEndPointBy(PortType portType, QPointF const & offset)
{
  QPointF & p = endPoint(portType);
  p += offset;
}


void
ConnectionGraphicsObject::
setGeometryChanged()
{
  prepareGeometryChange();
}


void
ConnectionGraphicsObject::
move()
{
  auto moveEnd =
    [&](NodeId nodeId, PortType portType, PortIndex portIndex)
    {
      if (nodeId == InvalidNodeId)
        return;

      NodeGraphicsObject * ngo =
        _scene.nodeGraphicsObject(nodeId);

      NodeGeometry nodeGeometry(*ngo);

      QPointF scenePos =
        nodeGeometry.portScenePosition(portType,
                                       portIndex,
                                       ngo->sceneTransform());

      QTransform sceneTransform = this->sceneTransform();

      QPointF connectionPos = sceneTransform.inverted().map(scenePos);

      this->setEndPoint(portType, connectionPos);

    };

  moveEnd(std::get<0>(_connectionId), PortType::Out, std::get<1>(_connectionId));
  moveEnd(std::get<2>(_connectionId), PortType::In, std::get<3>(_connectionId));

  setGeometryChanged();
  update();
}


void
ConnectionGraphicsObject::
lock(bool locked)
{
  setFlag(QGraphicsItem::ItemIsMovable,    !locked);
  setFlag(QGraphicsItem::ItemIsFocusable,  !locked);
  setFlag(QGraphicsItem::ItemIsSelectable, !locked);
}


ConnectionState const &
ConnectionGraphicsObject::
connectionState() const
{
  return _connectionState;
}


ConnectionState &
ConnectionGraphicsObject::
connectionState()
{
  return _connectionState;
}


void
ConnectionGraphicsObject::
paint(QPainter * painter,
      QStyleOptionGraphicsItem const * option,
      QWidget *)
{
  painter->setClipRect(option->exposedRect);

  ConnectionPainter::paint(painter,
                           *this);
}


void
ConnectionGraphicsObject::
mousePressEvent(QGraphicsSceneMouseEvent * event)
{
  QGraphicsItem::mousePressEvent(event);
  //event->ignore();
}


void
ConnectionGraphicsObject::
mouseMoveEvent(QGraphicsSceneMouseEvent * event)
{
  prepareGeometryChange();

  auto view = static_cast<QGraphicsView *>(event->widget());
  auto ngo = locateNodeAt(event->scenePos(),
                          _scene,
                          view->transform());
  if (ngo)
  {
    _connectionState.interactWithNode(ngo->nodeId());

    GraphModel const & model = ngo->scene().graphModel();

    PortType knownPortType = oppositePort(_connectionState.requiredPort());

    NodeId knownNodeId = (knownPortType == PortType::Out) ?
                         std::get<0>(_connectionId) :
                         std::get<2>(_connectionId);

    PortIndex knownPortIndex = (knownPortType ==  PortType::Out) ?
                               std::get<1>(_connectionId) :
                               std::get<3>(_connectionId);

    NodeDataType knownDataType =
      model.portData(knownNodeId,
                     knownPortType,
                     knownPortIndex,
                     PortRole::DataType).value<NodeDataType>();

    ngo->nodeState().reactToPossibleConnection(_connectionState.requiredPort(),
                                               knownDataType,
                                               event->scenePos());
  }

  //-------------------

  QPointF offset = event->pos() - event->lastPos();

  auto requiredPort = _connectionState.requiredPort();

  if (requiredPort != PortType::None)
  {
    this->moveEndPointBy(requiredPort, offset);
  }

  //-------------------

  update();

  event->accept();
}


void
ConnectionGraphicsObject::
mouseReleaseEvent(QGraphicsSceneMouseEvent * event)
{
  ungrabMouse();
  event->accept();

  auto view = static_cast<QGraphicsView *>(event->widget());

  auto ngo = locateNodeAt(event->scenePos(),
                          _scene,
                          view->transform());

  NodeId nodeId = ngo ? ngo->nodeId() : InvalidNodeId;

  NodeConnectionInteraction interaction(nodeId, _connectionId, _scene);

  if (ngo && interaction.tryConnect())
  {
    ngo->nodeState().resetReactionToConnection();
  }

  if (_connectionState.requiresPort())
  {
    _scene.deleteConnection(_connectionId);
  }
}


void
ConnectionGraphicsObject::
hoverEnterEvent(QGraphicsSceneHoverEvent * event)
{
  _connectionState.setHovered(true);

  update();

  // Signal
  _scene.connectionHovered(connectionId(), event->screenPos());

  event->accept();
}


void
ConnectionGraphicsObject::
hoverLeaveEvent(QGraphicsSceneHoverEvent * event)
{
  _connectionState.setHovered(false);

  update();

  // Signal
  _scene.connectionHoverLeft(connectionId());

  event->accept();
}


std::pair<QPointF, QPointF>
ConnectionGraphicsObject::
pointsC1C2() const
{
  const double defaultOffset = 200;

  double xDistance = _in.x() - _out.x();

  double horizontalOffset = qMin(defaultOffset, std::abs(xDistance));

  double verticalOffset = 0;

  double ratioX = 0.5;

  if (xDistance <= 0)
  {
    double yDistance = _in.y() - _out.y() + 20;

    double vector = yDistance < 0 ? -1.0 : 1.0;

    verticalOffset = qMin(defaultOffset, std::abs(yDistance)) * vector;

    ratioX = 1.0;
  }

  horizontalOffset *= ratioX;

  QPointF c1(_out.x() + horizontalOffset,
             _out.y() + verticalOffset);

  QPointF c2(_in.x() - horizontalOffset,
             _in.y() - verticalOffset);

  return std::make_pair(c1, c2);
}


void
ConnectionGraphicsObject::
addGraphicsEffect()
{
  auto effect = new QGraphicsBlurEffect;

  effect->setBlurRadius(5);
  setGraphicsEffect(effect);

  //auto effect = new QGraphicsDropShadowEffect;
  //auto effect = new ConnectionBlurEffect(this);
  //effect->setOffset(4, 4);
  //effect->setColor(QColor(Qt::gray).darker(800));
}


}

