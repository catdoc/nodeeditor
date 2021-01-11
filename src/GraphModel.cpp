#include "GraphModel.hpp"

#include <QtCore/QPoint>
#include <QtCore/QSize>
#include <QtCore/QVariant>
#include <QtWidgets/QWidget>

#include "ConnectionIdHash.hpp"
#include "StyleCollection.hpp"


Q_DECLARE_METATYPE(QWidget*)

namespace QtNodes
{

std::unordered_set<NodeId>
GraphModel::
allNodeIds() const
{
  std::unordered_set<NodeId> r = {1u, };

  return r;
}


std::unordered_set<std::pair<PortIndex, NodeId>>
GraphModel::
connectedNodes(NodeId    nodeId,
               PortType  portType,
               PortIndex portIndex) const
{
  Q_UNUSED(nodeId);
  Q_UNUSED(portType);
  Q_UNUSED(portIndex);

  // No connected nodes in the default implementation.
  return std::unordered_set<std::pair<PortIndex, NodeId>>();
}


void
GraphModel::
addConnection(ConnectionId const connectionId)
{
  Q_UNUSED(connectionId);
}


QVariant
GraphModel::
nodeData(NodeId nodeId, NodeRole role) const
{
  Q_UNUSED(nodeId);

  switch (role)
  {
    case NodeRole::Type:
      return QString("Default Node Type");
      break;

    case NodeRole::Position:
      return QPointF(0, 0); // _position;
      break;

    case NodeRole::Size:
      return QSize(100, 100);
      break;

    case NodeRole::CaptionVisible:
      return true;
      break;

    case NodeRole::Caption:
      return QString("Node");
      break;

    case NodeRole::Style:
      {
        auto style = StyleCollection::nodeStyle();
        return style.toJson().toVariant();
      }
      break;

    case NodeRole::NumberOfInPorts:
      return 1u;
      break;

    case NodeRole::NumberOfOutPorts:
      return 1u;
      break;

    case NodeRole::Widget:
      return QVariant();
      break;
  }
}


NodeFlags
GraphModel::
nodeFlags(NodeId nodeId) const
{
  Q_UNUSED(nodeId);

  return NodeFlag::Resizable;
}


bool
GraphModel::
setNodeData(NodeId nodeId, NodeRole role, QVariant value)
{
  Q_UNUSED(nodeId);
  Q_UNUSED(role);
  Q_UNUSED(value);

  return false;
}


QVariant
GraphModel::
portData(NodeId    nodeId,
         PortType  portType,
         PortIndex portIndex,
         PortRole  role) const
{
  switch (role)
  {
    case PortRole::Data:
      return QVariant();
      break;

    case PortRole::DataType:
      return QVariant();
      break;

    case PortRole::ConnectionPolicy:
      return QVariant::fromValue(ConnectionPolicy::One);
      break;

    case PortRole::CaptionVisible:
      return true;
      break;

    case PortRole::Caption:
      if (portType == PortType::In)
        return QString::fromUtf8("Port In");
      else
        return QString::fromUtf8("Port Out");

      break;
  }

  return QVariant();
}


bool
GraphModel::
setPortData(NodeId    nodeId,
            PortType  portType,
            PortIndex portIndex,
            PortRole  role) const
{
  Q_UNUSED(nodeId);
  Q_UNUSED(portType);
  Q_UNUSED(portIndex);
  Q_UNUSED(role);

  return false;
}


bool
GraphModel::
deleteConnection(ConnectionId const connectionId)
{
  return false;
}

bool
GraphModel::
deleteNode(NodeId const nodeId)
{
  return false;
}


}
