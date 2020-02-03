#pragma once

#include <QtCore/QPointF>
#include <QtGui/QTransform>

namespace QtNodes
{

class NodeGraphicsObject;
class NodeGraphicsScene;


NodeGraphicsObject*
locateNodeAt(QPointF scenePoint,
             NodeGraphicsScene &scene,
             QTransform const & viewTransform);


}
