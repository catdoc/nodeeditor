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
class NODE_EDITOR_PUBLIC BasicGraphicsScene : public QGraphicsScene
{
  Q_OBJECT
public:

  BasicGraphicsScene(GraphModel & graphModel);

  ~BasicGraphicsScene();

  // Scenes without models are not supported
  BasicGraphicsScene() = delete;

public:

  GraphModel const & graphModel() const;
  GraphModel & graphModel();

  ConnectionGraphicsObject *
  draftConnection() const;

public:

  /// Re-uses cached draft connection with the new Id.
  /** Function inserts a new ConnectionId into the GraphModel.
   */
  void useDraftConnection(ConnectionId const connectionId);

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


  // NodeId must exist in GraphModel.
  void createNode(NodeId const nodeId);

  /// Removes the node from the ModelGraph and then from the Scene.
  void deleteNode(NodeId const nodeId);

  void clearScene();

public:

  //void save() const;

  //void load();

  NodeGraphicsObject *
  nodeGraphicsObject(NodeId nodeId);

  ConnectionGraphicsObject *
  connectionGraphicsObject(ConnectionId connectionId);

Q_SIGNALS:

  void beforeNodeDeleted(NodeId const nodeId);

  void connectionCreated(ConnectionId const connectionId);
  void connectionDeleted(ConnectionId const connectionID);

  //void nodeMoved(Node& n, const QPointF& newLocation);

  void nodeDoubleClicked(NodeId const nodeId);

  void connectionHovered(ConnectionId const connectionId, QPoint const screenPos);

  void nodeHovered(NodeId const nodeId, QPoint const screenPos);

  void connectionHoverLeft(ConnectionId const connectionId);

  void nodeHoverLeft(NodeId const nodeId);

  void nodeContextMenu(NodeId const nodeId, QPointF const pos);

private:

  /// @brief Creates Node and Connection graphics objects.
  /** We perform depth-first graph traversal. The connections are
   * created by checking non-empyt node's Out ports.
   */
  void traverseGraphAndPopulateGraphicsObjects();

private Q_SLOTS:

  void
  onPortsAboutToBeDeleted(NodeId const   nodeId,
                          PortType const portType,
                          std::unordered_set<PortIndex> const & portIndexSet);

  void
  onPortsDeleted(NodeId const   nodeId,
                 PortType const portType,
                 std::unordered_set<PortIndex> const & portIndexSet);

  void
  onPortsAboutToBeInserted(NodeId const   nodeId,
                           PortType const portType,
                           std::unordered_set<PortIndex> const & portIndexSet);

  void
  onPortsInserted(NodeId const   nodeId,
                  PortType const portType,
                  std::unordered_set<PortIndex> const & portIndexSet);

  void setupConnectionSignals(ConnectionId const connectionId);

  void sendConnectionCreatedToNodes(ConnectionId const connectionId)

  void sendConnectionDeletedToNodes(Connection const connectionId);


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

};


}
