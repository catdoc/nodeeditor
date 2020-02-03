#pragma once

#include "Export.hpp"

#include <limits>
#include <unordered_set>
#include <unordered_map>

#include <QtCore/QVariant>

#include "Definitions.hpp"


namespace QtNodes
{

class NODE_EDITOR_PUBLIC GraphModel
{
public:

  using NodeId = QtNodes::NodeId;
  using NodeRole = QtNodes::NodeRole;

  /// @brief Returns the full set of unique Node Ids.
  /**
   * Users are responsible for generating unique unsigned int Ids for
   * all the nodes in the graph. From an Id it should be possible to
   * trace back to the model's internal representation of the node.
   */
  virtual
  std::unordered_set<NodeId> allNodeIds() const;

  /// @brief Returns all connected Node Ids for given port.
  /**
   * The returned set of nodes and port indices correspond to the type
   * opposite to the given `portType`.
   */
  virtual
  std::unordered_map<PortIndex, NodeId>
  connectedNodes(NodeId    nodeId,
                 PortType  portType,
                 PortIndex index) const;


  virtual
  void setConnectedNodes(NodeId    nodeId0,
                         PortType  portType0,
                         PortIndex index0,
                         NodeId    nodeId1,
                         PortIndex index1);

  /// @brief Returns node-related data for requested NodeRole.
  /**
   * Returns: Node Caption, Node Caption Visibility,
   * Node Position etc.
   */
  virtual
  QVariant nodeData(NodeId nodeId, NodeRole role) const;

  NodeFlags nodeFlags(NodeId nodeId) const;

  /// @brief Sets node properties.
  /**
   * Sets: Node Caption, Node Caption Visibility,
   * Shyle, State, Node Position etc.
   */
  virtual
  bool setNodeData(NodeId nodeId, NodeRole role, QVariant value);

  /// @brief Returns port-related data for requested NodeRole.
  /**
   * Returns: Port Data Type, Port Data, Connection Policy, Port
   * Caption.
   */
  virtual
  QVariant portData(NodeId    nodeId,
                    PortType  portType,
                    PortIndex index,
                    PortRole  role) const;

  virtual
  bool setPortData(NodeId    nodeId,
                   PortType  portType,
                   PortIndex index,
                   PortRole  role) const;

  virtual
  bool removeConnection(ConnectionId const connectionId);
};

}
