#pragma once

#include <QtCore/QPointF>
#include <QtGui/QTransform>

#include "Export.hpp"


class QGraphicsScene;

namespace QtNodes
{

class NodeGraphicsObject;


NodeGraphicsObject*
NODE_EDITOR_PUBLIC
locateNodeAt(QPointF scenePoint,
             QGraphicsScene &scene,
             QTransform const & viewTransform);


}
