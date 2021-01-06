#include "ConnectionState.hpp"

#include <QtCore/QPointF>

#include "ConnectionGraphicsObject.hpp"
#include "NodeGraphicsObject.hpp"
#include "NodeGraphicsScene.hpp"

namespace QtNodes
{

ConnectionState::
~ConnectionState()
{
  resetLastHoveredNode();
}


void
ConnectionState::
interactWithNode(NodeId const nodeId)
{
  if (nodeId != InvalidNodeId)
  {
    _lastHoveredNode = nodeId;
  }
  else
  {
    resetLastHoveredNode();
  }
}


void
ConnectionState::
setLastHoveredNode(NodeId const nodeId)
{
  _lastHoveredNode = nodeId;
}


void
ConnectionState::
resetLastHoveredNode()
{
  if (_lastHoveredNode != InvalidNodeId)
  {
    auto & ngo = *_cgo.scene().nodeGraphicsObject(_lastHoveredNode);
    ngo.nodeState().resetReactionToConnection();
  }

  _lastHoveredNode = InvalidNodeId;
}


}
