#pragma once

#include <QtCore/QPointF>
#include <QtGui/QTransform>

#include "Export.hpp"

namespace QtNodes
{

class NodeGraphicsObject;
class NodeGraphicsScene;


NodeGraphicsObject*
NODE_EDITOR_PUBLIC
locateNodeAt(QPointF scenePoint,
             NodeGraphicsScene &scene,
             QTransform const & viewTransform);


}
