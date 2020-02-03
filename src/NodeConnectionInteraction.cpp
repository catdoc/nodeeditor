#include "NodeConnectionInteraction.hpp"

#include "ConnectionGraphicsObject.hpp"
#include "DataModelRegistry.hpp"
#include "NodeDataModel.hpp"
#include "NodeGraphicsObject.hpp"
#include "NodeGraphicsScene.hpp"


namespace QtNodes
{


NodeConnectionInteraction::
NodeConnectionInteraction(NodeId const nodeId,
                          ConnectionId const connectionId,
                          NodeGraphicsScene& scene)
  : _nodeId(nodeId)
  , _connectionId(connectionId)
  , _scene(&scene)
{}


bool
NodeConnectionInteraction::
canConnect(PortIndex &portIndex) const
//canConnect(PortIndex &portIndex, TypeConverter & converter) const
{
  return false;
#if 0
  // 1) Connection requires a port

  PortType requiredPort = connectionRequiredPort();

  if (requiredPort == PortType::None)
  {
    return false;
  }

  // 1.5) Forbid connecting the node to itself
  Node* node = _connectionId->getNode(oppositePort(requiredPort));

  if (node == _nodeId)
    return false;

  // 2) connection point is on top of the node port

  QPointF connectionPoint = connectionEndScenePosition(requiredPort);

  portIndex = nodePortIndexUnderScenePoint(requiredPort,
                                           connectionPoint);

  if (portIndex == INVALID)
  {
    return false;
  }

  // 3) Node port is vacant

  // port should be empty
  if (!nodePortIsEmpty(requiredPort, portIndex))
    return false;

  // 4) Connection type equals node port type, or there is a registered type conversion that can translate between the two

  auto connectionDataType =
    _connectionId->dataType(oppositePort(requiredPort));

  auto const   &modelTarget = _nodeId->nodeDataModel();
  NodeDataType candidateNodeDataType = modelTarget->dataType(requiredPort, portIndex);

  if (connectionDataType.id != candidateNodeDataType.id)
  {
    if (requiredPort == PortType::In)
    {
      converter = _scene->registry().getTypeConverter(connectionDataType, candidateNodeDataType);
    }
    else if (requiredPort == PortType::Out)
    {
      converter = _scene->registry().getTypeConverter(candidateNodeDataType, connectionDataType);
    }

    return (converter != nullptr);
  }

  return true;
#endif
}


bool
NodeConnectionInteraction::
tryConnect() const
{
  return false;
#if 0
  // 1) Check conditions from 'canConnect'
  PortIndex portIndex = INVALID;

  TypeConverter converter;

  if (!canConnect(portIndex, converter))
  {
    return false;
  }

  // 1.5) If the connection is possible but a type conversion is needed,
  //      assign a convertor to connection
  if (converter)
  {
    _connectionId->setTypeConverter(converter);
  }

  // 2) Assign node to required port in Connection
  PortType requiredPort = connectionRequiredPort();
  _nodeId->nodeState().setConnection(requiredPort,
                                     portIndex,
                                     *_connectionId);

  // 3) Assign Connection to empty port in NodeState
  // The port is not longer required after this function
  _connectionId->setNodeToPort(*_nodeId, requiredPort, portIndex);

  // 4) Adjust Connection geometry

  _nodeId->nodeGraphicsObject().moveConnections();

  // 5) Poke model to intiate data transfer

  auto outNode = _connectionId->getNode(PortType::Out);
  if (outNode)
  {
    PortIndex outPortIndex = _connectionId->getPortIndex(PortType::Out);
    outNode->onDataUpdated(outPortIndex);
  }

  return true;
#endif
}


/// 1) Node and Connection should be already connected
/// 2) If so, clear Connection entry in the NodeState
/// 3) Set Connection end to 'requiring a port'
bool
NodeConnectionInteraction::
disconnect(PortType portToDisconnect) const
{
  return false;
#if 0
  PortIndex portIndex =
    _connectionId->getPortIndex(portToDisconnect);

  NodeState &state = _nodeId->nodeState();

  // clear pointer to Connection in the NodeState
  state.getEntries(portToDisconnect)[portIndex].clear();

  // 4) Propagate invalid data to IN node
  _connectionId->propagateEmptyData();

  // clear Connection side
  _connectionId->clearNode(portToDisconnect);

  _connectionId->setRequiredPort(portToDisconnect);

  _connectionId->getConnectionGraphicsObject().grabMouse();

  return true;
#endif
}


// ------------------ util functions below

PortType
NodeConnectionInteraction::
connectionRequiredPort() const
{
  auto const &state = _connectionId->connectionState();

  return state.requiredPort();
}


QPointF
NodeConnectionInteraction::
connectionEndScenePosition(PortType portType) const
{
  auto &go =
    _connectionId->getConnectionGraphicsObject();

  ConnectionGeometry& geometry = _connectionId->connectionGeometry();

  QPointF endPoint = geometry.getEndPoint(portType);

  return go.mapToScene(endPoint);
}


QPointF
NodeConnectionInteraction::
nodePortScenePosition(PortType portType, PortIndex portIndex) const
{
  NodeGeometry const &geom = _nodeId->nodeGeometry();

  QPointF p = geom.portScenePosition(portIndex, portType);

  NodeGraphicsObject& ngo = _nodeId->nodeGraphicsObject();

  return ngo.sceneTransform().map(p);
}


PortIndex
NodeConnectionInteraction::
nodePortIndexUnderScenePoint(PortType portType,
                             QPointF const & scenePoint) const
{
  NodeGeometry const &nodeGeom = _nodeId->nodeGeometry();

  QTransform sceneTransform =
    _nodeId->nodeGraphicsObject().sceneTransform();

  PortIndex portIndex = nodeGeom.checkHitScenePoint(portType,
                                                    scenePoint,
                                                    sceneTransform);
  return portIndex;
}


bool
NodeConnectionInteraction::
nodePortIsEmpty(PortType portType, PortIndex portIndex) const
{
  NodeState const & nodeState = _nodeId->nodeState();

  auto const & entries = nodeState.getEntries(portType);

  if (entries[portIndex].empty()) return true;

  const auto outPolicy = _nodeId->nodeDataModel()->portOutConnectionPolicy(portIndex);
  return ( portType == PortType::Out && outPolicy == NodeDataModel::ConnectionPolicy::Many);
}


}
