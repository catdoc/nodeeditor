#include "ConnectionState.hpp"

#include <iostream>

#include <QtCore/QPointF>

#include "ConnectionGraphicsObject.hpp"
#include "NodeGraphicsScene.hpp"

using QtNodes::ConnectionState;

ConnectionState::
~ConnectionState()
{
  resetLastHoveredNode();
}


void
ConnectionState::
interactWithNode(NodeId const node)
{
  if (node != InvalidNode)
  {
    _lastHoveredNode = node;
  }
  else
  {
    resetLastHoveredNode();
  }
}


void
ConnectionState::
setLastHoveredNode(NodeId const node)
{
  _lastHoveredNode = node;
}


void
ConnectionState::
resetLastHoveredNode()
{
  if (_lastHoveredNode != InvalidNode)
    _lastHoveredNode->resetReactionToConnection();

  _lastHoveredNode = nullptr;
}

