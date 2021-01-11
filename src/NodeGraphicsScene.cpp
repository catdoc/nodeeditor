#include "NodeGraphicsScene.hpp"

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

NodeGraphicsScene::
NodeGraphicsScene(GraphModel & graphModel)
  : _graphModel(graphModel)
{
  traverseGraphAndPopulateGraphicsObjects();
}


void
NodeGraphicsScene::
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

          auto connectionId = std::make_tuple(nodeId, index, cn.second, cn.first);

          _connectionGraphicsObjects[connectionId] =
            std::make_unique<ConnectionGraphicsObject>(*this,
                                                       connectionId);
        }
      }
    } // while
  }

  std::cout << "Scene construction finished" << std::endl;
}


//NodeGraphicsScene::
//NodeGraphicsScene(std::shared_ptr<DataModelRegistry> registry,
//QObject *                          parent)
//: QGraphicsScene(parent)
//, _registry(std::move(registry))
//{
//setItemIndexMethod(QGraphicsScene::NoIndex);

// This connection should come first
//connect(this, &NodeGraphicsScene::connectionCreated, this, &NodeGraphicsScene::setupConnectionSignals);
//connect(this, &NodeGraphicsScene::connectionCreated, this, &NodeGraphicsScene::sendConnectionCreatedToNodes);
//connect(this, &NodeGraphicsScene::connectionDeleted, this, &NodeGraphicsScene::sendConnectionDeletedToNodes);
//}


//NodeGraphicsScene::
//NodeGraphicsScene(QObject * parent)
//: NodeGraphicsScene(std::make_shared<DataModelRegistry>(),
//parent)
//{}


NodeGraphicsScene::
~NodeGraphicsScene()
{
  clearScene();
}


GraphModel const &
NodeGraphicsScene::
graphModel() const
{
  return _graphModel;
}


GraphModel &
NodeGraphicsScene::
graphModel()
{
  return _graphModel;
}


ConnectionGraphicsObject *
NodeGraphicsScene::
draftConnection() const
{
  return _draftConnection.get();
}


//------------------------------------------------------------------------------

#if 0
ConnectionGraphicsObject &
NodeGraphicsScene::
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


void
NodeGraphicsScene::
useDraftConnection(ConnectionId const newConnectionId)
{
  qDebug() << "Start creating connection";

  Q_ASSERT(_draftConnection);

  _draftConnection->setConnectionId(newConnectionId);
  _connectionGraphicsObjects[newConnectionId] = std::move(_draftConnection);

  _graphModel.addConnection(newConnectionId);

  // trigger data propagation
  //nodeOut.onDataUpdated(portIndexOut);
  //connectionCreated(*connection);
}


//std::shared_ptr<Connection>
//NodeGraphicsScene::
//restoreConnection(QJsonObject const & connectionJson)
//{
//QUuid nodeInId  = QUuid(connectionJson["in_id"].toString());
//QUuid nodeOutId = QUuid(connectionJson["out_id"].toString());

//PortIndex portIndexIn  = connectionJson["in_index"].toInt();
//PortIndex portIndexOut = connectionJson["out_index"].toInt();

//auto nodeIn  = _nodes[nodeInId].get();
//auto nodeOut = _nodes[nodeOutId].get();

//auto getConverter =
//[&]()
//{
//QJsonValue converterVal = connectionJson["converter"];

//if (!converterVal.isUndefined())
//{
//QJsonObject converterJson = converterVal.toObject();

//NodeDataType inType { converterJson["in"].toObject()["id"].toString(),
//converterJson["in"].toObject()["name"].toString() };

//NodeDataType outType { converterJson["out"].toObject()["id"].toString(),
//converterJson["out"].toObject()["name"].toString() };

//auto converter =
//registry().getTypeConverter(outType, inType);

//if (converter)
//return converter;
//}

//return TypeConverter{};
//};

//std::shared_ptr<Connection> connection =
//createConnection(*nodeIn, portIndexIn,
//*nodeOut, portIndexOut,
//getConverter());

//// Note: the connectionCreated(...) signal has already been sent
//// by createConnection(...)

//return connection;
//}


std::unique_ptr<ConnectionGraphicsObject>
NodeGraphicsScene::
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

  return removed;
}


