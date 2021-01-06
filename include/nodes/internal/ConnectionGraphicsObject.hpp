#pragma once

#include <utility>

#include <QtCore/QUuid>
#include <QtWidgets/QGraphicsObject>

#include "Definitions.hpp"
#include "ConnectionState.hpp"

class QGraphicsSceneMouseEvent;

namespace QtNodes
{

class NodeGraphicsScene;
class ConnectionGeometry;

/// Graphic Object for connection. Adds itself to scene
class ConnectionGraphicsObject : public QGraphicsObject
{
  Q_OBJECT

public:

  ConnectionGraphicsObject(NodeGraphicsScene & scene,
                           ConnectionId const  connectionId);

  virtual ~ConnectionGraphicsObject();

  enum { Type = UserType + 2 };
  int type() const override {
    return Type;
  }

public:

  NodeGraphicsScene & scene() const { return _scene; }

  ConnectionId connectionId() const;

  /// Needed when working with detached connection.
  void setConnectionId(ConnectionId const connectionId);

  QRectF boundingRect() const override;

  QPainterPath shape() const override;

  QPointF const & endPoint(PortType portType) const;
  QPointF out() const { return _out; }
  QPointF in() const { return _in; }

  std::pair<QPointF, QPointF> pointsC1C2() const;

  void setEndPoint(PortType portType, QPointF const & point);

  void moveEndPointBy(PortType portType, QPointF const & offset);


  void setGeometryChanged();

  /// Updates the position of both ends
  void move();

  ConnectionState const & connectionState() const;
  ConnectionState & connectionState();

protected:

  void paint(QPainter * painter,
             QStyleOptionGraphicsItem const * option,
             QWidget *  widget = 0) override;

  void mousePressEvent(QGraphicsSceneMouseEvent * event) override;

  void mouseMoveEvent(QGraphicsSceneMouseEvent * event) override;

  void mouseReleaseEvent(QGraphicsSceneMouseEvent * event) override;

  void hoverEnterEvent(QGraphicsSceneHoverEvent * event) override;

  void hoverLeaveEvent(QGraphicsSceneHoverEvent * event) override;

private:

  void addGraphicsEffect();

private:

  NodeGraphicsScene & _scene;

  ConnectionId _connectionId;

  ConnectionState _connectionState;

  QPointF _out;
  QPointF _in;

  PortIndex _outPortIndex;
  PortIndex _inPortIndex;
};

}
