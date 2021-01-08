#pragma once

#include <QtCore/QUuid>
#include <QtWidgets/QGraphicsScene>

#include <functional>
#include <tuple>
#include <unordered_map>

//#include "DataModelRegistry.hpp"
#include "Definitions.hpp"
#include "Export.hpp"
#include "GraphModel.hpp"
#include "ConnectionIdHash.hpp"
//#include "TypeConverter.hpp"

#include "QUuidStdHash.hpp"

namespace QtNodes
{

class ConnectionGraphicsObject;
class GraphModel;
class NodeGraphicsObject;
class NodeStyle;

/// Scene holds connections and nodes.
class NODE_EDITOR_PUBLIC NodeGraphicsScene : public QGraphicsScene
{
  Q_OBJECT
public:

  NodeGraphicsScene(GraphModel & graphModel);

  ~NodeGraphicsScene();

public:

  GraphModel const & graphModel() const;
  GraphModel & graphModel();

private:

  /// @brief Creates Node and Connection graphics objects.
  /**
   * 1. Get all the node ids.
   * 2. Tranverse the graph visiting all nodes.
   * 3. Create NodeGraphicsObjects.
   * 4. Create ConnectionGraphicsObjects.
   */
  void traverseGraphAndPopulateGraphicsObjects();

public:

  ConnectionGraphicsObject &
  createConnection(NodeId const    nodeId,
                   PortType const  connectedPort,
                   PortIndex const portIndex);

  //std::shared_ptr<Connection> createConnection(Node &                nodeIn,
  //PortIndex             portIndexIn,
  //Node &                nodeOut,
  //PortIndex             portIndexOut,
  //TypeConverter const & converter = TypeConverter{});

  //std::shared_ptr<Connection> restoreConnection(QJsonObject const & connectionJson);

  std::unique_ptr<ConnectionGraphicsObject>
  deleteConnection(ConnectionId const connectionId);

  bool
  makeDraftConnection(std::unique_ptr<ConnectionGraphicsObject> && cgo,
                      ConnectionId const newConnectionId);

  //Node & createNode(std::unique_ptr<NodeDataModel> && dataModel);

  //Node & restoreNode(QJsonObject const & nodeJson);

  void deleteNode(NodeId const nodeId);

  //DataModelRegistry & registry() const;

  //void setRegistry(std::shared_ptr<DataModelRegistry> registry);

  //void iterateOverNodes(std::function<void(Node *)> const & visitor);

  //void iterateOverNodeData(std::function<void(NodeDataModel *)> const & visitor);

  //void iterateOverNodeDataDependentOrder(std::function<void(NodeDataModel *)> const & visitor);

  //QPointF getNodePosition(Node const & node) const;

  //void setNodePosition(Node & node, QPointF const & pos) const;

  //QSizeF getNodeSize(Node const & node) const;

public:

  //std::unordered_map<QUuid, std::unique_ptr<Node> > const & nodes() const;

  //std::unordered_map<QUuid, std::shared_ptr<Connection> > const & connections() const;

  //std::vector<Node*> allNodes() const;

  std::vector<NodeId> selectedNodes() const;

public:

  void clearScene();

  //void save() const;

  //void load();

  //QByteArray saveToMemory() const;

  //void loadFromMemory(const QByteArray & data);

  NodeGraphicsObject *
  nodeGraphicsObject(NodeId nodeId);

  ConnectionGraphicsObject *
  connectionGraphicsObject(ConnectionId connectionId);

  bool
  insertDanglingConnection(std::unique_ptr<ConnectionGraphicsObject> && cgo,
                           ConnectionId connectionId);

Q_SIGNALS:

  /**
   * @brief Node has been created but not on the scene yet.
   * @see nodePlaced()
   */
  //void nodeCreated(Node &n);

  /**
   * @brief Node has been added to the scene.
   * @details Connect to this signal if need a correct position of node.
   * @see nodeCreated()
   */
  //void nodePlaced(Node &n);

  void beforeNodeDeleted(NodeId const nodeId);

  //void connectionCreated(Connection const &c);
  //void connectionDeleted(Connection const &c);

  //void nodeMoved(Node& n, const QPointF& newLocation);

  void nodeDoubleClicked(NodeId const nodeId);

  void connectionHovered(ConnectionId const connectionId, QPoint const screenPos);

  void nodeHovered(NodeId const nodeId, QPoint const screenPos);

  void connectionHoverLeft(ConnectionId const connectionId);

  void nodeHoverLeft(NodeId const nodeId);

  void nodeContextMenu(NodeId const nodeId, QPointF const pos);

private:

  //using SharedConnection = std::shared_ptr<Connection>;
  //using UniqueNode       = std::unique_ptr<Node>;

  // TODO shared pointer?
  GraphModel & _graphModel;


  // TODO: unique or normal pointers?
  // who owns graphic objects?

  using UniqueNodeGraphicsObject =
    std::unique_ptr<NodeGraphicsObject>;

  using UniqueConnectionGraphicsObject =
    std::unique_ptr<ConnectionGraphicsObject>;

  std::unordered_map<NodeId, UniqueNodeGraphicsObject>
  _nodeGraphicsObjects;

  std::unordered_map<ConnectionId,
                     UniqueConnectionGraphicsObject>
  _connectionGraphicsObjects;


  //std::unordered_map<QUuid, SharedConnection> _connections;
  //std::unordered_map<QUuid, UniqueNode>       _nodes;
  //std::shared_ptr<DataModelRegistry>          _registry;

  std::unique_ptr<ConnectionGraphicsObject> _draftConnection;

private Q_SLOTS:

  //void setupConnectionSignals(Connection const& c);

  //void sendConnectionCreatedToNodes(Connection const& c);
  //void sendConnectionDeletedToNodes(Connection const& c);
};


}
