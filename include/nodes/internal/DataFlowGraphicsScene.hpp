#pragma once

#include "BasicGraphicsScene.hpp"
#include "DataFlowGraphModel.hpp"
#include "DataModelRegistry.hpp"


class DataFlowGraphicsScene : public BasicGraphicsScene
{
public:

  DataFlowGraphicsScene(DataFlowGraphModel & graphModel,
                        std::shared_ptr<DataModelRegistry> modelRegistry);

public:

  DataModelRegistry & registry() const;
  void setRegistry(std::shared_ptr<DataModelRegistry> registry);

public:

  std::vector<NodeId> selectedNodes() const;


  //std::shared_ptr<Connection> restoreConnection(QJsonObject const & connectionJson);

  //Node & restoreNode(QJsonObject const & nodeJson)


  //void save() const;

  //void load();

  //QByteArray saveToMemory() const;

  //void loadFromMemory(const QByteArray & data);

private:

  DataFlowGraphModel & _graphModel;
  std::shared_ptr<DataModelRegistry> _modelRegistry;

};
