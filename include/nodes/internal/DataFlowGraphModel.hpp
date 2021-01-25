#pragma once

#include <memory>

#include "ConnectionIdUtils.hpp"
#include "DataModelRegistry.hpp"
#include "Export.hpp"
#include "GraphModel.hpp"
#include "StyleCollection.hpp"


namespace QtNodes
{

class NODE_EDITOR_PUBLIC DataFlowGraphModel : public GraphModel
{
  Q_OBJECT

public:

  struct NodeGeometryData
  {
    QSize size;
    QPointF pos;
  };

public:

  DataFlowGraphModel(std::shared_ptr<DataModelRegistry> registry)
    : _registry(registry)
    , _nextNodeId{0}
  {}

  std::shared_ptr<DataModelRegistry>
  dataModelRegistry() { return _registry; }

public:

  std::unordered_set<NodeId>
  allNodeIds() const override
  {
    std::unordered_set<NodeId> nodeIds;
    for_each(_models.begin(), _models.end(),
             [&nodeIds](auto const & p)
             {
               nodeIds.insert(p.first);

             });

    return nodeIds;
  }

  std::unordered_set<std::pair<NodeId, PortIndex>>
  connectedNodes(NodeId    nodeId,
                 PortType  portType,
                 PortIndex portIndex) const override
  {
    return _connectivity[std::make_tuple(nodeId,
                                         portType,
                                         portIndex)];
  }

  NodeId
  addNode(QString const nodeType) override
  {

    std::unique_ptr<NodeDataModel> model =
      _registry->create(nodeType);

    if (model)
    {
      NodeId newId = newNodeId();
      _models[newId] = std::move(model);

      Q_EMIT nodeCreated(newId);

      return newId;
    }

    return InvalidNodeId;
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

    Q_EMIT connectionCreated(connectionId);
  }

  QVariant
  nodeData(NodeId nodeId, NodeRole role) const override
  {
    QVariant result;

    auto it = _models.find(nodeId);
    if (it == _models.end())
      return result;

    auto & model = it->second;

    switch (role)
    {
      case NodeRole::Type:
        result = model->name();
        break;

      case NodeRole::Position:
        result = _nodeGeometryData[nodeId].pos;
        break;

      case NodeRole::Size:
        result = _nodeGeometryData[nodeId].size;
        break;

      case NodeRole::CaptionVisible:
        result = model->captionVisible();
        break;

      case NodeRole::Caption:
        result = model->caption();
        break;

      case NodeRole::Style:
        {
          auto style = StyleCollection::nodeStyle();
          result = style.toJson().toVariant();
        }
        break;

      case NodeRole::NumberOfInPorts:
        result = model->nPorts(PortType::In);
        break;

      case NodeRole::NumberOfOutPorts:
        result = model->nPorts(PortType::Out);
        break;

      case NodeRole::Widget:
        result = QVariant();
        break;
    }

    return result;
  }

  bool
  setNodeData(NodeId   nodeId,
              NodeRole role,
              QVariant value) override
  {
    Q_UNUSED(nodeId);
    Q_UNUSED(role);
    Q_UNUSED(value);

    bool result = false;

    switch (role)
    {
      case NodeRole::Type:
        break;
      case NodeRole::Position:
        {
          _nodeGeometryData[nodeId].pos = value.value<QPointF>();

          Q_EMIT nodePositonUpdated(nodeId);

          result = true;
        }
        break;

      case NodeRole::Size:
        {
          _nodeGeometryData[nodeId].size = value.value<QSize>();
          result = true;
        }
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
  portData(NodeId    nodeId,
           PortType  portType,
           PortIndex portIndex,
           PortRole  role) const override
  {
    QVariant result;

    auto it = _models.find(nodeId);
    if (it == _models.end())
      return result;

    auto & model = it->second;

    switch (role)
    {
      case PortRole::Data:
        result = QVariant();
        break;

      case PortRole::DataType:
        result = QVariant::fromValue(model->dataType(portType, portIndex));
        break;

      case PortRole::ConnectionPolicy:
        if (portType == PortType::Out)
          result = QVariant::fromValue(model->portOutConnectionPolicy(portIndex));
        else
          result = QVariant::fromValue(ConnectionPolicy::One);
        break;

      case PortRole::CaptionVisible:
        result = model->portCaptionVisible(portType, portIndex);
        break;

      case PortRole::Caption:
        result = model->portCaption(portType, portIndex);

        break;
    }

    return result;
  }


  bool
  setPortData(NodeId    nodeId,
              PortType  portType,
              PortIndex portIndex,
              PortRole  role) const override
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

    Q_EMIT connectionDeleted(connectionId);

    return disconnected;
  }

  bool
  deleteNode(NodeId const nodeId) override
  {
    // Delete connections to this node first.
    auto connectionIds = allConnectionIds(nodeId);
    for (auto & cId : connectionIds)
    {
      deleteConnection(cId);
    }

    _nodeGeometryData.erase(nodeId);
    _models.erase(nodeId);

    Q_EMIT nodeDeleted(nodeId);

    return true;
  }

private:

  NodeId
  newNodeId() { return _nextNodeId++; }

private:

  std::shared_ptr<DataModelRegistry> _registry;

  NodeId _nextNodeId;

  std::unordered_map<NodeId,
                     std::unique_ptr<NodeDataModel>>
  _models;

  mutable
  std::unordered_map<std::tuple<NodeId, PortType, PortIndex>,
                     std::unordered_set<std::pair<NodeId, PortIndex>>>
  _connectivity;

  mutable std::unordered_map<NodeId, NodeGeometryData>
  _nodeGeometryData;
};


}
