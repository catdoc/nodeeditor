#pragma once

#include "Definitions.hpp"

namespace QtNodes
{

inline
PortIndex
getNodeIndex(PortType portType, ConnectionId connectionId)
{
  NodeId id = InvalidNodeId;

  if (portType == PortType::Out)
  {
    id = std::get<0>(connectionId);
  }
  else if (portType == PortType::In)
  {
    id = std::get<2>(connectionId);
  }

  return id;
}

inline
PortIndex
getPortIndex(PortType portType, ConnectionId connectionId)
{
  PortIndex index = InvalidPortIndex;

  if (portType == PortType::Out)
  {
    index = std::get<1>(connectionId);
  }
  else if (portType == PortType::In)
  {
    index = std::get<3>(connectionId);
  }

  return index;
}


inline
bool
isPortValid(PortIndex index)
{
  return index != InvalidPortIndex;
}


inline
bool
isPortTypeValid(PortType portType)
{
  return portType != PortType::None;
}


}
