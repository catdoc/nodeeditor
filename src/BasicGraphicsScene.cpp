#include "BasicGraphicsScene.hpp"

#include <queue>
#include <iostream>
#include <stdexcept>
#include <unordered_set>
#include <utility>

#include <QtWidgets/QGraphicsSceneMoveEvent>
#include <QtWidgets/QFileDialog>

#include <QtCore/QBuffer>
#include <QtCore/QByteArray>
#include <QtCore/QDataStream>
#include <QtCore/QFile>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QtGlobal>

#include <QtCore/QDebug>

#include "ConnectionGraphicsObject.hpp"
#include "GraphicsView.hpp"
#include "NodeGraphicsObject.hpp"


namespace QtNodes
{

BasicGraphicsScene::
BasicGraphicsScene(GraphModel &graphModel,
                   QObject *   parent)
  : QGraphicsScene(parent)
  , _graphModel(graphModel)
{

  connect(&_graphModel, &GraphModel::portsAboutToBeDeleted,
          this, &BasicGraphicsScene::onPortsAboutToBeDeleted);

  connect(&_graphModel, &GraphModel::portsDeleted,
          this, &BasicGraphicsScene::onPortsDeleted);

  connect(&_graphModel, &GraphModel::portsAboutToBeInserted,
          this, &BasicGraphicsScene::onPortsAboutToBeInserted);

  connect(&_graphModel, &GraphModel::portsInserted,
          this, &BasicGraphicsScene::onPortsInserted);

  // This connection should come first
  connect(this, &BasicGraphicsScene::connectionCreated,
          this, &BasicGraphicsScene::setupConnectionSignals);

  connect(this, &BasicGraphicsScene::connectionCreated,
          this, &BasicGraphicsScene::sendConnectionCreatedToNodes);

  connect(this, &BasicGraphicsScene::connectionDeleted,
          this, &BasicGraphicsScene::sendConnectionDeletedToNodes);


  traverseGraphAndPopulateGraphicsObjects();
}


BasicGraphicsScene::
~BasicGraphicsScene()
{
  clearScene();
}


GraphModel const &
BasicGraphicsScene::
graphModel() const
{
  return _graphModel;
}


GraphModel &
BasicGraphicsScene::
graphModel()
{
  return _graphModel;
}


ConnectionGraphicsObject *
BasicGraphicsScene::
draftConnection() const
{
  return _draftConnection.get();
}


void
BasicGraphicsScene::
useDraftConnection(ConnectionId const newConnectionId)
{
  qDebug() << "Start creating connection";

  Q_ASSERT(_draftConnection);

  _draftConnection->setConnectionId(newConnectionId);
  _connectionGraphicsObjects[newConnectionId] = std::move(_draftConnection);

  _graphModel.addConnection(newConnectionId);

  // trigger data propagation
  //nodeOut.onDataUpdated(portIndexOut);
  Q_EMIT connectionCreated(newConnectionId);
}


std::unique_ptr<ConnectionGraphicsObject>
BasicGraphicsScene::
deleteConnection(ConnectionId const connectionId)
{
  std::unique_ptr<ConnectionGraphicsObject> removed;

  auto it = _connectionGraphicsObjects.find(connectionId);
  if (it != _connectionGraphicsObjects.end())
  {
    _graphModel.deleteConnection(connectionId);

    removed = std::move(it->second);

    _connectionGraphicsObjects.erase(it);
  }

  if (_draftConnection &&
      _draftConnection->connectionId() == connectionId)
  {
    removed = std::move(_draftConnection);
  }

  Q_EMIT connectionDeleted(connectionId);

  return removed;
}


bool
BasicGraphicsScene::
makeDraftConnection(std::unique_ptr<ConnectionGraphicsObject> &&cgo,
                    ConnectionId const newConnectionId)
{
  if (!cgo)
    cgo = std::make_unique<ConnectionGraphicsObject>(*this, newConnectionId);

  _draftConnection = std::move(cgo);

  if (_draftConnection)
  {
    _draftConnection->setConnectionId(newConnectionId);
    _draftConnection->grabMouse();

    return true;
  }

  return false;
}


bool
BasicGraphicsScene::
makeDraftConnection(ConnectionId const newConnectionId)
{
  auto uniqueCgo =
    std::make_unique<ConnectionGraphicsObject>(*this, newConnectionId);

  return makeDraftConnection(std::move(uniqueCgo), newConnectionId);
}


void
BasicGraphicsScene::
createNode(NodeId const nodeId)
{
  _nodeGraphicsObjects[nodeId] =
    std::make_unique<NodeGraphicsObject>(*this, nodeId);
}


void
BasicGraphicsScene::
deleteNode(NodeId const nodeId)
{
  Q_EMIT beforeNodeDeleted(nodeId);

  auto it = _nodeGraphicsObjects.find(nodeId);
  if (it != _nodeGraphicsObjects.end())
  {
    _graphModel.deleteNode(nodeId);
    _nodeGraphicsObjects.erase(it);
  }

  Q_EMIT nodeDeleted(nodeId);
}


void
BasicGraphicsScene::
clearScene()
{
  _connectionGraphicsObjects.clear();

  _nodeGraphicsObjects.clear();
}


NodeGraphicsObject*
BasicGraphicsScene::
nodeGraphicsObject(NodeId nodeId)
{
  NodeGraphicsObject * ngo = nullptr;
  auto it = _nodeGraphicsObjects.find(nodeId);
  if (it != _nodeGraphicsObjects.end())
  {
    ngo = it->second.get();
  }

  return ngo;
}


ConnectionGraphicsObject*
BasicGraphicsScene::
connectionGraphicsObject(ConnectionId connectionId)
{
  ConnectionGraphicsObject * cgo = nullptr;
  auto it = _connectionGraphicsObjects.find(connectionId);
  if (it != _connectionGraphicsObjects.end())
  {
    cgo = it->second.get();
  }

  return cgo;
}


QMenu *
BasicGraphicsScene::
createSceneMenu(QPointF const scenePos)
{
  Q_UNUSED(scenePos);
  return nullptr;
}


void
BasicGraphicsScene::
traverseGraphAndPopulateGraphicsObjects()
{
  auto allNodeIds = _graphModel.allNodeIds();

  while (!allNodeIds.empty())
  {
    std::queue<NodeId> fifo;

    auto firstId = *allNodeIds.begin();
    allNodeIds.erase(firstId);

    fifo.push(firstId);

    while (!fifo.empty())
    {
      auto nodeId = fifo.front();
      fifo.pop();

      _nodeGraphicsObjects[nodeId] =
        std::make_unique<NodeGraphicsObject>(*this, nodeId);

      unsigned int nOutPorts =
        _graphModel.nodeData(nodeId, NodeRole::NumberOfOutPorts).toUInt();

      for (PortIndex index = 0; index < nOutPorts; ++index)
      {
        auto connectedNodes =
          _graphModel.connectedNodes(nodeId,
                                     PortType::Out,
                                     index);

        for (auto cn : connectedNodes)
        {
          fifo.push(cn.second);
          allNodeIds.erase(cn.second);

          auto connectionId = std::make_tuple(nodeId, index, cn.first, cn.second);

          _connectionGraphicsObjects[connectionId] =
            std::make_unique<ConnectionGraphicsObject>(*this,
                                                       connectionId);
        }
      }
    } // while
  }
}


void
BasicGraphicsScene::
onPortsAboutToBeDeleted(NodeId const   nodeId,
                        PortType const portType,
                        std::unordered_set<PortIndex> const & portIndexSet)
{
  NodeGraphicsObject * node = nodeGraphicsObject(nodeId);

  if (node)
  {
    for (auto portIndex : portIndexSet)
    {
      auto const connectedNodes =
        _graphModel.connectedNodes(nodeId, portType, portIndex);

      for (auto cn : connectedNodes)
      {
        ConnectionId connectionId =
          (portType == PortType::In) ?
          std::make_tuple(cn.first, cn.second, nodeId, portIndex) :
          std::make_tuple(nodeId, portIndex, cn.first, cn.second);

        deleteConnection(connectionId);
      }
    }
  }
}


void
BasicGraphicsScene::
onPortsDeleted(NodeId const   nodeId,
               PortType const portType,
               std::unordered_set<PortIndex> const & portIndexSet)
{
  NodeGraphicsObject * node = nodeGraphicsObject(nodeId);

  if (node)
  {
    node->update();
  }
}


void
BasicGraphicsScene::
onPortsAboutToBeInserted(NodeId const   nodeId,
                         PortType const portType,
                         std::unordered_set<PortIndex> const & portIndexSet)
{
  // TODO
}


void
BasicGraphicsScene::
onPortsInserted(NodeId const   nodeId,
                PortType const portType,
                std::unordered_set<PortIndex> const & portIndexSet)
{
  // TODO
}


void
BasicGraphicsScene::
setupConnectionSignals(ConnectionId const connectionId)
{
  ConnectionGraphicsObject * cgo =
    connectionGraphicsObject(connectionId);

  //if (cgo)
  //{
  //connect(cgo, &ConnectionGraphicsObject::connectionMadeIncomplete,
  //this, &BasicGraphicsScene::connectionDeleted,
  //Qt::UniqueConnection);
  //}
}


void
BasicGraphicsScene::
sendConnectionCreatedToNodes(ConnectionId const connectionId)
{
  //Node* from = c.getNode(PortType::Out);
  //Node* to   = c.getNode(PortType::In);

  //Q_ASSERT(from != nullptr);
  //Q_ASSERT(to != nullptr);

  //from->nodeDataModel()->outputConnectionCreated(c);
  //to->nodeDataModel()->inputConnectionCreated(c);
}


void
BasicGraphicsScene::
sendConnectionDeletedToNodes(ConnectionId const connectionId)
{
  //Node* from = c.getNode(PortType::Out);
  //Node* to   = c.getNode(PortType::In);

  //Q_ASSERT(from != nullptr);
  //Q_ASSERT(to != nullptr);

  //from->nodeDataModel()->outputConnectionDeleted(c);
  //to->nodeDataModel()->inputConnectionDeleted(c);
}


//------------------------------------------------------------------------------

#if 0
ConnectionGraphicsObject &
BasicGraphicsScene::
createConnection(NodeId const    nodeId,
                 PortType const  connectedPort,
                 PortIndex const portIndex)
{
  // Construct an incomplete ConnectionId with one dangling end.
  ConnectionId const connectionId =
    (connectedPort == PortType::In) ?
    std::make_tuple(InvalidNodeId, InvalidPortIndex, nodeId, portIndex) :
    std::make_tuple(nodeId, portIndex, InvalidNodeId, InvalidPortIndex);

  auto cgo = std::make_unique<ConnectionGraphicsObject>(*this, connectionId);

  _connectionGraphicsObjects[connectionId] = std::move(cgo);

  // Note: this connection isn't truly created yet.
  // It has just one valid attached end.
  // Thus, don't send the connectionCreated(...) signal.

  //QObject::connect(connection.get(),
  //&Connection::connectionCompleted,
  //this,
  //[this](Connection const & c)
  //{ connectionCreated(c); });

  return *_connectionGraphicsObjects[connectionId];
}


#endif


//Node &
//BasicGraphicsScene::
//createNode(std::unique_ptr<NodeDataModel> && dataModel)
//{
//auto node = detail::make_unique<Node>(std::move(dataModel));
//auto ngo  = detail::make_unique<NodeGraphicsObject>(*this, *node);

//node->setGraphicsObject(std::move(ngo));

//auto nodePtr = node.get();
//_nodes[node->id()] = std::move(node);

//nodeCreated(*nodePtr);
//return *nodePtr;
//}


}
