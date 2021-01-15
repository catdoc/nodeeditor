#include "GraphModel.hpp"

#include <QtCore/QPoint>
#include <QtCore/QSize>
#include <QtCore/QVariant>
#include <QtWidgets/QWidget>

#include "ConnectionIdHash.hpp"
#include "ConnectionIdUtils.hpp"
#include "NodeData.hpp"
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


bool
GraphModel::
connectionPossible(ConnectionId const connectionId)
{
  NodeId nodeIdIn = getNodeId(PortType::In, connectionId);
  PortIndex portIndexIn = getPortIndex(PortType::In, connectionId);

  NodeDataType typeIn =
    portData(nodeIdIn,
             PortType::In,
             portIndexIn,
             PortRole::DataType).value<NodeDataType>();



  NodeId nodeIdOut = getNodeId(PortType::Out, connectionId);
  PortIndex portIndexOut = getPortIndex(PortType::Out, connectionId);

  NodeDataType typeOut =
    portData(nodeIdOut,
             PortType::Out,
             portIndexOut,
             PortRole::DataType).value<NodeDataType>();

  return (typeIn.id == typeOut.id);
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

  QVariant result;

  switch (role)
  {
    case NodeRole::Type:
      result = QString("Default Node Type");
      break;

    case NodeRole::Position:
      result = QPointF(0, 0); // _position;
      break;

    case NodeRole::Size:
      result = QSize(100, 100);
      break;

    case NodeRole::CaptionVisible:
      result = true;
      break;

    case NodeRole::Caption:
      result = QString("Node");
      break;

    case NodeRole::Style:
      {
        auto style = StyleCollection::nodeStyle();
        result = style.toJson().toVariant();
      }
      break;

    case NodeRole::NumberOfInPorts:
      result = 1u;
      break;

    case NodeRole::NumberOfOutPorts:
      result = 1u;
      break;

    case NodeRole::Widget:
      result = QVariant();
      break;
  }

  return result;
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
  Q_UNUSED(nodeId);
  Q_UNUSED(portIndex);

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
  Q_UNUSED(connectionId);

  return false;
}


bool
GraphModel::
deleteNode(NodeId const nodeId)
{
  Q_UNUSED(nodeId);

  return false;
}


}
