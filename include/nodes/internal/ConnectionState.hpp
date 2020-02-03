#pragma once

#include <QtCore/QUuid>

#include "PortType.hpp"
#include "Definitions.hpp"

class QPointF;

namespace QtNodes
{

class ConnectionGraphicsObject;

/// Stores currently draggind end.
/// Remembers last hovered Node.
class ConnectionState
{
public:

  /// Defines whether we construct a new connection
  /// or it is already binding two nodes.
  enum LooseEnd
  {
    Pending   = 0,
    Connected = 1
  };

public:

  ConnectionState(ConnectionGraphicsObject & cgo,
                  PortType requiredPort = PortType::None)
    : _cgo(cgo)
    , _requiredPort(requiredPort)
    , _connectedState(LooseEnd::Pending)
    , _hovered(false)
  {}

  ConnectionState(ConnectionState const&) = delete;
  ConnectionState(ConnectionState &&) = delete;

  ConnectionState& operator=(ConnectionState const&) = delete;
  ConnectionState& operator=(ConnectionState &&) = delete;

  ~ConnectionState();

public:

  void setRequiredPort(PortType end)
  { _requiredPort = end; }

  PortType requiredPort() const
  { return _requiredPort; }

  bool requiresPort() const
  { return _requiredPort != PortType::None; }

  void setNoRequiredPort()
  { _requiredPort = PortType::None; }

  bool hovered() const { return _hovered; }
  void setHovered(bool hovered) { _hovered = hovered; }

public:

  /// Caches NodeId for further interaction.
  void interactWithNode(NodeId const node);

  void setLastHoveredNode(NodeId const node);

  NodeId
  lastHoveredNode() const
  { return _lastHoveredNode; }

  void resetLastHoveredNode();

private:

  ConnectionGraphicsObject & _cgo;

  PortType _requiredPort;

  LooseEnd _connectedState;

  bool _hovered;

  NodeId _lastHoveredNode{InvalidNodeId};

};
}
