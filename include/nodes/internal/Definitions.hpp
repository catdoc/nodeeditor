#pragma once

#include <limits>

#include <QtCore/QMetaObject>

namespace QtNodes
{
Q_NAMESPACE


enum class NodeRole
{
  Position         = 0,
  Size             = 1,
  CaptionVisible   = 2,
  Caption          = 3,
  Style            = 4,
  NumberOfInPorts  = 5,
  NumberOfOutPorts = 6,
  Widget           = 7,
};
Q_ENUM_NS(NodeRole)


enum NodeFlag
{
  Resizable = 0x0,
};
Q_DECLARE_FLAGS(NodeFlags, NodeFlag)
Q_FLAG_NS(NodeFlags)
Q_DECLARE_OPERATORS_FOR_FLAGS(NodeFlags)


enum class PortRole
{
  Data             = 0,
  DataType         = 1,
  ConnectionPolicy = 2,
  CaptionVisible   = 3,
  Caption          = 4,
};
Q_ENUM_NS(PortRole)


enum class ConnectionPolicy
{
  One,
  Many,
};
Q_ENUM_NS(ConnectionPolicy)


enum class PortType
{
  In   = 0,
  Out  = 1,
  None = 2
};
Q_ENUM_NS(PortType)


inline
PortType
oppositePort(PortType port)
{
  PortType result = PortType::None;

  switch (port)
  {
    case PortType::In:
      result = PortType::Out;
      break;

    case PortType::Out:
      result = PortType::In;
      break;

    case PortType::None:
      result = PortType::None;
      break;

    default:
      break;
  }
  return result;
}


using PortIndex = unsigned int;

static constexpr PortIndex InvalidPortIndex =
  std::numeric_limits<PortIndex>::max();

using NodeId = unsigned int;

static constexpr NodeId InvalidNodeId =
  std::numeric_limits<NodeId>::max();

using ConnectionId = std::tuple<NodeId, PortIndex,  // Port Out
                                NodeId, PortIndex>; // Port In

}
