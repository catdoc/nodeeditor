#pragma once

#include <QtCore/QUuid>
#include <QtWidgets/QGraphicsScene>

#include <functional>
#include <memory>
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

  ConnectionGraphicsObject *
  draftConnection() const;

private:

  /// @brief Creates Node and Connection graphics objects.
  /** We perform depth-first graph traversal. The connections are
   * created by checking non-empyt node's Out ports.
   */
  void traverseGraphAndPopulateGraphicsObjects();

public:

  /// Re-uses cached draft connection with the new Id.
  /** Function inserts a new ConnectionId into theGraphModel.
   */
  void useDraftConnection(ConnectionId const connectionId);

  //std::shared_ptr<Connection> restoreConnection(QJsonObject const & connectionJson);

  /// Deletes the object from the main connection object set.
  /** The corresponding ConnectionId is removed from the GraphModel.
   * The function returns a unique pointer to the graphics object. If
   * the pointer is not stored somewhere, the object is automatically
   * destroyed and removed from the scene.
   */
  std::unique_ptr<ConnectionGraphicsObject>
  deleteConnection(ConnectionId const connectionId);

  /// Caches ConnectionGraphicsObject to a "draft connection" variable.
  /**
   * The cached variable is designed to store a "draft" connection
   * which has not been attached to both nodes yes. After a proper
   * attachemed the variable is cleared an the ConnectionGraphicsObject
   * reseives some proper valid ConnectionId. After that the object is
   * inserted into the main set with connectinos.
   * 
   * If the passed connection pointer is empty, a new object is created
   * automatically.
   */
  bool
  makeDraftConnection(std::unique_ptr<ConnectionGraphicsObject> && cgo,
                      ConnectionId const newConnectionId);

  /// Same function as before but creates Graphics object inside.
  bool
  makeDraftConnection(ConnectionId const newConnectionId);

  //Node & createNode(std::unique_ptr<NodeDataModel> && dataModel);

  //Node & restoreNode(QJsonObject const & nodeJson);

  // NodeId must exist in GraphModel.
  void createNode(NodeId const nodeId);

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

  // TODO shared pointer?
  GraphModel & _graphModel;


  using UniqueNodeGraphicsObject =
    std::unique_ptr<NodeGraphicsObject>;

  using UniqueConnectionGraphicsObject =
    std::unique_ptr<ConnectionGraphicsObject>;

  std::unordered_map<NodeId, UniqueNodeGraphicsObject>
  _nodeGraphicsObjects;

  std::unordered_map<ConnectionId,
                     UniqueConnectionGraphicsObject>
  _connectionGraphicsObjects;


  //std::shared_ptr<DataModelRegistry>          _registry;

  std::unique_ptr<ConnectionGraphicsObject> _draftConnection;

private Q_SLOTS:

  //void setupConnectionSignals(Connection const& c);
  //void sendConnectionCreatedToNodes(Connection const& c);
  //void sendConnectionDeletedToNodes(Connection const& c);
};


}