bool
NodeGraphicsScene::
makeDraftConnection(std::unique_ptr<ConnectionGraphicsObject> && cgo,
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
NodeGraphicsScene::
makeDraftConnection(ConnectionId const newConnectionId)
{
  auto uniqueCgo =
    std::make_unique<ConnectionGraphicsObject>(*this, newConnectionId);

  return makeDraftConnection(std::move(uniqueCgo), newConnectionId);
}


//Node &
//NodeGraphicsScene::
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


//Node &
//NodeGraphicsScene::
//restoreNode(QJsonObject const & nodeJson)
//{
//QString modelName = nodeJson["model"].toObject()["name"].toString();

//auto dataModel = registry().create(modelName);

//if (!dataModel)
//throw std::logic_error(std::string("No registered model with name ") +
//modelName.toLocal8Bit().data());

//auto node = detail::make_unique<Node>(std::move(dataModel));
//auto ngo  = detail::make_unique<NodeGraphicsObject>(*this, *node);
//node->setGraphicsObject(std::move(ngo));

//node->restore(nodeJson);

//auto nodePtr = node.get();
//_nodes[node->id()] = std::move(node);

//nodePlaced(*nodePtr);
//nodeCreated(*nodePtr);
//return *nodePtr;
//}


void
NodeGraphicsScene::
deleteNode(NodeId const nodeId)
{
  // Signal
  beforeNodeDeleted(nodeId);

  auto it = _nodeGraphicsObjects.find(nodeId);
  if (it != _nodeGraphicsObjects.end())
  {
    _graphModel.deleteNode(nodeId);
    _nodeGraphicsObjects.erase(it);
  }
}


//DataModelRegistry &
//NodeGraphicsScene::
//registry() const
//{
//return *_registry;
//}


//void
//NodeGraphicsScene::
//setRegistry(std::shared_ptr<DataModelRegistry> registry)
//{
//_registry = std::move(registry);
//}


//void
//NodeGraphicsScene::
//iterateOverNodes(std::function<void(Node *)> const & visitor)
//{
//for (const auto & _node : _nodes)
//{
//visitor(_node.second.get());
//}
//}


//void
//NodeGraphicsScene::
//iterateOverNodeData(std::function<void(NodeDataModel *)> const & visitor)
//{
//for (const auto & _node : _nodes)
//{
//visitor(_node.second->nodeDataModel());
//}
//}


//void
//NodeGraphicsScene::
//iterateOverNodeDataDependentOrder(std::function<void(NodeDataModel *)> const & visitor)
//{
//std::set<QUuid> visitedNodesSet;

////A leaf node is a node with no input ports, or all possible input ports empty
//auto isNodeLeaf =
//[](Node const & node, NodeDataModel const & model)
//{
//for (unsigned int i = 0; i < model.nPorts(PortType::In); ++i)
//{
//auto connections = node.nodeState().connections(PortType::In, i);
//if (!connections.empty())
//{
//return false;
//}
//}

//return true;
//};

////Iterate over "leaf" nodes
//for (auto const & _node : _nodes)
//{
//auto const & node = _node.second;
//auto model        = node->nodeDataModel();

//if (isNodeLeaf(*node, *model))
//{
//visitor(model);
//visitedNodesSet.insert(node->id());
//}
//}

//auto areNodeInputsVisitedBefore =
//[&](Node const & node, NodeDataModel const & model)
//{
//for (size_t i = 0; i < model.nPorts(PortType::In); ++i)
//{
//auto connections = node.nodeState().connections(PortType::In, i);

//for (auto & conn : connections)
//{
//if (visitedNodesSet.find(conn.second->getNode(PortType::Out)->id()) == visitedNodesSet.end())
//{
//return false;
//}
//}
//}

//return true;
//};

////Iterate over dependent nodes
//while (_nodes.size() != visitedNodesSet.size())
//{
//for (auto const & _node : _nodes)
//{
//auto const & node = _node.second;
//if (visitedNodesSet.find(node->id()) != visitedNodesSet.end())
//continue;

//auto model = node->nodeDataModel();

//if (areNodeInputsVisitedBefore(*node, *model))
//{
//visitor(model);
//visitedNodesSet.insert(node->id());
//}
//}
//}
//}


//QPointF
//NodeGraphicsScene::
//getNodePosition(const Node & node) const
//{
//return node.nodeGraphicsObject().pos();
//}


//void
//NodeGraphicsScene::
//setNodePosition(Node & node, const QPointF & pos) const
//{
//node.nodeGraphicsObject().setPos(pos);
//node.nodeGraphicsObject().moveConnections();
//}


//QSizeF
//NodeGraphicsScene::
//getNodeSize(const Node & node) const
//{
//return QSizeF(node.nodeGeometry().width(), node.nodeGeometry().height());
//}


//std::unordered_map<QUuid, std::unique_ptr<Node>> const &
//NodeGraphicsScene::
//nodes() const
//{
//return _nodes;
//}


//std::unordered_map<QUuid, std::shared_ptr<Connection>> const &
//NodeGraphicsScene::
//connections() const
//{
//return _connections;
//}


//std::vector<Node *>
//NodeGraphicsScene::
//allNodes() const
//{
//std::vector<Node *> nodes;

//std::transform(_nodes.begin(),
//_nodes.end(),
//std::back_inserter(nodes),
//[](std::pair<QUuid const, std::unique_ptr<Node>> const & p) { return p.second.get(); });

//return nodes;
//}


std::vector<NodeId>
NodeGraphicsScene::
selectedNodes() const
{
  QList<QGraphicsItem *> graphicsItems = selectedItems();

  std::vector<NodeId> ret;
  ret.reserve(graphicsItems.size());

  for (QGraphicsItem * item : graphicsItems)
  {
    auto ngo = qgraphicsitem_cast<NodeGraphicsObject *>(item);

    if (ngo != nullptr)
    {
      ret.push_back(ngo->nodeId());
    }
  }

  return ret;
}


//------------------------------------------------------------------------------

void
NodeGraphicsScene::
clearScene()
{
//Manual node cleanup. Simply clearing the holding datastructures
//doesn't work, the code crashes when there are both nodes and
//connections in the scene. (The data propagation internal logic
//tries to propagate data through already freed connections.)

//while (_connections.size() > 0)
//{
//deleteConnection(*_connections.begin()->second);
//}

//while (_nodes.size() > 0)
//{
//removeNode(*_nodes.begin()->second);
//}
}


NodeGraphicsObject*
NodeGraphicsScene::
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
NodeGraphicsScene::
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


//void
//NodeGraphicsScene::
//save() const
//{
//QString fileName =
//QFileDialog::getSaveFileName(nullptr,
//tr("Open Flow Scene"),
//QDir::homePath(),
//tr("Flow Scene Files (*.flow)"));

//if (!fileName.isEmpty())
//{
//if (!fileName.endsWith("flow", Qt::CaseInsensitive))
//fileName += ".flow";

//QFile file(fileName);
//if (file.open(QIODevice::WriteOnly))
//{
//file.write(saveToMemory());
//}
//}
//}


#if 0
void
NodeGraphicsScene::
load()
{
  clearScene();

//-------------

  QString fileName =
    QFileDialog::getOpenFileName(nullptr,
                                 tr("Open Flow Scene"),
                                 QDir::homePath(),
                                 tr("Flow Scene Files (*.flow)"));

  if (!QFileInfo::exists(fileName))
    return;

  QFile file(fileName);

  if (!file.open(QIODevice::ReadOnly))
    return;

  QByteArray wholeFile = file.readAll();

  loadFromMemory(wholeFile);
}


#endif


//QByteArray
//NodeGraphicsScene::
//saveToMemory() const
//{
//QJsonObject sceneJson;

//QJsonArray nodesJsonArray;

//for (auto const & pair : _nodes)
//{
//auto const & node = pair.second;

//nodesJsonArray.append(node->save());
//}

//sceneJson["nodes"] = nodesJsonArray;

//QJsonArray connectionJsonArray;
//for (auto const & pair : _connections)
//{
//auto const & connection = pair.second;

//QJsonObject connectionJson = connection->save();

//if (!connectionJson.isEmpty())
//connectionJsonArray.append(connectionJson);
//}

//sceneJson["connections"] = connectionJsonArray;

//QJsonDocument document(sceneJson);

//return document.toJson();
//}


#if 0
void
NodeGraphicsScene::
loadFromMemory(const QByteArray & data)
{
  QJsonObject const jsonDocument = QJsonDocument::fromJson(data).object();

  QJsonArray nodesJsonArray = jsonDocument["nodes"].toArray();

  for (QJsonValueRef node : nodesJsonArray)
  {
    restoreNode(node.toObject());
  }

  QJsonArray connectionJsonArray = jsonDocument["connections"].toArray();

  for (QJsonValueRef connection : connectionJsonArray)
  {
    restoreConnection(connection.toObject());
  }
}


#endif


//void
//NodeGraphicsScene::
//setupConnectionSignals(Connection const& c)
//{
//connect(&c,
//&Connection::connectionMadeIncomplete,
//this,
//&NodeGraphicsScene::connectionDeleted,
//Qt::UniqueConnection);
//}


//void
//NodeGraphicsScene::
//sendConnectionCreatedToNodes(Connection const& c)
//{
//Node* from = c.getNode(PortType::Out);
//Node* to   = c.getNode(PortType::In);

//Q_ASSERT(from != nullptr);
//Q_ASSERT(to != nullptr);

//from->nodeDataModel()->outputConnectionCreated(c);
//to->nodeDataModel()->inputConnectionCreated(c);
//}


//void
//NodeGraphicsScene::
//sendConnectionDeletedToNodes(Connection const& c)
//{
//Node* from = c.getNode(PortType::Out);
//Node* to   = c.getNode(PortType::In);

//Q_ASSERT(from != nullptr);
//Q_ASSERT(to != nullptr);

//from->nodeDataModel()->outputConnectionDeleted(c);
//to->nodeDataModel()->inputConnectionDeleted(c);
//}


}
