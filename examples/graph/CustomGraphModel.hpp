#pragma once

#include <nodes/GraphModel>
#include <nodes/StyleCollection>
#include <nodes/ConnectionIdUtils>


using ConnectionId     = QtNodes::ConnectionId;
using ConnectionPolicy = QtNodes::ConnectionPolicy;
using NodeFlag        = QtNodes::NodeFlag;
using NodeFlags       = QtNodes::NodeFlags;
using NodeId          = QtNodes::NodeId;
using PortIndex       = QtNodes::PortIndex;
using PortRole        = QtNodes::PortRole;
using PortType        = QtNodes::PortType;
using StyleCollection = QtNodes::StyleCollection;
using QtNodes::InvalidNodeId;

namespace std
{
template<>
struct hash<std::tuple<NodeId, PortType, PortIndex>>
{
  using Key = std::tuple<NodeId, PortType, PortIndex>;

  inline
  std::size_t
  operator()(Key const &key) const
  {
    std::size_t h = 0;
    hash_combine(h,
                 std::get<0>(key),
                 std::get<1>(key),
                 std::get<2>(key));
    return h;
  }


};
}

struct NodeData
{
  QSize size;
  QPointF pos;
};


class CustomGraphModel : public QtNodes::GraphModel
{
public:

  CustomGraphModel()
    : _lastNodeId{0}
  {}


  std::unordered_set<NodeId>
  allNodeIds() const override
  {
    return _nodeIds;
  }


  std::unordered_set<std::pair<NodeId, PortIndex>>
  connectedNodes(NodeId nodeId,
                 PortType portType,
                 PortIndex portIndex) const override
  {
    Q_UNUSED(nodeId);
    Q_UNUSED(portType);
    Q_UNUSED(portIndex);

    return _connectivity[std::make_tuple(nodeId, portType, portIndex)];
  }


  NodeId
  addNode(QString const nodeType = QString())
  {
    NodeId newId = _lastNodeId++;
    // Create new node.
    _nodeIds.insert(newId);

    return newId;
  }


  void
  addConnection(ConnectionId const connectionId) override
  {
    auto connect =
      [&](PortType portType)
      {
        auto key = std::make_tuple(getNodeId(portType, connectionId),
                                   portType,
                                   getPortIndex(portType, connectionId));

        PortType opposite = oppositePort(portType);

        _connectivity[key].insert(std::make_pair(getNodeId(opposite, connectionId),
                                                 getPortIndex(opposite, connectionId)));
      };

    connect(PortType::Out);
    connect(PortType::In);
  }


  NodeFlags
  nodeFlags(NodeId nodeId) const override
  {
    Q_UNUSED(nodeId);

    return NodeFlag::Resizable;
  }


  QVariant
  nodeData(NodeId nodeId, NodeRole role) const override
  {
    Q_UNUSED(nodeId);

    QVariant result;

    switch (role)
    {
      case NodeRole::Type:
        result = QString("Default Node Type");
        break;

      case NodeRole::Position:
        result = _nodeData[nodeId].pos;
        break;

      case NodeRole::Size:
        result = _nodeData[nodeId].size;
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


  bool
  setNodeData(NodeId nodeId, NodeRole role, QVariant value) override
  {
    Q_UNUSED(nodeId);
    Q_UNUSED(role);
    Q_UNUSED(value);

    bool result = false;

    switch (role)
    {
      case NodeRole::Position:
      {
        _nodeData[nodeId].pos = value.value<QPointF>();
        auto p = _nodeData[nodeId].pos;

        result = true;
      }
      break;

      case NodeRole::Size:
        _nodeData[nodeId].size = value.value<QSize>();
        result = true;
        break;

      case NodeRole::CaptionVisible:
        break;

      case NodeRole::Caption:
        break;

      case NodeRole::Style:
        break;

      case NodeRole::NumberOfInPorts:
        break;

      case NodeRole::NumberOfOutPorts:
        break;

      case NodeRole::Widget:
        break;
    }

    return result;
  }


  QVariant
  portData(NodeId nodeId,
           PortType portType,
           PortIndex portIndex,
           PortRole role) const override
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
  setPortData(NodeId nodeId,
              PortType portType,
              PortIndex portIndex,
              PortRole role) const override
  {
    Q_UNUSED(nodeId);
    Q_UNUSED(portType);
    Q_UNUSED(portIndex);
    Q_UNUSED(role);

    return false;
  }


  bool
  deleteConnection(ConnectionId const connectionId) override
  {
    bool disconnected = false;

    auto disconnect =
      [&](PortType portType)
      {
        auto key = std::make_tuple(getNodeId(portType, connectionId),
                                   portType,
                                   getPortIndex(portType, connectionId));
        auto it = _connectivity.find(key);

        if (it != _connectivity.end())
        {
          disconnected = true;

          PortType opposite = oppositePort(portType);

          auto oppositePair =
            std::make_pair(getNodeId(opposite, connectionId),
                           getPortIndex(opposite, connectionId));
          it->second.erase(oppositePair);

          if (it->second.empty())
          {
            _connectivity.erase(it);
          }
        }

      };

    disconnect(PortType::Out);
    disconnect(PortType::In);

    return disconnected;
  }


  bool
  deleteNode(NodeId const nodeId) override
  {
    return false;
  }


private:

  std::unordered_set<NodeId> _nodeIds;

  mutable
  std::unordered_map<std::tuple<NodeId, PortType, PortIndex>,
                     std::unordered_set<std::pair<NodeId, PortIndex>>>
  _connectivity;

  mutable std::unordered_map<NodeId, NodeData> _nodeData;


  unsigned int _lastNodeId;

};
